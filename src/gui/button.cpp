#include "button.hpp"
#include "../SDL_Helper.hpp"
#include "../udplog.hpp"

Button::Button(){ }
Button::Button(int _x, int _y, ButtonTypes _type, SDL_Texture* _tex, bool _active, int _layer){
    x = _x;
    y = _y;
    type = _type;
    tex = _tex;
    SetActive(_active);

    if (_type == ButtonTypes::Checkbox)
        w = h = 75;
    else
        SDL_QueryTexture(button_tex, nullptr, nullptr, &w, &h);

    buttonLayer = _layer;
}

Button::~Button(){ }

void Button::Render(){
    SDL_DrawImage(button_tex, x, y);
    SDL_DrawImage(tex, x, y);
}

bool Button::IsTouched(){
    if (active && buttonLayer == layer && touchStatus == TouchStatus::TOUCHED_UP &&
        vpad.tpNormal.x >= x && vpad.tpNormal.x <= x + w &&
        vpad.tpNormal.y >= y && vpad.tpNormal.y <= y + h){
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
        ERROR("Unknown button type (%d)", type);
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