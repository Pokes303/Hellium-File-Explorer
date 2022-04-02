#pragma once
#include "../../main.hpp"
#include "dialog.hpp"

class DialogDefault : public Dialog{
    protected:
        std::string desc;
        std::string footer;
    public:
        DialogDefault(std::string _title, std::string _desc, std::string _footer, DialogButtons _buttons);
        ~DialogDefault();
        void Render();
        void UpdateDescription(std::string _desc);
        void UpdateFooter(std::string _footer);
};