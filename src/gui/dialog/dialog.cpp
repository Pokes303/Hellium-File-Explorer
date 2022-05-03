#include "dialog.hpp"
#include "../../SDL_Helper.hpp"
#include "../../udplog.hpp"

Dialog::Dialog(){ }

Dialog::~Dialog(){
    ClearTitle();
    
    for (uint32_t i = 0; i < buttons.size(); i++){
        delete buttons[i];
    }
    buttons.clear();
}

void Dialog::Render(){ }

DialogType Dialog::GetType(){
    return DialogType::UNKNOWN;
}

DialogResult Dialog::GetDialogResult(){
    return result;
}

//Protected
void Dialog::GenerateButtons(int y){
    switch (dialogButtons)
    {
    case DialogButtons::NONE:
        break;
    case DialogButtons::OK:
        buttons.push_back(new Button(233, y, ButtonTypes::DialogButton1, arialBold48_font, black_col, "OK", true, true));
        break;
    case DialogButtons::CANCEL:
        buttons.push_back(new Button(233, y, ButtonTypes::DialogButton1, arialBold48_font, black_col, "CANCEL", true, true));
        break;
    case DialogButtons::NO_YES:
        buttons.push_back(new Button(233, y, ButtonTypes::DialogButton2, arialBold48_font, black_col, "NO", true, true));
        buttons.push_back(new Button(647, y, ButtonTypes::DialogButton2, arialBold48_font, black_col, "YES", true, true));
        break;
    case DialogButtons::CANCEL_OK:
        buttons.push_back(new Button(233, y, ButtonTypes::DialogButton2, arialBold48_font, black_col, "CANCEL", true, true));
        buttons.push_back(new Button(647, y, ButtonTypes::DialogButton2, arialBold48_font, black_col, "OK", true, true));
        break;
    case DialogButtons::SKIP_TRYAGAIN:
        buttons.push_back(new Button(233, y, ButtonTypes::DialogButton2, arialBold48_font, black_col, "SKIP", true, true));
        buttons.push_back(new Button(647, y, ButtonTypes::DialogButton2, arialBold48_font, black_col, "TRY AGAIN", true, true));
        break;
    default:
        LOG_E("Unknown DialogButtons value: %d", dialogButtons);
        break;
    }
}

void Dialog::SetTitle(std::string title){
    ClearTitle();

    title_tex = SDLH::GetText(arialBold80_font, black_col, title.c_str());
}

void Dialog::SetButtonActive(uint32_t i, bool active){
    if (i < buttons.size()){
        buttons[i]->SetActive(active);
    }
}

//Protected
void Dialog::ClearTitle(){
    SDLH::ClearTexture(&title_tex);
}
