#pragma once
#include "main.hpp"
#include "gui/filebutton.hpp"

namespace Utils{
    bool StartsWith(std::string str, std::string start);
    bool AlphabeticalSort(FileButton* a, FileButton* b);
    std::string GetFilename(std::string file);
    void DrawFPS();
    std::string IntToHex(int n);
    std::vector<std::string> SplitString(std::string str, char c);
}