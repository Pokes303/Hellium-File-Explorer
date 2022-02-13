#include "main.hpp"
#include "gui/filebutton.hpp"

namespace Utils{
    bool StartsWith(std::string str, std::string start);
    bool AlphabeticalSort(FileButton* a, FileButton* b);
    std::string GetFilename(std::string file);
    int WaitForDialogResponse();
}