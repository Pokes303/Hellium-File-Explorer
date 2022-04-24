#pragma once
#include "../../main.hpp"
#include "dialog.hpp"

class DialogProgressbar : public Dialog{
    protected:
        SDL_Texture* desc_tex[4];
        SDL_Texture* footer_tex;
        float progressBarPos;

        void ClearDescription();
        void ClearFooter();
    public:
        DialogProgressbar(std::string title, std::string desc, std::string footer, bool hasCancelButton);
        ~DialogProgressbar();
        void Render();
        DialogType GetType();
        void SetDescription(std::string desc);
        void SetFooter(std::string footer);
        void SetProgressBar(float pos);
};