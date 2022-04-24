#pragma once
#include "../main.hpp"

enum PathType{
    REAL,
    IOSUHAX,
    VIRTUAL
};

namespace Path{
    void SetPath(std::string newPath);
    std::string GetPath();
    void SetPathType(PathType newType);
    PathType GetPathType();

    void PreviousPath();
    void NextPath();

    void Render();
    void Shutdown();
}