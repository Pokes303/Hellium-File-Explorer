#include "filesystem.hpp"
#include "udplog.hpp"
#include "filesystem_helper.hpp"
#include "utils.hpp"

#define IOSUHAX_END_OF_DIR      (int32_t)0xFFFCFFFC
#define IOSUHAX_DIR_NOT_EXISTS  (int32_t)0xFFFCFFE9
#define IOSUHAX_NOT_A_DIR       (int32_t)0xFFFCFFD7

FSStatus fs_err = FS_STATUS_OK;
int iosuhax_err = 0;

std::vector<std::string> mountedDrives;

#define COPY_BUFFER_SIZE 1024 * 1024 * 2 //2MB
uint8_t* copyBuffer;

void (*internal_FSTimeToCalendarTime)(FSTime time, OSCalendarTime *tc);
void someFunc(IOSError iose, void* arg) { (void)arg; }

int MCPHookOpen()
{
    LOG("Opening MCP Hook...");
    mcp_hook_fd = MCP_Open();
	
    if (mcp_hook_fd < 0)
		return -1;
	
    IOS_IoctlAsync(mcp_hook_fd, 0x62, (void*)0, 0, (void*)0, 0, someFunc, (void*)0);
    OSSleepTicks(OSMicrosecondsToTicks(1000));
	
    if (IOSUHAX_Open("/dev/mcp") < 0) {
        MCP_Close(mcp_hook_fd);
        mcp_hook_fd = -1;
        return -1;
    }
    return 0;
}

bool Filesystem::Init(){
    //Normal filesystem
    FSInit();
    cli = (FSClient*)MEMAllocFromDefaultHeap(sizeof(FSClient));
    FSAddClient(cli, FS_ERROR_FLAG_NONE);
    block = (FSCmdBlock*)MEMAllocFromDefaultHeap(sizeof(FSCmdBlock));
    FSInitCmdBlock(block);

    int res = IOSUHAX_Open(NULL);
    if (res < 0){
        res = MCPHookOpen();
        if (res < 0){
            LOG_E("MCP Hook finally failed (%d)", res);
            return false;
        }
    }

    if (res >= 0)
    {
        //IOSUHAX filesystem
        fsaFd = IOSUHAX_FSA_Open();
        if (fsaFd < 0){
            LOG_E("IOSUHAX_FSA_Open failed (%d)", fsaFd);
            return false;
        }
    }

    copyBuffer = (uint8_t*)MEMAllocFromDefaultHeap(COPY_BUFFER_SIZE);
    if (!copyBuffer){
        LOG_E("MEMAllocFromDefaultHeap failed");
            return false;
    }

    //Load FSTimeToCalendarTime function
    OSDynLoad_Module coreinitModule;
    OSDynLoad_Acquire("coreinit.rpl", &coreinitModule);
    void* functionHandle = NULL;
    OSDynLoad_FindExport(coreinitModule, false, "FSTimeToCalendarTime", &functionHandle);
    internal_FSTimeToCalendarTime = (void (*)(FSTime time, OSCalendarTime *tc))functionHandle;

    return true;
}

void MCPHookClose()
{
   if (mcp_hook_fd < 0)
		return;
	
    IOSUHAX_Close();

    OSSleepTicks(OSMicrosecondsToTicks(1000));

    MCP_Close(mcp_hook_fd);
    mcp_hook_fd = -1;
}
void Filesystem::Shutdown(){
    FilesystemHelper::ClearPathDir();

    for (uint32_t i = 0; i < mountedDrives.size(); i++){
        int res = IOSUHAX_FSA_Unmount(fsaFd, mountedDrives[i].c_str(), 0);
        if (res < 0){
            LOG_E("IOSUHAX_FSA_Unmount failed (%d)", res);
        }
        mountedDrives[i] = "";
    }
    mountedDrives.clear();

    OSFreeToSystem(copyBuffer);

    IOSUHAX_FSA_Close(fsaFd);
    MCPHookClose();

    MEMFreeToDefaultHeap(block);
    block = nullptr;
    FSDelClient(cli, FS_ERROR_FLAG_NONE);
    MEMFreeToDefaultHeap(cli);
    cli = nullptr;

    FSShutdown();
}

