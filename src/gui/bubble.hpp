#pragma once
#include "../main.hpp"

class Bubble{
    private:
        SDL_Texture* tex;
        int x, y;
        float xdir, ydir;
    public:
        Bubble();
        ~Bubble();

        void Render();
};