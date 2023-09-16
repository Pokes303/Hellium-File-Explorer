#pragma once

#include "main.hpp"

class Clipboard{
    private:
        std::vector<std::string> items;
        std::string path;
        bool cut;
    public:
        Clipboard();
        Clipboard(bool _cut);
        ~Clipboard();

        void AddItem(std::string _item);
        std::string GetItem(int index);
        std::map<std::string, bool> GetItems();
        uint32_t GetSize();
        std::string GetPath();
        void SetPath(std::string _path);
        bool IsCut();

};