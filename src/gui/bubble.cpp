#include "bubble.hpp"
#include "../udplog.hpp"
#include "../SDL_Helper.hpp"
#include "../bubbles.hpp"

#define GEN_DIR() ({ float d = ((rand() % 75000 * 2) - 75000) / 100000.0f; \
                      d += (d >= 0) ? 0.25 : -0.25; \
                      d; });

Bubble::Bubble(){
    x = (rand() % (1240 + 200)) - 200;
    y = (rand() % (720 + 200)) - 200;

    xdir = GEN_DIR();
    ydir = GEN_DIR();

    int size = MAX_BUBBLE_SIZE - 1;
    for(int i = 0; i < MAX_BUBBLE_SIZE - 1; i++){
        if (rand() % (i + 2) > 0){
            size = i;
            break;
        }
    }

    switch(size){
        default:
            LOG_E("Unknown bubble size: %d", size);
        case 0: tex = bubble1_tex;
            break;
        case 1: tex = bubble2_tex;
            break;
        case 2: tex = bubble3_tex;
            break;
        case 3: tex = bubble4_tex;
            break;
        case 4: tex = bubble5_tex;
            break;
    }
}

Bubble::~Bubble(){
    tex = nullptr;
}

void Bubble::Render(){
    SDLH::DrawImage(tex, x, y);
    x += xdir * bubbleSpeed;
    y += ydir * bubbleSpeed;

    if (x < -200)
        x = 1240;
    else if (x > 1240)
        x = -100;

    if (y < -200)
        y = 720;
    else if (y > 720)
        y = -100;
}