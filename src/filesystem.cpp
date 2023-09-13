#include "filesystem.hpp"
#include "udplog.hpp"
#include "filesystem_helper.hpp"
#include "utils.hpp"

bool unlockedFS = false;
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
        return;
    }
    MochaUtilsStatus unlockRes = Mocha_UnlockFSClient(cli);
    if (unlockRes != MochaUtilsStatus::MOCHA_RESULT_SUCCESS){
        LOG_E("Mocha_UnlockFSClient failed: %d", unlockRes);
        return;
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
    FSFileHandle copyFh;
    lastError = FSOpenFile(cli, block, from.c_str(), "rb", &copyFh, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK) {
        LOG_E("FSOpenFile for copy failed <%d> with file: %s", lastError, from.c_str());
        return false;
    }

    FSStat copy_stat;
    lastError = FSGetStatFile(cli, block, copyFh, &copy_stat, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK){
        LOG_E("FSGetStatFile for copy failed <%d> with file: %s", lastError, from.c_str());
        return false;
    }

    //PASTE FILE
    FSFileHandle pasteFh;
    lastError = FSOpenFile(cli, block, from.c_str(), "rb", &pasteFh, FS_ERROR_FLAG_ALL);
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
        lastError = FSReadFileWithPos(cli, block, copyBuffer, segmentSize, 1, dataCopied, copyFh, 0, FS_ERROR_FLAG_ALL);
        if (lastError != FS_STATUS_OK){
            LOG_E("FSReadFileWithPos <%d> with file: %s", lastError, from.c_str());
            break;
        }

        LOG("Writing...");
        lastError = FSWriteFileWithPos(cli, block, copyBuffer, segmentSize, 1, dataCopied, pasteFh, 0, FS_ERROR_FLAG_ALL);
        if (lastError != FS_STATUS_OK){
            LOG_E("FSWriteFileWithPos <%d> with file: %s", lastError, to.c_str());
            break;
        }

        dataCopied += segmentSize;
        LOG("Copied bytes %d/%d", dataCopied, dataSize);
    }
    LOG("Copy operation finished with code <%d>", lastError);

    FSCloseFile(cli, block, copyFh, FS_ERROR_FLAG_ALL);
    FSCloseFile(cli, block, pasteFh, FS_ERROR_FLAG_ALL);

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
    LOG("Read dir: %s", path.c_str());

    FSDirectoryHandle dirHandle;
    lastError = FSOpenDir(cli, block, path.c_str(), &dirHandle, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK){
        LOG_E("FSOpenDir failed <%d> trying to get files from: %s", lastError, path.c_str());
        return false;
    }
    FSDirectoryEntry entry;
    while((lastError = FSReadDir(cli, block, dirHandle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
        items->push_back(entry);
    }
    if (lastError != FS_ERROR_END_OF_DIR)
        LOG_E("FSReadDir ended with unknown value (%d)", lastError);
    
    FSCloseDir(cli, block, dirHandle, FS_ERROR_FLAG_ALL);

    FSStatus statRes = FSGetStat(cli, block, path.c_str(), stat, FS_ERROR_FLAG_ALL);
    if (statRes != FS_STATUS_OK)
        LOG_E("FSGetStat error (%d)", statRes);

    return true;
}

bool Filesystem::ReadDirRecursive(std::map<std::string, bool>* items, std::string path, std::string route){
    FSDirectoryHandle dirHandle;
    lastError = FSOpenDir(cli, block, (path + route).c_str(), &dirHandle, FS_ERROR_FLAG_ALL);
    if (lastError != FS_STATUS_OK){
        LOG_E("FSOpenDir failed <%d> trying to get files from: %s;%s", lastError, path.c_str(), route.c_str());
        return false;
    }

    FSDirectoryEntry entry;
    while((lastError = FSReadDir(cli, block, dirHandle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
        bool isDir = entry.info.flags & FS_STAT_DIRECTORY;
        items->insert(std::make_pair(route + entry.name, isDir));

        if (isDir)
            ReadDirRecursive(items, path, route + std::string(entry.name) + "/");
    }
    if (lastError != FS_ERROR_END_OF_DIR)
        LOG_E("FSReadDir ended with unknown value <%d>", lastError);
    
    FSCloseDir(cli, block, dirHandle, FS_ERROR_FLAG_ALL);
    return true;
}

bool Filesystem::MakeFile(std::string file){
    LOG("Make file: %s", file.c_str());

    FSFileHandle itemFh = 0;
    lastError = FSOpenFile(cli, block, file.c_str(), "w", &itemFh, FS_ERROR_FLAG_ALL);
    if (lastError == FS_STATUS_OK){    
        LOG_E("FSOpenFile for creating a file <%d> with file: %s", lastError, file.c_str());
        return false;
    }

    FSCloseFile(cli, block, itemFh, FS_ERROR_FLAG_ALL);
    return true;
}

bool Filesystem::MakeDir(std::string dir){
    LOG("Make dir: %s", dir.c_str());
    return true;
}

bool Filesystem::DirExists(std::string dir){
    LOG("Directory exists?: %s", dir.c_str());

    FSDirectoryHandle dirHandle;
    lastError = FSOpenDir(cli, block, dir.c_str(), &dirHandle, FS_ERROR_FLAG_ALL);
    if (lastError == FS_STATUS_OK){
        LOG_E("FSOpenDir failed (%d) checking if (%s) exists", lastError, dir.c_str());
        return false;
    }

    FSCloseDir(cli, block, dirHandle, FS_ERROR_FLAG_ALL);
    return true;
}

bool Filesystem::FileExists(std::string file){
    LOG("File exists?: %s", file.c_str());
    
    FSFileHandle itemFh = 0;
    lastError = FSOpenFile(cli, block, file.c_str(), "r", &itemFh, FS_ERROR_FLAG_ALL);
    if (lastError == FS_STATUS_OK){
        LOG_E("FSOpenFile failed (%d) checking if (%s) exists", lastError, file.c_str());
        return false;
    }

    FSCloseFile(cli, block, itemFh, FS_ERROR_FLAG_ALL);
    return true;
}

bool Filesystem::MountDevice(std::string device){
    LOG("Mount device: %s", device.c_str());

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
    LOG("Unmount device: %s", device.c_str());

    MochaUtilsStatus unmountRes = Mocha_UnmountFS(device.c_str());
    if (unmountRes != MOCHA_RESULT_SUCCESS){
        LOG("Mocha_MountFS failed <%d>", unmountRes);
        return false;
    }
}

std::string Filesystem::GetLastError(){
    //TODO: try FSGetLastError
    switch(lastError){
        case FS_STATUS_OK:
            return "No error";
        case FS_STATUS_CANCELLED:
            return "CANCELLED";
        case FS_STATUS_END:
            return "END";
        case FS_STATUS_MAX:
            return "MAX";
        case FS_STATUS_ALREADY_OPEN:
            return "ALREADY_OPEN";
        case FS_STATUS_EXISTS:
            return "Item already exists";
        case FS_STATUS_NOT_FOUND:
            return "Item not found";
        case FS_STATUS_NOT_FILE:
            return "This is not a file";
        case FS_STATUS_NOT_DIR:
            return "This is not a folder";
        case FS_STATUS_ACCESS_ERROR:
            return "Access error";
        case FS_STATUS_PERMISSION_ERROR:
            return "Permission error";
        case FS_STATUS_FILE_TOO_BIG:
            return "File too big";
        case FS_STATUS_STORAGE_FULL:
            return "STORAGE_FULL";
        case FS_STATUS_JOURNAL_FULL:
            return "JOURNAL_FULL";
        case FS_STATUS_UNSUPPORTED_CMD:
            return "UNSUPPORTED_CMD";
        case FS_STATUS_MEDIA_NOT_READY:
            return "MEDIA_NOT_READY";
        case FS_STATUS_MEDIA_ERROR:
            return "MEDIA_ERROR";
        case FS_STATUS_CORRUPTED:
            return "CORRUPTED";
        case FS_STATUS_FATAL_ERROR:
            return "FATAL_ERROR";
        default:
            return "UNKNOWN ERROR";
    }
}