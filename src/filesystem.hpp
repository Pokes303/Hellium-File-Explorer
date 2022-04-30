#pragma once
#include "main.hpp"

typedef uint64_t FSTime;

enum FError{
    OK,
    
    FILE_OPEN_ERROR,
    FILE_READ_ERROR,
    FILE_WRITE_ERROR,

    DIR_OPEN_ERROR,

    ITEM_STAT_ERROR,

    DRIVE_NOT_PLUGGED,
    MOUNT_UNKNOWN_ERROR,

    DELETE_UNKNOWN_ERROR
};

namespace Filesystem{
    bool Init();
    void Shutdown();

    FError CopyFile(std::string from, std::string to, bool cut);

    FError Rename(std::string item, std::string newPath);
    FError Delete(std::string item);
    
    FError ReadDir(std::vector<FSDirectoryEntry>* items, FSStat* stat, std::string path);
    FError ReadDirRecursive(std::map<std::string, FSDirectoryEntry>* items, std::string path, std::string route);

    FError MakeFile(std::string file);
    FError MakeDir(std::string dir);

    FError DirExists(std::string dir);
    FError FileExists(std::string file);

    FError GetItemStat(std::string item, FSStat* stat);

    FError FSTimeToCalendarTime(FSTime time, OSCalendarTime* ct);
    FError MountDevice(std::string dev, std::string vol);

    void SetLastError(FSStatus _fs_err, int _iosuhax_err);
    std::string GetLastError();
};