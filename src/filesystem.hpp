#pragma once
#include "main.hpp"

/*enum class FError{
    OK,
    
    FILE_OPEN_ERROR,
    FILE_READ_ERROR,
    FILE_WRITE_ERROR,

    DIR_OPEN_ERROR,
    DIR_READ_EMPTY,

    ITEM_STAT_ERROR,
    
    ITEM_IS_FILE,
    ITEM_IS_DIR,
    ITEM_NOT_EXISTS,

    DRIVE_NOT_PLUGGED,
    MOUNT_UNKNOWN_ERROR,

    DELETE_UNKNOWN_ERROR
};*/

namespace Filesystem{
    void Init();
    void Shutdown();

    FError CopyFile(std::string from, std::string to, bool cut);

    FError Rename(std::string item, std::string newPath);
    FError Delete(std::string item);
    
    FError ReadDir(std::vector<FSDirectoryEntry>* items, FSStat* stat, std::string path, bool forceIOSUHAX);
    FError ReadDirRecursive(std::map<std::string, bool>* items, std::string path, std::string route);

    FError MakeFile(std::string file);
    FError MakeDir(std::string dir);

    bool DirExists(std::string dir);
    bool FileExists(std::string file);

    bool MountDevice(std::string device);
    bool Filesystem::UnmountDevice(std::string device){

    void SetLastError(FSStatus _fs_err, int _iosuhax_err);
    std::string GetLastError();
};