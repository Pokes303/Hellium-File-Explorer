#include "clipboard.hpp"

#include "filesystem_helper.hpp"
#include "menus/menu_main.hpp"
#include "filesystem.hpp"

Clipboard::Clipboard(){
    path = "";
    cut = false;
}

Clipboard::Clipboard(bool _cut){
    path = "";
    cut = _cut;
}

Clipboard::~Clipboard(){
    items.clear();
    paste_b->SetActive(false);
}

void Clipboard::AddItem(std::string _item){
    items.push_back(_item);
}

std::string Clipboard::GetItem(int index){
    return items[index];
}

std::map<std::string, bool> Clipboard::GetItems(){
    std::map<std::string, bool> recursiveItems;
    for (uint32_t i = 0; i < clipboard.GetSize(); i++){
        bool isDir = Filesystem::DirExists(clipboard.GetItem(i));
        recursiveItems.insert(std::make_pair(clipboard.GetItem(i), isDir));
        if (isDir)
            Filesystem::ReadDirRecursive(&recursiveItems, clipboard.GetPath(), clipboard.GetItem(i));
    }
    return recursiveItems;
}

uint32_t Clipboard::GetSize(){
    return items.size();
}

std::string Clipboard::GetPath(){
    return path;
}

void Clipboard::SetPath(std::string _path){
    path = _path;
}

bool Clipboard::IsCut(){
    return cut;
}