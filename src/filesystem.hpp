#pragma once
#include "main.hpp"

typedef uint64_t FSTime;

enum FError{
    OK
}

namespace Filesystem{
    bool Init();
    void Shutdown();

    FError Copy(std::string from, std::string to);
    FError Cut(std::string from, std::string to);

    FError Rename(std::string item, std::string newPath);
    FError Delete(std::string item);
    
    FError ReadDir(std::vector<FSDirectoryEntry>* items, std::string path);
    FError ReadDirRecursive(std::vector<FSDirectoryEntry>* items, std::string path);

    FError MakeFile(std::string file)
    FError MakeDir(std::string dir);

    FError FileExists(std::string file);
    FError DirExists(std::string dir);

    FError FSTimeToCalendarTime(FSTime time, OSCalendarTime* ct);
    FError TryToMount(std::string dev, std::string vol);
}