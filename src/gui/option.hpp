#pragma once
#include "../main.hpp"

enum OptionTypes{
    BOOL,
    INT
};

class Option{
    private:
        SDL_Texture* name;
        SDL_Texture* desc;
        OptionTypes type;

        int yRelative;
    public:
        Option();
        ~Option();
        virtual void Render();
        OptionTypes GetType();
}