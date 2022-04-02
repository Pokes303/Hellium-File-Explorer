#include "dialog_default.hpp"
#include "../../SDL_Helper.hpp"
#include "../../udplog.hpp"

DialogDefault::DialogDefault(std::string _title, std::string _desc, std::string _footer, DialogButtons _buttons){
    title = _title;
    desc = _desc;
    footer = _footer;
    
    dialogButtons = _buttons;
    switch (dialogButtons)
    {
    case DialogButtons::NONE:
        break;
    case DialogButtons::OK:
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton1, arialBold48_font, black_col, "OK", true, true));
        break;
    case DialogButtons::CANCEL:
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton1, arialBold48_font, black_col, "CANCEL", true, true));
        break;
    case DialogButtons::NO_YES:
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton2, arialBold48_font, black_col, "NO", true, true));
        buttons.push_back(new Button(647, 500, ButtonTypes::DialogButton2, arialBold48_font, black_col, "YES", true, true));
    case DialogButtons::CANCEL_OK:
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton2, arialBold48_font, black_col, "CANCEL", true, true));
        buttons.push_back(new Button(647, 500, ButtonTypes::DialogButton2, arialBold48_font, black_col, "OK", true, true));
        break;
    default:
        LOG("[dialog.cpp]>Error: Unknown DialogButtons value: %d", _buttons);
        break;
    }

    result = DialogResult::UNKNOWN_RES;
}

DialogDefault::~DialogDefault(){
    LOG("Destroying default");
}

void DialogDefault::Render(){
    SDLH::DrawImage(dialog_tex, 0, 0);
    SDLH::DrawText(arialBold80_font, 20, 145, AlignmentsX::LEFT, black_col, title.c_str());
    SDLH::DrawText(arial40_font, 20, 250, AlignmentsX::LEFT, black_col, desc.c_str());
    SDLH::DrawText(arial40_font, 1280 / 2, 450, AlignmentsX::MIDDLE_X, dark_grey_col, footer.c_str());

    for (uint32_t i = 0; i < buttons.size(); i++){
        buttons[i]->Render();
        if (buttons[i]->IsTouched())
            switch(dialogButtons){
                case DialogButtons::OK:
                    result = DialogResult::OK_RES;
                    break;
                case DialogButtons::CANCEL:
                    result = DialogResult::CANCELLED_RES;
                    break;
                case DialogButtons::NO_YES:
                    result = i == 1 ? DialogResult::YES_RES : DialogResult::NO_RES;
                    break;
                case DialogButtons::CANCEL_OK:
                    result = i == 1 ? DialogResult::OK_RES : DialogResult::CANCELLED_RES;
                    break;
                default:
                    LOG_E("Unknown DialogButton case (%d). Will be set to CANCELLED", dialogButtons);
                    result = DialogResult::CANCELLED_RES;
                    break;
            }
    }
}

void DialogDefault::UpdateDescription(std::string _desc){
    desc = _desc;
}

void DialogDefault::UpdateFooter(std::string _footer){
    footer = _footer;
}