FError Filesystem::CopyFile(std::string from, std::string to, bool cut){
    LOG("CopyFile (%s) to (%s). Cut: %d", from.c_str(), to.c_str(), cut);
    
    bool copyIsFS = true;
    bool pasteIsFS = true;

    FSStat copy_stat;

    //COPY FILE
    FSFileHandle fs_copy_fh;
    int iosuhax_copy_fh = 0;
    
    fs_err = FSOpenFile(cli, block, from.c_str(), "rb", &fs_copy_fh, FS_ERROR_FLAG_ALL);
    if (fs_err != FS_STATUS_OK) {
        LOG_E("FSOpenFile for copy failed (%d) with file (%s)", fs_err, from.c_str());

        iosuhax_err = IOSUHAX_FSA_OpenFile(fsaFd, from.c_str(), "rb", &iosuhax_copy_fh);
        if (iosuhax_err < 0){
            LOG_E("IOSUHAX_FSA_OpenFile for copy failed (%d) with file (%s)", iosuhax_err, from.c_str());
            return FError::FILE_OPEN_ERROR;
        }

        iosuhax_err = IOSUHAX_FSA_GetStat(fsaFd, from.c_str(), &copy_stat);
        if (iosuhax_err < 0){
            LOG_E("IOSUHAX_FSA_GetStat for copy failed (%d) with file (%s)", iosuhax_err, from.c_str());
            return FError::ITEM_STAT_ERROR;
        }
        copyIsFS = false;
    }
    else{
        fs_err = FSGetStatFile(cli, block, fs_copy_fh, &copy_stat, FS_ERROR_FLAG_ALL);
        if (fs_err < 0){
            LOG_E("FSGetStatFile for copy failed (%d) with file (%s)", fs_err, from.c_str());
            return FError::ITEM_STAT_ERROR;
        }
    }

    //PASTE FILE
    FSFileHandle fs_paste_fh;
    int iosuhax_paste_fh = 0;

    fs_err = FSOpenFile(cli, block, from.c_str(), "rb", &fs_paste_fh, FS_ERROR_FLAG_ALL);
    if (fs_err != FS_STATUS_OK) {
        LOG_E("FSOpenFile for paste failed (%d) with file (%s)", fs_err, from.c_str());

        iosuhax_err = IOSUHAX_FSA_OpenFile(fsaFd, from.c_str(), "rb", &iosuhax_paste_fh);
        if (iosuhax_err < 0){
            LOG_E("IOSUHAX_FSA_OpenFile for paste failed (%d) with file (%s)", iosuhax_err, from.c_str());
            return FError::FILE_OPEN_ERROR;
        }
        pasteIsFS = false;
    }

    //Start copy r/w
    const uint32_t dataSize = copy_stat.size;
    uint32_t dataCopied = 0;
    FError err = FError::OK;
    while(dataCopied < dataSize){
        uint32_t segmentSize = (dataSize - dataCopied < COPY_BUFFER_SIZE) ? dataSize - dataCopied : COPY_BUFFER_SIZE;

        LOG("Reading...");
        if (copyIsFS){
            fs_err = FSReadFileWithPos(cli, block, (uint8_t*)copyBuffer, segmentSize, 1, dataCopied, fs_copy_fh, 0, FS_ERROR_FLAG_ALL);
            if (fs_err < 0){
                LOG_E("FSReadFileWithPos failed (%d) with file (%s)", fs_err, from.c_str());
                err = FError::FILE_READ_ERROR;
                break;
            }
        }
        else{
            iosuhax_err = IOSUHAX_FSA_ReadFile(fsaFd, (uint8_t*)copyBuffer, segmentSize, 1, iosuhax_copy_fh, 0);
            if (iosuhax_err < 0){
                LOG_E("IOSUHAX_FSA_ReadFile failed (%d) with file (%s)", iosuhax_err, from.c_str());
                err = FError::FILE_READ_ERROR;
                break;
            }
        }

        LOG("Writing...");
        if (copyIsFS){
            fs_err = FSWriteFileWithPos(cli, block, (uint8_t*)copyBuffer, segmentSize, 1, dataCopied, iosuhax_paste_fh, 0, FS_ERROR_FLAG_ALL);
            if (fs_err < 0){
                LOG_E("FSWriteFileWithPos failed (%d) with file (%s)", fs_err, to.c_str());
                err = FError::FILE_WRITE_ERROR;
                break;
            }
        }
        else{
            iosuhax_err = IOSUHAX_FSA_WriteFile(fsaFd, (uint8_t*)copyBuffer, segmentSize, 1, iosuhax_copy_fh, 0);
            if (iosuhax_err < 0){
                LOG_E("IOSUHAX_FSA_WriteFile failed (%d) with file (%s)", iosuhax_err, from.c_str());
                err = FError::FILE_WRITE_ERROR;
                break;
            }
        }

        dataCopied += segmentSize;
        LOG("Copied bytes %d/%d", dataCopied, dataSize);
    }
    LOG("Copy operation finished. Success: %d", err == FError::OK);

    if (copyIsFS)
        FSCloseFile(cli, block, fs_copy_fh, FS_ERROR_FLAG_ALL);
    else
        IOSUHAX_FSA_CloseFile(fsaFd, iosuhax_copy_fh);
        
    if (pasteIsFS)
        FSCloseFile(cli, block, fs_paste_fh, FS_ERROR_FLAG_ALL);
    else
        IOSUHAX_FSA_CloseFile(fsaFd, iosuhax_paste_fh);

    if (err != FError::OK)
        return err;
    return Delete(from);
}

