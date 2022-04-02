#pragma once
#include "../main.hpp"
#include "button.hpp"
#include "dialog.hpp"

class DialogTextBox{
    private:
        std::string title;
        std::string text;
        std::vector<Button*> buttons;
    public:
        DialogTextBox(std::string title, DialogButtons _buttons);
        ~DialogTextBox();
        void Render();
        int GetDialogResult();
        void SetTitle(std::string _title);
        void SetText(std::string _text);
        std::string GetText(std::string _text);
};