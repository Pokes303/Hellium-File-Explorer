#include "dialog_progressbar.hpp"
#include "../../SDL_Helper.hpp"
#include "../../udplog.hpp"

DialogProgressbar::DialogProgressbar(std::string _title, std::string _desc, std::string _footer, bool hasCancelButton){
    title = _title;
    desc = _desc;
    footer = _footer;
    progressBarPos = 0.0f;

    if (hasCancelButton)
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton1, arialBold48_font, black_col, "CANCEL", true, true));

    result = DialogResult::UNKNOWN_RES;
}

DialogProgressbar::~DialogProgressbar(){
    LOG("Destroying progressbar dialog");
}

void DialogProgressbar::Render(){
    SDLH::DrawImage(dialog_tex, 0, 0);
    SDLH::DrawText(arialBold80_font, 20, 145, AlignmentsX::LEFT, black_col, title.c_str());
    SDLH::DrawText(arial40_font, 20, 250, AlignmentsX::LEFT, black_col, desc.c_str());
    SDLH::DrawText(arial40_font, 1280 / 2, 450, AlignmentsX::MIDDLE_X, dark_grey_col, footer.c_str());

    //Progressbar
    SDLH::DrawImage(dialog_progress_bar_tex, 10, 410);
    SDLH::DrawImageCut(dialog_progress_bar_status_tex, 10, 410, (int)(progressBarPos * (1280 - 20)), 40);

    SDLH::DrawTextf(arial40_font, 1280 - 20, 370, AlignmentsX::RIGHT, black_col, "%.0f%%", progressBarPos * 100.0);

    if (buttons.size() > 0){
        buttons[0]->Render();
        if (buttons[0]->IsTouched()){
            result = DialogResult::CANCELLED_RES;
        }
    }
}

void DialogProgressbar::UpdateDescription(std::string _desc){
    desc = _desc;
}

void DialogProgressbar::UpdateFooter(std::string _footer){
    footer = _footer;
}

void DialogProgressbar::UpdateProgressBar(float pos){
    progressBarPos = pos;
}