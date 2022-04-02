#include "dialog.hpp"
#include "../../SDL_Helper.hpp"
#include "../../udplog.hpp"

Dialog::Dialog(){

}

Dialog::~Dialog(){
    for (uint32_t i = 0; i < buttons.size(); i++){
        delete buttons[i];
    }
    buttons.clear();
    LOG("Destroyed dialog buttons");
}

void Dialog::Render(){
    
}

DialogResult Dialog::GetDialogResult(){
    return result;
}

void Dialog::UpdateTitle(std::string _title){
    title = _title;
}

