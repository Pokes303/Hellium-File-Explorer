#include "dialog_textbox.hpp"
#include "../../SDL_Helper.hpp"
#include "../../udplog.hpp"
#include "../../dialog_helper.hpp"
#include "../../input.hpp"
#include "../../gui/path.hpp"

DialogTextbox::DialogTextbox(std::string title, std::string hint, DialogButtons buttons){
    SetTitle(title);
    
    dialogButtons = buttons;
    GenerateButtons(410);

    result = DialogResult::UNKNOWN_RES;

    text = "";
    this->hint = hint;
    hint_tex = SDLH::GetText(arialItalic40_font, dark_grey_col, hint.c_str());
}

DialogTextbox::~DialogTextbox(){
    ClearText();
    SDL_DestroyTexture(hint_tex);
    LOG("Destroyed textbox dialog");
}

void setTextCallback(std::string result){
    DialogTextbox* dtb = (DialogTextbox*)DialogHelper::GetDialog();
    dtb->SetText(result);
}

void DialogTextbox::Render(){
    SDLH::DrawImage(dialog_textbox_tex, 0, 0);
    SDLH::DrawImageAligned(title_tex, 1280 / 2, 210 + 10 + (110 / 2), AlignmentsX::MIDDLE_X, AlignmentsY::MIDDLE_Y);

    SDLH::DrawImage(textbox_tex, 20, 340);
    SDLH::DrawImageAligned((text == "") ? hint_tex : text_tex, 40, 340 + (60 / 2), AlignmentsX::LEFT, AlignmentsY::MIDDLE_Y);

    if (touch.status == TouchStatus::TOUCHED_DOWN &&
            !SWKBD::IsShown() &&
            touch.x > 20 && touch.x < 1280 - 20 &&
            touch.y > 340 && touch.y < 340 + 60){
        SWKBD::Appear(text, hint, (void*)setTextCallback);
    }

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

DialogType DialogTextbox::GetType(){
    return DialogType::TEXTBOX;
}

std::string DialogTextbox::GetTextboxResult(){
    return text;
}

void DialogTextbox::SetText(std::string newText){
    ClearText();

    text = newText;
    text_tex = SDLH::GetText(arial40_font, black_col, newText.c_str());
}
//Protected
void DialogTextbox::ClearText(){
    SDLH::ClearTexture(&text_tex);
}