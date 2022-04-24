#pragma once
#include "main.hpp"

extern VPADStatus vpad;
extern VPADReadError vpaderror;

enum TouchStatus {
    NOT_TOUCHED,
    TOUCHED_DOWN,
    TOUCHED_HELD,
    TOUCHED_UP
};

typedef struct TouchData{
    TouchStatus status = NOT_TOUCHED; 
    int x, y;
} TouchData;
extern TouchData touch;

namespace Input{
    void ReadInput();
}

namespace SWKBD{
    enum Result{
        NOT_FINISHED,
        CANCEL,
        OK
    };

    void Init();
    void Shutdown();
    void Appear(std::string text, std::string hint, void* callback);
    bool IsShown();
    void Render();
    std::string GetResult();
}