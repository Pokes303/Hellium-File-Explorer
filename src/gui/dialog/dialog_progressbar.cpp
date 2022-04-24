#include "dialog_progressbar.hpp"
#include "../../SDL_Helper.hpp"
#include "../../udplog.hpp"
#include "../../utils.hpp"

DialogProgressbar::DialogProgressbar(std::string title, std::string desc, std::string footer, bool hasCancelButton){
    SetTitle(title);
    SetDescription(desc);
    SetFooter(footer);
    progressBarPos = 0.0f;

    if (hasCancelButton)
        buttons.push_back(new Button(233, 500, ButtonTypes::DialogButton1, arialBold48_font, black_col, "CANCEL", true, true));

    result = DialogResult::UNKNOWN_RES;
}

DialogProgressbar::~DialogProgressbar(){
    ClearDescription();
    ClearFooter();
    LOG("Destroyed progressbar dialog");
}

void DialogProgressbar::Render(){
    SDLH::DrawImage(dialog_tex, 0, 0);
    SDLH::DrawImage(dialog_tex, 0, 0);
    SDLH::DrawImageAligned(title_tex, 20 / 2, 210 + 10 + (110 / 2), AlignmentsX::LEFT, AlignmentsY::MIDDLE_Y);
    for (int i = 0; i < 4; i++){
        if (desc_tex[i])
            SDLH::DrawImage(desc_tex[i], 20, 250 + i * 45);
    }
    SDLH::DrawImageAligned(footer_tex, 1280 / 2, 450, AlignmentsX::MIDDLE_X);

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

DialogType Dialog::GetType(){
    return DialogType::PROGRESSBAR;
}

void DialogProgressbar::SetDescription(std::string desc){
    ClearDescription();

    std::vector<std::string> descLines = Utils::SplitString(desc, '\n');
    int iterations = (descLines.size() < 4) ? descLines.size() : 4;
    for (int i = 0; i < iterations; i++){
        desc_tex[i] = SDLH::GetText(arial40_font, black_col, descLines[i].c_str());
    }
}

void DialogProgressbar::SetFooter(std::string footer){
    ClearFooter();

    footer_tex = SDLH::GetText(arial40_font, dark_grey_col, footer.c_str());
}

void DialogProgressbar::SetProgressBar(float pos){
    progressBarPos = pos;
}

//Protected
void DialogProgressbar::ClearDescription(){
    for (int i = 0; i < 4; i++){
        SDLH::ClearTexture(&desc_tex[i]);
    }
}

void DialogProgressbar::ClearFooter(){
    SDLH::ClearTexture(&footer_tex);
}