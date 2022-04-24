#pragma once
#include "../../main.hpp"
#include "dialog.hpp"

class DialogDefault : public Dialog{
    protected:
        SDL_Texture* desc_tex[4] = { nullptr, nullptr, nullptr, nullptr };
        SDL_Texture* footer_tex = nullptr;

        void ClearDescription();
        void ClearFooter();
    public:
        DialogDefault(std::string title, std::string desc, std::string footer, DialogButtons buttons);
        ~DialogDefault();
        void Render();
        DialogType GetType();
        void SetDescription(std::string desc);
        void SetFooter(std::string footer);
};