#include "menu.hpp"

Menu::Menu(){
}
Menu::~Menu(){
    for (uint32_t i = 0; i < bv.size(); i++){
        delete bv[i];
    }
    bv.clear();
}

Button* Menu::AddButton(Button* b){
    bv.push_back(b);
    return b;
}

void Menu::RenderAll(){
    for (uint32_t i = 0; i < bv.size(); i++){
        bv[i]->Render();
    }
}