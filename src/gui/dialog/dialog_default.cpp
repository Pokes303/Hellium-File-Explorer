#include "dialog_default.hpp"
#include "../../SDL_Helper.hpp"
#include "../../udplog.hpp"
#include "../../utils.hpp"

DialogDefault::DialogDefault(std::string title, std::string desc, std::string footer, DialogButtons buttons){
    SetTitle(title);
    SetDescription(desc);
    SetFooter(footer);
    
    dialogButtons = buttons;
    GenerateButtons(500);

    result = DialogResult::UNKNOWN_RES;
}

DialogDefault::~DialogDefault(){
    ClearDescription();
    ClearFooter();
}

void DialogDefault::Render(){
    SDLH::DrawImage(dialog_tex, 0, 0);
    SDLH::DrawImageAligned(title_tex, 20, 120 + 10 + (110 / 2), AlignmentsX::LEFT, AlignmentsY::MIDDLE_Y);
    for (int i = 0; i < 4; i++){
        if (desc_tex[i] != nullptr)
            SDLH::DrawImage(desc_tex[i], 20, 250 + i * 45);
    }
    SDLH::DrawImageAligned(footer_tex, 1280 / 2, 450, AlignmentsX::MIDDLE_X);

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

DialogType DialogDefault::GetType(){
    return DialogType::DEFAULT;
}

void DialogDefault::SetDescription(std::string desc){
    ClearDescription();

    std::vector<std::string> descLines = Utils::SplitString(desc, '\n');
    for (uint32_t i = 0; i < 4; i++){
        desc_tex[i] = (i < descLines.size()) ? SDLH::GetText(arial40_font, black_col, descLines[i].c_str()) : nullptr;
    }
}

void DialogDefault::SetFooter(std::string footer){
    ClearFooter();

    footer_tex = SDLH::GetText(arial40_font, dark_grey_col, footer.c_str());
}

//Protected
void DialogDefault::ClearDescription(){
    for (int i = 0; i < 4; i++){
        SDLH::ClearTexture(&desc_tex[i]);
    }
}

void DialogDefault::ClearFooter(){
    SDLH::ClearTexture(&footer_tex);
}