#include "dialog.hpp"
#include "../SDL_Helper.hpp"
#include "../udplog.hpp"

Dialog::Dialog(std::string _title, std::string _desc, std::string _footer, DialogButtons _buttons, bool hasProgressBar){
    layer++;

    title = _title;
    desc = _desc;
    footer = _footer;

    progressBarPos = (hasProgressBar) ? 0 : -1;
    thisLayer = layer;
    
    switch (_buttons)
    {
    case DialogButtons::NONE:
        break;
    case DialogButtons::OK:
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton1, dialog_ok_tex, true, thisLayer));
        break;
    case DialogButtons::CANCEL:
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton1, dialog_cancel_tex, true, thisLayer));
        break;
    case DialogButtons::NO_YES:
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton2, dialog_no_tex, true, thisLayer));
        buttons.push_back(new Button(647, 500, ButtonTypes::DialogButton2, dialog_yes_tex, true, thisLayer));
        break;
    default:
        LOG("[dialog.cpp]>Error: Unknown DialogButtons value: %d", _buttons);
        break;
    }

    result = -1;
}
Dialog::~Dialog(){
    for (uint32_t i = 0; i < buttons.size(); i++){
        delete buttons[i];
    }
    buttons.clear();
    layer--;
}

void Dialog::Render(){
    SDL_DrawImage(dialog_tex, 0, 0);
    SDL_DrawText(arialBold80_font, 20, 145, Alignments::LEFT, black_col, title.c_str());
    SDL_DrawText(arial40_font, 20, 250, Alignments::LEFT, black_col, desc.c_str());
    SDL_DrawText(arial40_font, 1280 / 2, 450, Alignments::MIDDLE, dark_grey_col, footer.c_str());

    if (progressBarPos >= 0){
        SDL_DrawImage(dialog_progress_bar_tex, 10, 410);
        SDL_DrawImageCut(dialog_progress_bar_status_tex, 10, 410, (int)(progressBarPos * (1280 - 20)), 40);

        SDL_DrawTextf(arial40_font, 1280 - 20, 370, Alignments::RIGHT, black_col, "%.0f%%", progressBarPos * 100.0);
    }

    for (uint32_t i = 0; i < buttons.size(); i++){
        buttons[i]->Render();
        if (buttons[i]->IsTouched())
            result = i;
    }
}

int Dialog::GetDialogResult(){
    return result;
}

void Dialog::UpdateTitle(std::string _title){
    title = _title;
}

void Dialog::UpdateDescription(std::string _desc){
    desc = _desc;
}

void Dialog::UpdateProgressBar(float pos){
    progressBarPos = pos;
}

void Dialog::UpdateFooter(std::string _footer){
    footer = _footer;
}