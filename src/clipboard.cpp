#include "clipboard.hpp"

#include "filesystem_helper.hpp"
#include "menus/menu_main.hpp"
#include "filesystem.hpp"

std::vector<std::string> items;
std::string clipboardPath = "";
bool cut = false;

void Clipboard::AddItem(std::string _item){
    items.push_back(_item);
}

std::string Clipboard::GetItem(int index){
    return items[index];
}

std::map<std::string, bool> Clipboard::GetItems(){
    std::map<std::string, bool> recursiveItems;
    for (uint32_t i = 0; i < GetSize(); i++){
        bool isDir = Filesystem::DirExists(GetItem(i));
        recursiveItems.insert(std::make_pair(GetItem(i), isDir));
        if (isDir)
            Filesystem::ReadDirRecursive(&recursiveItems, GetPath(), GetItem(i));
    }
    return recursiveItems;
}

uint32_t Clipboard::GetSize(){
    return items.size();
}

std::string Clipboard::GetPath(){
    return clipboardPath;
}

void Clipboard::SetPath(std::string _clipboardPath){
    clipboardPath = _clipboardPath;
}

bool Clipboard::IsCut(){
    return cut;
}

void Clipboard::Clear(){
    items.clear();
    paste_b->SetActive(false);
    cut = false;
}

void Clipboard::SetCut(){
    cut = true;
}
