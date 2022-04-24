#pragma once
#include "../../main.hpp"
#include "../button.hpp"

enum DialogButtons{
    NONE,
    OK,
    CANCEL,
    NO_YES,
    CANCEL_OK,
};

enum DialogResult{
    UNKNOWN_RES,
    OK_RES,
    CANCELLED_RES,
    YES_RES,
    NO_RES
};

enum DialogType{
    UNKNOWN,
    DEFAULT,
    PROGRESSBAR,
    TEXTBOX
};

class Dialog{
    protected:
        SDL_Texture* title_tex = nullptr;
        DialogButtons dialogButtons;
        std::vector<Button*> buttons;
        DialogResult result;

        void GenerateButtons(int y);
        void ClearTitle();
    public:
        Dialog();
        virtual ~Dialog();
        virtual void Render();
        virtual DialogType GetType();
        DialogResult GetDialogResult();
        void SetTitle(std::string title);
        void SetButtonActive(uint32_t i, bool active);
};