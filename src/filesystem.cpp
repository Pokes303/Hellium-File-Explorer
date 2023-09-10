#include "filesystem.hpp"
#include "udplog.hpp"
#include "filesystem_helper.hpp"
#include "utils.hpp"

boolean unlockedFS = false;
std::vector<std::string> mountedDevices;

#define COPY_BUFFER_SIZE 1024 * 1024 * 2 //2MB (0x40*0x8000), TODO: check align with 0x40
uint8_t* copyBuffer;

FSStatus lastError;

void Filesystem::Init(){
    //Normal filesystem
    FSInit();
    cli = (FSClient*)malloc(sizeof(FSClient));
    FSAddClient(cli, FS_ERROR_FLAG_NONE);
    block = (FSCmdBlock*)malloc(sizeof(FSCmdBlock));
    FSInitCmdBlock(block);

    copyBuffer = (uint8_t*)malloc(COPY_BUFFER_SIZE);

    //Unlock with Mocha
    MochaUtilsStatus mochaRes = Mocha_InitLibrary();
    if (mochaRes != MOCHA_RESULT_SUCCESS){
        LOG_E("Mocha_InitLibrary failed: %d", mochaRes);
    }
    MochaUtilsStatus unlockRes = Mocha_UnlockFSClient(cli);
    if (unlockRes != MochaUtilsStatus::MOCHA_RESULT_SUCCESS){
        LOG_E("Mocha_UnlockFSClient failed: %d", unlockRes);
    }
    unlockedFS = true;
}

void Filesystem::Shutdown(){
    FilesystemHelper::ClearPathDir();

    //Unmount devices
    while(mountedDevices.size() > 0){
        Filesystem::UnmountDevice(mountedDevices[mountedDevices.size() - 1]);
        mountedDevices.pop_back();
    }

    //Deinit mocha
    Mocha_DeInitLibrary();

    //Free copyBuffer
    OSFreeToSystem(copyBuffer);

    //Free FS
    free(block);
    block = nullptr;
    FSDelClient(cli, FS_ERROR_FLAG_NONE);
    free(cli);
    cli = nullptr;

    FSShutdown();
}

bool Filesystem::CopyFile(std::string from, std::string to, bool cut){
    LOG("CopyFile (%s) to (%s). Cut: %d", from.c_str(), to.c_str(), cut);

    //COPY FILE
    FSFileHandle fs_copy_fh;
    lastError = FSOpenFile(cli, block, from.c_str(), "rb", &fs_copy_fh, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK) {
        LOG_E("FSOpenFile for copy failed <%d> with file: %s", lastError, from.c_str());
        return false;
    }

    FSStat copy_stat;
    lastError = FSGetStatFile(cli, block, fs_copy_fh, &copy_stat, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK){
        LOG_E("FSGetStatFile for copy failed <%d> with file: %s", lastError, from.c_str());
        return false;
    }

    //PASTE FILE
    FSFileHandle fs_paste_fh;
    lastError = FSOpenFile(cli, block, from.c_str(), "rb", &fs_paste_fh, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK) {
        LOG_E("FSOpenFile for paste failed <%d> with file: %s", lastError, from.c_str());
        return false;
    }

    //Start copy r/w
    const uint32_t dataSize = copy_stat.size;
    uint32_t dataCopied = 0;
    
    while(dataCopied < dataSize){
        uint32_t segmentSize = (dataSize - dataCopied < COPY_BUFFER_SIZE) ? dataSize - dataCopied : COPY_BUFFER_SIZE;

        LOG("Reading...");
        lastError = FSReadFileWithPos(cli, block, copyBuffer, segmentSize, 1, dataCopied, fs_copy_fh, 0, FS_ERROR_FLAG_ALL);
        if (lastError != FS_STATUS_OK){
            LOG_E("FSReadFileWithPos <%d> with file: %s", lastError, from.c_str());
            break;
        }

        LOG("Writing...");
        lastError = FSWriteFileWithPos(cli, block, copyBuffer, segmentSize, 1, dataCopied, fs_paste_fh, 0, FS_ERROR_FLAG_ALL);
        if (lastError != FS_STATUS_OK){
            LOG_E("FSWriteFileWithPos <%d> with file: %s", lastError, to.c_str());
            break;
        }

        dataCopied += segmentSize;
        LOG("Copied bytes %d/%d", dataCopied, dataSize);
    }
    LOG("Copy operation finished with code <%d>", lastError);

    FSCloseFile(cli, block, fs_copy_fh, FS_ERROR_FLAG_ALL);
    FSCloseFile(cli, block, fs_paste_fh, FS_ERROR_FLAG_ALL);

    if (!cut){
        return true;
    }
    return Delete(from);
}

bool Filesystem::Rename(std::string item, std::string newName){
    LOG("Rename file: %s, to %s", item.c_str(), newName.c_str());

    lastError = FSRename(cli, block, item.c_str(), newName.c_str(), FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK){
        LOG_E("FSRename failed <%d> with path: %s", lastError, item.c_str());
        return false;
    }
    return true;
}

bool Filesystem::Delete(std::string item){
    LOG("Delete file: %s", item.c_str());

    lastError = FSRemove(cli, block, item.c_str(), FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK){
        LOG_E("FSRemove failed <%d> with path: %s", lastError, item.c_str());
        return false;
    }
    return true;
}
    
