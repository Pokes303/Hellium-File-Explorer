#pragma once
#include "main.hpp"
#include "gui/filebutton.hpp"
#include "clipboard.hpp"

extern std::vector<FileButton*> files;

namespace FilesystemHelper{
    void ReadPathDir();
    void ClearPathDir();

    void SetPathDir(std::string dir);
    void ChangePathDir(std::string dir);
    void RewindPath();
    
    void AddPreviousPath(std::string where);

    void CreateFileProccess();
    void CreateFolderProccess();

    void CopyProccess(bool cut);
    void PasteProccess();
    
    void DeleteProccess();
    void RenameProccess();
}