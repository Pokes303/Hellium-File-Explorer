#pragma once
#include "../main.hpp"

enum ButtonTypes{
    Button0 = 0,
    Button1,
    Button2,
    Button3,
    Button4,
    Checkbox,
    DialogButton1,
    DialogButton2
};

class Button{
    private:
        int x, y;
        int w, h;
        ButtonTypes type;
        SDL_Texture* button_tex;
        SDL_Texture* tex;
        bool isTextButton;
        bool active;
        bool isDialogButton;
    public:
        Button();
        Button(int _x, int _y, ButtonTypes _type, SDL_Texture* _tex, bool _active, bool _isDialogButton);
        Button(int _x, int _y, ButtonTypes _type, TTF_Font* _font, SDL_Color _color, std::string _text, bool _active, bool _isDialogButton);
        ~Button();

        void Render();
        bool IsTouched();
        bool IsInsideButton(int _x, int _y);

        void SetActive(bool _active);
        void SetTexture(SDL_Texture* _tex);

        SDL_Rect GetRect();
};