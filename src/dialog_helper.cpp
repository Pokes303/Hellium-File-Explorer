#include "dialog_helper.hpp"
#include "bubbles.hpp"
#include "udplog.hpp"
#include "gui/dialog/dialog_textbox.hpp"

Dialog* dialog;
bool destroyRequest = false;

void DialogHelper::SetDialog(Dialog* _dialog){
    if (DialogExists()){
        DialogHelper::DestroyDialog();
    }
    dialog = _dialog;
}

Dialog* DialogHelper::GetDialog(){
    return dialog;
}

void DialogHelper::DestroyDialog(){
    if (DialogExists()){
        delete dialog;
        dialog = nullptr;
        destroyRequest = false;
    }
}

bool DialogHelper::DialogExists(){
    return dialog != nullptr;
}

void DialogHelper::RenderIfDialogExists(){
    if (destroyRequest){
        DestroyDialog();
        return;
    }

    if (DialogExists()){
        renderBubbles();
        dialog->Render();
    }
}

DialogResult DialogHelper::WaitForDialogResponse(){
    do {} while(!dialog || dialog->GetDialogResult() == DialogResult::UNKNOWN_RES);
    DialogResult res = dialog->GetDialogResult();
    RequestDestroy();
    return res;
}

DialogResult DialogHelper::WaitForTextboxDialogResponse(std::string& textboxRes){
    do {} while(!dialog || dialog->GetDialogResult() == DialogResult::UNKNOWN_RES);
    DialogResult res = dialog->GetDialogResult();
    if (dialog->GetType() == DialogType::TEXTBOX){
        DialogTextbox* dtb = (DialogTextbox*)dialog;
        textboxRes = dtb->GetTextboxResult();
        LOG("Textbox result: %s", textboxRes.c_str());
    }
    else
        LOG_E("Called WaitForTextboxDialogResponse without having a DialogTextbox");
    RequestDestroy();
    return res;
}

void DialogHelper::RequestDestroy(){
    destroyRequest = true;
}