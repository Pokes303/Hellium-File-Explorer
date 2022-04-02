#include "bubbles.hpp"
#include "gui/bubble.hpp"

float bubbleSpeed = 2.0f;
Bubble bubbles[MAX_BUBBLES];

void initBubbles(){

    for (int i = 0; i < MAX_BUBBLES; i++){
        bubbles[i] = Bubble();
    }
}

void renderBubbles(){
    for (int i = 0; i < MAX_BUBBLES; i++){
        bubbles[i].Render();
    }
}

void destroyBubbles(){
    for (int i = 0; i < MAX_BUBBLES; i++){
        bubbles[i].~Bubble();
    }
}