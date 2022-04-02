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

class Dialog{
    protected:
        std::string title;
        DialogButtons dialogButtons;
        std::vector<Button*> buttons;
        DialogResult result;
    public:
        Dialog();
        virtual ~Dialog();
        virtual void Render();
        DialogResult GetDialogResult();
        void UpdateTitle(std::string _title);
};