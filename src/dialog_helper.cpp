#include "dialog_helper.hpp"
#include "bubbles.hpp"

Dialog* dialog;

void DialogHelper::SetDialog(Dialog* _dialog){
    if (DialogExists())
        DialogHelper::DestroyDialog();
    dialog = _dialog;
}

Dialog* DialogHelper::GetDialog(){
    return dialog;
}

void DialogHelper::DestroyDialog(){
    if (DialogExists()){
        delete dialog;
        dialog = nullptr;
    }
}

bool DialogHelper::DialogExists(){
    return dialog != nullptr;
}

void DialogHelper::RenderIfDialogExists(){
    if (DialogExists()){
        renderBubbles();
        dialog->Render();
    }
}

DialogResult DialogHelper::WaitForDialogResponse(){
    do {} while(!dialog || dialog->GetDialogResult() == DialogResult::UNKNOWN_RES);
    DialogResult res = dialog->GetDialogResult();
    DestroyDialog();
    return res;
}