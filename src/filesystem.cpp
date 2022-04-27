#include "filesystem.hpp"
#include "filesystem_helper.hpp"

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
        }
    }

    if (res >= 0)
    {
        //IOSUHAX filesystem
        fsaFd = IOSUHAX_FSA_Open();
        if (fsaFd < 0){
            LOG_E("IOSUHAX_FSA_Open failed (%d)", fsaFd);
        }
    }

    copyBuffer = MEMAllocFromDefaultHeap(COPY_BUFFER_SIZE);
    if (!copyBuffer){
        LOG_E("MEMAllocFromDefaultHeap failed (Not enough memory)");
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
    ClearPathDir();

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

FError Filesystem::Copy(std::string from, std::string to){

}
FError Filesystem::Cut(std::string from, std::string to){

}

FError Filesystem::Rename(std::string item, std::string newName){

}
FError Filesystem::Delete(std::string item){

}
    
FError Filesystem::ReadDir(std::vector<FSDirectoryEntry>* items, std::string path){
    FSDirectoryHandle fs_handle;
    int fs_openRes = FSOpenDir(cli, block, items.c_str(), &fs_handle, FS_ERROR_FLAG_ALL);
    if (fs_openRes < 0){
        LOG_E("FSOpenDir failed (%d) trying to get files from (%s)", iosuhax_openRes, dir.c_str());

        int iosuhax_handle;
        int iosuhax_openRes = IOSUHAX_FSA_OpenDir(fsaFd, items.c_str(), &iosuhax_handle);
        if (iosuhax_openRes < 0) {
            LOG_E("IOSUHAX_FSA_OpenDir failed (%d) trying to get files from (%s)", iosuhax_openRes, dir.c_str());
            return;
        }
        
        IOSUHAX_FSA_DirectoryEntry entry;
        int iosuhax_readRes = 0;
        while ((iosuhax_readRes = IOSUHAX_FSA_ReadDir(fsaFd, iosuhax_handle, &entry)) == 0){
            items.push_back(entry);
        }
        if (iosuhax_readRes != IOSUHAX_END_OF_DIR)
            LOG_E("IOSUHAX_FSA_ReadDir ended with unknown value (%d)", iosuhax_readRes);

        IOSUHAX_FSA_CloseDir(fsaFd, iosuhax_handle);
    }
    else {
        FSDirectoryEntry entry;
        int fs_readRes;
        while((fs_readRes = FSReadDir(cli, block, fs_handle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
            items.push_back(entry);
        }
        if (fs_readRes != FS_ERROR_END_OF_DIR)
            LOG_E("FSReadDir ended with unknown value (%d)", fs_readRes);

        FSCloseDir(cli, block, fs_handle, FS_ERROR_FLAG_ALL);
    }
    return FError::OK;


}

FError ReadDirRecursive(std::vector<FSDirectoryEntry>* items, std::string path){
    FSDirectoryHandle fs_handle;
    int fs_openRes = FSOpenDir(cli, block, dir.c_str(), &fs_handle, FS_ERROR_FLAG_ALL);
    if (fs_openRes < 0){
        LOG_E("FSOpenDir failed (%d) trying to get files from (%s)", iosuhax_openRes, dir.c_str());

        int iosuhax_handle;
        int iosuhax_openRes = IOSUHAX_FSA_OpenDir(fsaFd, _path.c_str(), &iosuhax_handle);
        if (iosuhax_openRes < 0) {
            LOG_E("IOSUHAX_FSA_OpenDir failed (%d) trying to get files from (%s)", iosuhax_openRes, dir.c_str());
            return;
        }
        (*nfolders)++;
        
        IOSUHAX_FSA_DirectoryEntry entry;
        int iosuhax_readRes = 0;
        while ((iosuhax_readRes = IOSUHAX_FSA_ReadDir(fsaFd, iosuhax_handle, &entry)) == 0){
            if (entry.info.flags & FS_STAT_DIRECTORY)
                ReadDirRecursive(dir + std::string(entry.name) + "/", nfiles, nfolders);
            else
                (*nfiles)++;
        }
        if (iosuhax_readRes != IOSUHAX_END_OF_DIR)
            LOG_E("IOSUHAX_FSA_ReadDir ended with unknown value (%d)", iosuhax_readRes);

        IOSUHAX_FSA_CloseDir(fsaFd, iosuhax_handle);
        }
    }
    else {
        FSDirectoryEntry entry;
        int fs_readRes;
        while((fs_readRes = FSReadDir(cli, block, fs_handle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
            if (entry.info.flags & FS_STAT_DIRECTORY)
                ReadDirRecursive(dir + std::string(entry.name) + "/", nfiles, nfolders);
            else
                (*nfiles)++;
        }
        if (fs_readRes != FS_ERROR_END_OF_DIR)
            LOG_E("FSReadDir ended with unknown value (%d)", fs_readRes);
        FSCloseDir(cli, block, fs_handle, FS_ERROR_FLAG_ALL);
    }
}

FError Filesystem::MakeDir(std::string dir){

}

FError Filesystem::FileExists(std::string item){

}

FError Filesystem::FSTimeToCalendarTime(FSTime time, OSCalendarTime* ct){
    internal_FSTimeToCalendarTime(time, ct);
}
FError Filesystem::TryToMount(std::string dev, std::string vol){
    int tempHandle = 0;

    int openRes = IOSUHAX_FSA_OpenDir(fsaFd, vol.c_str(), &tempHandle);
    if (openRes == (int32_t)0xFFFCFFE9){ //Not mounted; try to mount
        int mountRes = IOSUHAX_FSA_Mount(fsaFd, dev.c_str(), vol.c_str(), 0, "", 0);
        if (mountRes < 0){
            if (mountRes == (int32_t)0xFFFCFFE9){
                LOG_E("IOSUHAX_FSA_Mount failed because the drive isn't plugged (%d)", mountRes);
            }
            else{
                LOG_E("IOSUHAX_FSA_Mount unknown error (%d)", mountRes);
            }
        }
        else
            mountedDrives.push_back(vol);
    }
    else if (openRes == 0){ //Mounted; close directory and continue
        IOSUHAX_FSA_CloseDir(fsaFd, tempHandle);
    }
    else{ //Unknown open error
        LOG_E("IOSUHAX_FSA_OpenDir returned unknown value (%d)", openRes);
    }
}