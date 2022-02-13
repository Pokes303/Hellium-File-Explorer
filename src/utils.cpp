#include "utils.hpp"
#include "menus/menu_main.hpp"

#include <cstdarg>

bool Utils::StartsWith(std::string str, std::string start){
    for (uint32_t i = 0; i < start.size() && i < str.size(); i++){
        if (str[i] == start[i]){
            continue;
        }
        return false;
    }
    return true;
}

bool Utils::AlphabeticalSort(FileButton* a, FileButton* b){
    if (a->IsDirectory() && !b->IsDirectory())
        return true;
    else if (!a->IsDirectory() && b->IsDirectory())
        return false;
    return a->GetName() < b->GetName();
}

std::string Utils::GetFilename(std::string file){
    std::string name = "";
    for (int i = file.length() - 1; i > 0; i--){
        if (file[i] == '/'){
            return name;
        }

        name = file[i] + name;
    }
    return name;
}

int Utils::WaitForDialogResponse(){
    while (d->GetDialogResult() == -1) {}
    int res = d->GetDialogResult();
    delete d;
    d = nullptr;
    return res;
}