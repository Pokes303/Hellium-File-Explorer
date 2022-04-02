#pragma once
#include "../../main.hpp"
#include "dialog.hpp"

class DialogProgressbar : public Dialog{
    protected:
        std::string desc;
        std::string footer;
        float progressBarPos;
    public:
        DialogProgressbar(std::string _title, std::string _desc, std::string _footer, bool hasCancelButton);
        ~DialogProgressbar();
        void Render();
        void UpdateDescription(std::string _desc);
        void UpdateFooter(std::string _footer);
        void UpdateProgressBar(float pos);
};