FError Filesystem::Rename(std::string item, std::string newName){
    LOG("Rename (%s) to (%s)", item.c_str(), newName.c_str());

    fs_err = FSRename(cli, block, item.c_str(), newName.c_str(), FS_ERROR_FLAG_ALL);
    if (fs_err != FS_STATUS_OK){
        LOG_E("FSRemove failed (%d) with path (%s)", fs_err, item.c_str());

        return CopyFile(item, newName, true);
    }
    return FError::OK;
}

FError Filesystem::Delete(std::string item){
    LOG("Delete (%s)", item.c_str());

    fs_err = FSRemove(cli, block, item.c_str(), FS_ERROR_FLAG_ALL);
    if (fs_err != FS_STATUS_OK){
        LOG_E("FSRemove failed (%d) with path (%s)", fs_err, item.c_str());

        iosuhax_err = IOSUHAX_FSA_Remove(fsaFd, item.c_str());
        if (iosuhax_err < 0){
            LOG_E("IOSUHAX_FSA_Remove failed (%d) with path (%s)", iosuhax_err, item.c_str());
            return FError::DELETE_UNKNOWN_ERROR;
        }
    }
    return FError::OK;
}
    
FError fs_readDir(std::vector<FSDirectoryEntry>* items, FSStat* stat, std::string path){
    FSDirectoryHandle fs_handle;
    fs_err = FSOpenDir(cli, block, path.c_str(), &fs_handle, FS_ERROR_FLAG_ALL);
    if (fs_err != FS_STATUS_OK){
        LOG_E("FSOpenDir failed (%d) trying to get files from (%s)", fs_err, path.c_str());
        return FError::DIR_OPEN_ERROR;
    }
    FSDirectoryEntry entry;
    int fs_readRes;
    while((fs_readRes = FSReadDir(cli, block, fs_handle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
        items->push_back(entry);
    }
    if (fs_readRes != FS_ERROR_END_OF_DIR)
        LOG_E("FSReadDir ended with unknown value (%d)", fs_readRes);
    
    FSCloseDir(cli, block, fs_handle, FS_ERROR_FLAG_ALL);

    FSStatus statRes = FSGetStat(cli, block, path.c_str(), stat, FS_ERROR_FLAG_ALL);
    if (statRes != FS_STATUS_OK)
        LOG_E("FSGetStat error (%d)", statRes);

    return FError::OK;
}

FError iosuhax_readDir(std::vector<FSDirectoryEntry>* items, FSStat* stat, std::string path){
    int iosuhax_handle = 0;
    iosuhax_err = IOSUHAX_FSA_OpenDir(fsaFd, path.c_str(), &iosuhax_handle);
    if (iosuhax_err < 0) {
        LOG_E("IOSUHAX_FSA_OpenDir failed (%d) trying to get files from (%s)", iosuhax_err, path.c_str());
        return FError::DIR_OPEN_ERROR;
    }
    
    IOSUHAX_FSA_DirectoryEntry entry;
    int iosuhax_readRes = 0;
    while ((iosuhax_readRes = IOSUHAX_FSA_ReadDir(fsaFd, iosuhax_handle, &entry)) == 0){
        items->push_back(entry);
    }
    if (iosuhax_readRes != IOSUHAX_END_OF_DIR)
        LOG_E("IOSUHAX_FSA_ReadDir ended with unknown value (%d)", iosuhax_readRes);

    IOSUHAX_FSA_CloseDir(fsaFd, iosuhax_handle);;

    int statRes = IOSUHAX_FSA_GetStat(fsaFd, path.c_str(), stat);
    if (statRes < 0)
        LOG_E("IOSUHAX_FSA_GetStat error (%d)", statRes);
}

FError Filesystem::ReadDir(std::vector<FSDirectoryEntry>* items, FSStat* stat, std::string path, bool forceIOSUHAX){
    FError err;
    if (!forceIOSUHAX){
        err = fs_readDir(items,stat, path);
        if (err != FError::OK)
            return err;
    }
    err = iosuhax_readDir(items, stat, path);
    if (err != FError::OK)
        return err;

    if (!items->size())
        return FError::DIR_READ_EMPTY;
    return FError::OK;
}

FError Filesystem::ReadDirRecursive(std::map<std::string, bool>* items, std::string path, std::string route){
    FSDirectoryHandle fs_handle;
    fs_err = FSOpenDir(cli, block, (path + route).c_str(), &fs_handle, FS_ERROR_FLAG_ALL);
    if (fs_err == FS_STATUS_OK){
        FSDirectoryEntry entry;
        int fs_readRes;
        while((fs_readRes = FSReadDir(cli, block, fs_handle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
            bool isDir = entry.info.flags & FS_STAT_DIRECTORY;
            items->insert(std::make_pair(route + entry.name, isDir));

            if (isDir)
                ReadDirRecursive(items, path, route + std::string(entry.name) + "/");
        }
        if (fs_readRes != FS_ERROR_END_OF_DIR)
            LOG_E("FSReadDir ended with unknown value (%d)", fs_readRes);
        
        FSCloseDir(cli, block, fs_handle, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenDir failed (%d) trying to get files from (%s):(%s)", fs_err, path.c_str(), route.c_str());

        int iosuhax_handle = 0;
        iosuhax_err = IOSUHAX_FSA_OpenDir(fsaFd, (path + route).c_str(), &iosuhax_handle);
        if (iosuhax_err < 0) {
            LOG_E("IOSUHAX_FSA_OpenDir failed (%d) trying to get files from (%s)", iosuhax_err, path.c_str(), route.c_str());
            return FError::DIR_OPEN_ERROR;
        }
        
        IOSUHAX_FSA_DirectoryEntry entry;
        int iosuhax_readRes = 0;
        while ((iosuhax_readRes = IOSUHAX_FSA_ReadDir(fsaFd, iosuhax_handle, &entry)) == 0){
            bool isDir = entry.info.flags & FS_STAT_DIRECTORY;
            items->insert(std::make_pair(route + entry.name, isDir));

            if (isDir)
                ReadDirRecursive(items, path, route + std::string(entry.name) + "/");
        }
        if (iosuhax_readRes != IOSUHAX_END_OF_DIR)
            LOG_E("IOSUHAX_FSA_ReadDir ended with unknown value (%d)", iosuhax_readRes);

        IOSUHAX_FSA_CloseDir(fsaFd, iosuhax_handle);
    }
    return FError::OK;
}

FError Filesystem::MakeFile(std::string file){
    LOG("MakeFile (%s)", file.c_str());

    FSFileHandle fs_newitem_fh = 0;
    fs_err = FSOpenFile(cli, block, file.c_str(), "w", &fs_newitem_fh, FS_ERROR_FLAG_ALL);
    if (fs_err == FS_STATUS_OK){
        FSCloseFile(cli, block, fs_newitem_fh, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenFile for creating a file failed (%d) with file (%s)", fs_err, file.c_str());

        int iosuhax_newitem_fh = 0;
        iosuhax_err = IOSUHAX_FSA_OpenFile(fsaFd, file.c_str(), "w", &iosuhax_newitem_fh);
        if (iosuhax_err < 0){
            LOG_E("IOSUHAX_FSA_OpenFile for creating a file failed (%d) with file (%s)", iosuhax_err, file.c_str());
            return FError::FILE_OPEN_ERROR;
        }
        
        IOSUHAX_FSA_CloseFile(fsaFd, iosuhax_newitem_fh);
    }
    return FError::OK;
}

FError Filesystem::MakeDir(std::string dir){
    LOG("MakeDir (%s)", dir.c_str());
    return FError::OK;
}

bool Filesystem::DirExists(std::string dir){
    LOG("DirExists (%s)", dir.c_str());

    FSDirectoryHandle fs_handle;
    fs_err = FSOpenDir(cli, block, dir.c_str(), &fs_handle, FS_ERROR_FLAG_ALL);
    if (fs_err == FS_STATUS_OK){
        FSCloseDir(cli, block, fs_handle, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenDir failed (%d) checking if (%s) exists", fs_err, dir.c_str());

        int iosuhax_handle = 0;
        iosuhax_err = IOSUHAX_FSA_OpenDir(fsaFd, dir.c_str(), &iosuhax_handle);
        if (iosuhax_err < 0) {
            LOG_E("IOSUHAX_FSA_OpenDir failed (%d) checking if (%s) exists", iosuhax_err, dir.c_str());
            return false;
        }
        IOSUHAX_FSA_CloseDir(fsaFd, iosuhax_handle);
    }
    return true;
}

bool Filesystem::FileExists(std::string file){
    LOG("FileExists (%s)", file.c_str());
    
    FSFileHandle fs_newitem_fh = 0;
    fs_err = FSOpenFile(cli, block, file.c_str(), "r", &fs_newitem_fh, FS_ERROR_FLAG_ALL);
    if (fs_err == FS_STATUS_OK){
        FSCloseFile(cli, block, fs_newitem_fh, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenFile failed (%d) checking if (%s) exists", fs_err, file.c_str());

        int iosuhax_newitem_fh = 0;
        iosuhax_err = IOSUHAX_FSA_OpenFile(fsaFd, file.c_str(), "r", &iosuhax_newitem_fh);
        if (iosuhax_err < 0){
            LOG_E("IOSUHAX_FSA_OpenFile failed (%d) checking if (%s) exists", iosuhax_err, file.c_str());
            return false;
        }
        
        IOSUHAX_FSA_CloseFile(fsaFd, iosuhax_newitem_fh);
    }
    return true;
}

FError Filesystem::GetItemType(std::string item){
    if (FileExists(item))
        return FError::ITEM_IS_FILE;
    else if (DirExists(item))
        return FError::ITEM_IS_DIR;
    
    return FError::ITEM_NOT_EXISTS;
}

FError Filesystem::GetItemStat(std::string item, FSStat* stat){
    LOG("GetItemStat (%s)", item.c_str());
    return FError::OK;
}

OSCalendarTime Filesystem::FSTimeToCalendarTime(FSTime time){
    LOG("FSTimeToCalendarTime (%d)", time);

    OSCalendarTime ct;
    internal_FSTimeToCalendarTime(time, &ct);
    return ct;
}

FError Filesystem::MountDevice(std::string dev, std::string vol){
    LOG("MountDevice (%s) to (%s)", dev.c_str(), vol.c_str());
    
    if (!DirExists(vol)){
        iosuhax_err = IOSUHAX_FSA_Mount(fsaFd, dev.c_str(), vol.c_str(), 0, "", 0);
        if (iosuhax_err < 0){
            if (iosuhax_err == IOSUHAX_DIR_NOT_EXISTS){ //Drive isn't plugged in
                LOG_E("IOSUHAX_FSA_Mount failed because the drive isn't plugged");
                return FError::DRIVE_NOT_PLUGGED;
            }
            else{
                LOG_E("IOSUHAX_FSA_Mount unknown error (%d)", iosuhax_err);
                return FError::MOUNT_UNKNOWN_ERROR;
            }
        }
        
        mountedDrives.push_back(vol);
    }
    return FError::OK;
}

void SetLastError(FSStatus _fs_err, int _iosuhax_err){
    fs_err = _fs_err;
    iosuhax_err = _iosuhax_err;
}

std::string Filesystem::GetLastError(){
    return Utils::IntToHex(fs_err) + "_" + Utils::IntToHex(iosuhax_err);
}