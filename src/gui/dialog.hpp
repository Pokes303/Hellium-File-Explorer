#pragma once
#include "../main.hpp"
#include "button.hpp"

enum DialogButtons{
    NONE = 0,
    OK,
    CANCEL,
    NO_YES
};

class Dialog{
    private:
        std::string title;
        std::string desc;
        float progressBarPos;
        std::string footer;
        std::vector<Button*> buttons;
        int thisLayer;
        int result;
    public:
        Dialog(std::string _title, std::string _desc, std::string _footer, DialogButtons _buttons, bool hasProgressBar);
        ~Dialog();
        void Render();
        int GetDialogResult();
        void UpdateTitle(std::string _title);
        void UpdateDescription(std::string _desc);
        void UpdateProgressBar(float pos);
        void UpdateFooter(std::string _footer);
};