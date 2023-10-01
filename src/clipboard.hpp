#pragma once

#include "main.hpp"

namespace Clipboard{
    void AddItem(std::string _item);
    std::string GetItem(int index);
    std::map<std::string, bool> GetItems();
    uint32_t GetSize();
    std::string GetPath();
    void SetPath(std::string _clipboardPath);
    bool IsCut();

    void Clear();
    void SetCut();
};