bool ReadDir(std::vector<FSDirectoryEntry>* items, FSStat* stat, std::string path){
    FSDirectoryHandle fsHandle;
    lastError = FSOpenDir(cli, block, path.c_str(), &fsHandle, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK){
        LOG_E("FSOpenDir failed (%d) trying to get files from (%s)", lastError, path.c_str());
        return false;
    }
    FSDirectoryEntry entry;
    int fs_readRes;
    while((fs_readRes = FSReadDir(cli, block, fsHandle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
        items->push_back(entry);
    }
    if (fs_readRes != FS_ERROR_END_OF_DIR)
        LOG_E("FSReadDir ended with unknown value (%d)", fs_readRes);
    
    FSCloseDir(cli, block, fsHandle, FS_ERROR_FLAG_ALL);

    FSStatus statRes = FSGetStat(cli, block, path.c_str(), stat, FS_ERROR_FLAG_ALL);
    if (statRes != FS_STATUS_OK)
        LOG_E("FSGetStat error (%d)", statRes);

    return FError::OK;
}

bool Filesystem::ReadDirRecursive(std::map<std::string, bool>* items, std::string path, std::string route){
    FSDirectoryHandle fsHandle;
    lastError = FSOpenDir(cli, block, (path + route).c_str(), &fsHandle, FS_ERROR_FLAG_ALL);
    if (lastError == FS_STATUS_OK){
        FSDirectoryEntry entry;
        int fs_readRes;
        while((fs_readRes = FSReadDir(cli, block, fsHandle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
            bool isDir = entry.info.flags & FS_STAT_DIRECTORY;
            items->insert(std::make_pair(route + entry.name, isDir));

            if (isDir)
                ReadDirRecursive(items, path, route + std::string(entry.name) + "/");
        }
        if (fs_readRes != FS_ERROR_END_OF_DIR)
            LOG_E("FSReadDir ended with unknown value (%d)", fs_readRes);
        
        FSCloseDir(cli, block, fsHandle, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenDir failed (%d) trying to get files from (%s):(%s)", lastError, path.c_str(), route.c_str());

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

bool Filesystem::MakeFile(std::string file){
    LOG("MakeFile (%s)", file.c_str());

    FSFileHandle fs_newitem_fh = 0;
    lastError = FSOpenFile(cli, block, file.c_str(), "w", &fs_newitem_fh, FS_ERROR_FLAG_ALL);
    if (lastError == FS_STATUS_OK){
        FSCloseFile(cli, block, fs_newitem_fh, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenFile for creating a file <%d> with file: %s", lastError, file.c_str());

        int iosuhax_newitem_fh = 0;
        iosuhax_err = IOSUHAX_FSA_OpenFile(fsaFd, file.c_str(), "w", &iosuhax_newitem_fh);
        if (iosuhax_err < 0){
            LOG_E("IOSUHAX_FSA_OpenFile for creating a file <%d> with file: %s", iosuhax_err, file.c_str());
            return FError::FILE_OPEN_ERROR;
        }
        
        IOSUHAX_FSA_CloseFile(fsaFd, iosuhax_newitem_fh);
    }
    return FError::OK;
}

bool Filesystem::MakeDir(std::string dir){
    LOG("MakeDir: %s", dir.c_str());
    return FError::OK;
}

bool Filesystem::DirExists(std::string dir){
    LOG("DirExists (%s)", dir.c_str());

    FSDirectoryHandle fsHandle;
    lastError = FSOpenDir(cli, block, dir.c_str(), &fsHandle, FS_ERROR_FLAG_ALL);
    if (lastError == FS_STATUS_OK){
        FSCloseDir(cli, block, fsHandle, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenDir failed (%d) checking if (%s) exists", lastError, dir.c_str());

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
    LOG("FileExists: %s", file.c_str());
    
    FSFileHandle fs_newitem_fh = 0;
    lastError = FSOpenFile(cli, block, file.c_str(), "r", &fs_newitem_fh, FS_ERROR_FLAG_ALL);
    if (lastError == FS_STATUS_OK){
        FSCloseFile(cli, block, fs_newitem_fh, FS_ERROR_FLAG_ALL);
    }
    else{
        LOG_E("FSOpenFile failed (%d) checking if (%s) exists", lastError, file.c_str());

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

bool Filesystem::MountDevice(std::string device){
    LOG("MountDevice for device: %s", device.c_str());

    std::string devPath = "/dev/" + device;
    std::string volPath = "/vol/storage_" + device;
    
    if (!DirExists(volPath)){
        //Device not mounted
        MochaUtilsStatus mountRes = Mocha_MountFS(device.c_str(), devPath.c_str(), volPath.c_str());
        if (mountRes != MOCHA_RESULT_SUCCESS){
            LOG("Mocha_MountFS failed <%d>", mountRes);
            return false;
        }
        mountedDevices.push_back(device);
    }
    return true;
}

bool Filesystem::UnmountDevice(std::string device){
    LOG("UnmountDevice for device: %s", device.c_str());

    MochaUtilsStatus unmountRes = Mocha_UnmountFS(device.c_str());
    if (unmountRes != MOCHA_RESULT_SUCCESS){
        LOG("Mocha_MountFS failed <%d>", unmountRes);
        return false;
    }
}

std::string Filesystem::GetLastError(){
    //TODO: try FSGetLastError
    //return Utils::IntToHex(lastError);
}