#pragma once
#include "../main.hpp"

namespace Path{
    void SetPath(std::string newPath);
    std::string GetPath();

    void SavePath();

    void PreviousPath();
    void NextPath();

    void Render();
    void Shutdown();
}