#pragma once
#include "main.hpp"
#include "gui/dialog/dialog.hpp"

namespace DialogHelper{
    void SetDialog(Dialog* _dialog);
    Dialog* GetDialog();
    void DestroyDialog();
    bool DialogExists();
    void RenderIfDialogExists();
    DialogResult WaitForDialogResponse();
}