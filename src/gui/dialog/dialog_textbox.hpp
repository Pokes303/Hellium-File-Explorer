#pragma once
#include "../../main.hpp"
#include "dialog.hpp"

class DialogTextbox : public Dialog{
    protected:
        std::string text;
        std::string hint;
        SDL_Texture* text_tex = nullptr;
        SDL_Texture* hint_tex = nullptr;

        void ClearText();
    public:
        DialogTextbox(std::string title, std::string hint, DialogButtons buttons);
        ~DialogTextbox();
        void Render();
        DialogType GetType();
        void SetText(std::string newText);
        std::string GetTextboxResult();
};