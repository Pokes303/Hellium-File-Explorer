#pragma once
#include "main.hpp"

namespace Filesystem{
    void Init();
    void Shutdown();

    bool CopyFile(std::string from, std::string to, bool cut);

    bool Rename(std::string item, std::string newPath);
    bool Delete(std::string item);
    
    bool ReadDir(std::vector<FSDirectoryEntry>* items, FSStat* stat, std::string path);
    bool ReadDirRecursive(std::map<std::string, bool>* items, std::string path, std::string route);

    bool MakeFile(std::string file);
    bool MakeDir(std::string dir);

    bool DirExists(std::string dir);
    bool FileExists(std::string file);

    bool MountDevice(std::string device);
    bool Filesystem::UnmountDevice(std::string device);

    //TODO: work with filesystem.h FSError GetLastError()?
    std::string GetLastError();
};