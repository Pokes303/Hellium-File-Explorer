#include "button.hpp"
#include "../SDL_Helper.hpp"
#include "../udplog.hpp"
#include "../input.hpp"
#include "../dialog_helper.hpp"

Button::Button(){ }
Button::Button(int _x, int _y, ButtonTypes _type, SDL_Texture* _tex, bool _active, bool _isDialogButton){
    x = _x;
    y = _y;
    type = _type;
    tex = _tex;
    SetActive(_active);

    if (_type == ButtonTypes::Checkbox)
        w = h = 75;
    else
        SDL_QueryTexture(button_tex, nullptr, nullptr, &w, &h);

    isDialogButton = _isDialogButton;
    isTextButton = false;
}

Button::Button(int _x, int _y, ButtonTypes _type, TTF_Font* _font, SDL_Color _color, std::string _text, bool _active, bool _isDialogButton){
    x = _x;
    y = _y;
    type = _type;
    tex = SDLH::GetText(_font, _color, _text.c_str());
    SetActive(_active);

    if (_type == ButtonTypes::Checkbox)
        w = h = 75;
    else
        SDL_QueryTexture(button_tex, nullptr, nullptr, &w, &h);

    isDialogButton = _isDialogButton;
    isTextButton = true;
}

Button::~Button(){
    if (isTextButton){
        SDL_DestroyTexture(tex);
    }
}

void Button::Render(){
    if (button_tex)
        SDLH::DrawImage(button_tex, x, y);
    SDLH::DrawImageAligned(tex, x + w / 2, y + h / 2, AlignmentsX::MIDDLE_X, AlignmentsY::MIDDLE_Y);
}

bool Button::IsTouched(){
    if (active &&
            touch.status == TouchStatus::TOUCHED_UP &&
            (isDialogButton == DialogHelper::DialogExists()) &&
            !SWKBD::IsShown() &&
            touch.x >= x && touch.x <= x + w &&
            touch.y >= y && touch.y <= y + h){
        Mix_PlayChannel(0, click_sound_ch, 0);
        LOG("Is touched");
        return true;
    }
    return false;
}

void Button::SetActive(bool _active){
    active = _active;
    switch (type)
    {
    case ButtonTypes::Button0:
        button_tex = (active) ? button0_tex : button0_deactivated_tex;
        break;
    case ButtonTypes::Button1:
        button_tex = (active) ? button1_tex : button1_deactivated_tex;
        break;
    case ButtonTypes::Button2:
        button_tex = (active) ? button2_tex : button2_deactivated_tex;
        break;
    case ButtonTypes::Button3:
        button_tex = (active) ? button3_tex : button3_deactivated_tex;
        break;
    case ButtonTypes::Button4:
        button_tex = (active) ? button4_tex : button4_deactivated_tex;
        break;
    case ButtonTypes::Checkbox:
        button_tex = nullptr;
        break;
    case ButtonTypes::DialogButton1:
        button_tex = (active) ? dialog_button1_tex : dialog_button1_deactivated_tex;
        break;
    case ButtonTypes::DialogButton2:
        button_tex = (active) ? dialog_button2_tex : dialog_button2_deactivated_tex;
        break;
    default:
        LOG_E("Unknown button type (%d)", type);
        break;
    }
}

void Button::SetTexture(SDL_Texture* _tex){
    tex = _tex;
}

SDL_Rect Button::GetRect(){
    SDL_Rect rect = { x, y, w, h};
    return rect;
}