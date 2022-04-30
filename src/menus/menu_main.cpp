#include "menu_main.hpp"
#include "../SDL_Helper.hpp"
#include "../filesystem_helper.hpp"
#include "../udplog.hpp"
#include "../utils.hpp"
#include "../bubbles.hpp"
#include "../input.hpp"
#include "../dialog_helper.hpp"
#include "../gui/path.hpp"

int previousTicks = 0;
float timeDelta = 0.0;

SDL_Texture* directoryInfo1;
SDL_Texture* directoryInfo2;
SDL_Texture* directoryInfoExtra;
SDL_Texture* checkedItems_tex;
SDL_Texture* permissions_tex;

bool firstTouchedSlider = false;
float sliderSpeed = 0.0175;

//Variables
uint32_t selectedItems = 0;
std::string folderPerms = "";

uint32_t loadingIconTicks = 0;
double loadingIconAngle = 0;

//Menu
Menu* main_m = nullptr;

//Buttons
Button* back_b;
Button* next_b;
Button* rewind_b;
Button* checkbox_b;

Button* open_b;

Button* newFile_b;
Button* newFolder_b;

Button* copy_b;
Button* cut_b;
Button* paste_b;

Button* delete_b;
Button* rename_b;

Button* settings_b;
Button* properties_b;

void loadMenu_Main(){
    main_m = new Menu();

    //Create buttons
    back_b = main_m->AddButton(new Button(10, 10, ButtonTypes::Button0, arrow_left_tex, false, 0));
    next_b = main_m->AddButton(new Button(95, 10, ButtonTypes::Button0, arrow_right_tex, false, 0));
    rewind_b = main_m->AddButton(new Button(190, 10, ButtonTypes::Button0, arrow_up_tex, false, 0));
    checkbox_b = main_m->AddButton(new Button(283, 8, ButtonTypes::Checkbox, op_checkbox_false_tex, true, 0));

    open_b = main_m->AddButton(new Button(10, 250, ButtonTypes::Button3, open_tex, false, 0));

    newFile_b = main_m->AddButton(new Button(10, 350, ButtonTypes::Button2, file_new_tex, true, 0));
    newFolder_b = main_m->AddButton(new Button(155, 350, ButtonTypes::Button2, folder_new_tex, true, 0));

    copy_b = main_m->AddButton(new Button(10, 450, ButtonTypes::Button1, copy_tex, false, 0));
    cut_b = main_m->AddButton(new Button(109, 450, ButtonTypes::Button1, cut_tex, false, 0));
    paste_b = main_m->AddButton(new Button(201, 450, ButtonTypes::Button1, paste_tex, false, 0));

    delete_b = main_m->AddButton(new Button(10, 540, ButtonTypes::Button2, delete_tex, false, 0));
    rename_b = main_m->AddButton(new Button(155, 540, ButtonTypes::Button2, rename_tex, false, 0));

    settings_b = main_m->AddButton(new Button(10, 630, ButtonTypes::Button1, settings_tex, true, 0));
    properties_b = main_m->AddButton(new Button(109, 630, ButtonTypes::Button4, properties_tex, false, 0));

    //Do filesystem read
    Path::SetPath("/vol/external01/");

    //Initialize timedelta
    previousTicks = SDL_GetTicks();

    LOG("Entering main menu loop...");
    while(WHBProcIsRunning()){
        timeDelta = (float)(SDL_GetTicks() - previousTicks) / 1000.0;
        previousTicks = SDL_GetTicks();

	    SDL_RenderClear(renderer);

        SDLH::DrawImage(bg_tex, 0, 0);
        if (!DialogHelper::DialogExists())
            renderBubbles();

        Input::ReadInput();

        //Files
        if (files.size() > 0){
            for (uint32_t i = 0; i < files.size(); i++){
                files[i]->Render(i);
                files[i]->CheckSelection(i);
            }
        }
        else{
            SDLH::DrawImageAligned(directoryInfo1, 300 + (1280 - 300) / 2, 140, AlignmentsX::MIDDLE_X);
            if (directoryInfo2)
                SDLH::DrawImageAligned(directoryInfo2, 300 + (1280 - 300) / 2, 140 + 50, AlignmentsX::MIDDLE_X);
            if (directoryInfoExtra)
                SDLH::DrawImageAligned(directoryInfoExtra, 300 + (1280 - 300) / 2, 140 + 100 + 50, AlignmentsX::MIDDLE_X);
        }

        //Slider
        if (files.size() * 100 < 720 - 100){
            SDLH::DrawImage(slider_path_deactivated_tex, 1180, 100);
            SDLH::DrawImage(button_slider_deactivated_tex, 1180, 100);
        }
        else
        {
            SDLH::DrawImage(slider_path_tex, 1180, 100);
            if (touch.status != TouchStatus::NOT_TOUCHED &&
                !DialogHelper::DialogExists() &&
                !SWKBD::IsShown() &&
                touch.x > 1180 && touch.x < 1280 &&
                touch.y > 100 && touch.y < 720){
                if (touch.status == TouchStatus::TOUCHED_DOWN){
                    firstTouchedSlider = true;
                }
                else if (touch.status == TouchStatus::TOUCHED_HELD && firstTouchedSlider && !DialogHelper::DialogExists()){
                    if (touch.y < 150)
                        slider = 0;
                    else if (touch.y > 720 - 50)
                        slider = 1;
                    else
                        slider = ((float)touch.y - 150.0) / (720.0 - 50.0 - 150.0);
                    sliderY = 100 + slider * (720 - 100 - 100);
                }
            }
            else if (touch.status == TouchStatus::NOT_TOUCHED){
                firstTouchedSlider = false;

                if (abs(vpad.rightStick.y) > 0.25 && !DialogHelper::DialogExists()){
                    slider -= vpad.rightStick.y * sliderSpeed;

                    if (slider > 1)
                        slider = 1;
                    else if (slider < 0)
                        slider = 0;
                    sliderY = 100 + slider * (720 - 100 - 100);
                }
            }
            SDLH::DrawImage(button_slider_tex, 1180, sliderY);
        }

        ////Left menu
        SDLH::DrawImage(menu_left_tex, 0, 100);
        //Draw directory info
        SDLH::DrawImage(checked_items, 0, 109);
        SDLH::DrawImage(checkedItems_tex, 80, 115);
        SDLH::DrawImageAligned(permissions_tex, 147, 170, AlignmentsX::MIDDLE_X);

        ////Upper menu
        SDLH::DrawImage(path_bottom, 370, 0);
        Path::Render();
        SDLH::DrawImage(menu_up_tex, 0, 0);

        //Buttons
        main_m->RenderAll();

        if (vpad.trigger & VPAD_BUTTON_B || rewind_b->IsTouched()){
            FilesystemHelper::RewindPath();
        }

        if (vpad.trigger & VPAD_BUTTON_L || back_b->IsTouched())
            Path::PreviousPath();
        else if (vpad.trigger & VPAD_BUTTON_R || next_b->IsTouched())
            Path::NextPath();

        if (checkbox_b->IsTouched() && files.size() > 0){
            uint32_t f = 0;
            for (uint32_t i = 0; i < files.size(); i++){
                if (files[i]->IsSelected()){
                    f++;
                }
            }
            if (f > 0) {
                for (uint32_t i = 0; i < files.size(); i++){
                    files[i]->SetSelection(false);
                }
                checkbox_b->SetTexture(op_checkbox_false_tex);
                selectedItems = 0;
            }
            else {
                for (uint32_t i = 0; i < files.size(); i++){
                    files[i]->SetSelection(true);
                }
                checkbox_b->SetTexture(op_checkbox_true_tex);
                selectedItems = files.size();
            }
        }

        if (newFile_b->IsTouched()){
            FilesystemHelper::CreateFileProccess();
        }
        if (newFolder_b->IsTouched()){

        }

        if (copy_b->IsTouched()){
            FilesystemHelper::CopyProccess(false);
        }
        if (cut_b->IsTouched()){
            FilesystemHelper::CopyProccess(true);
        }
        if (paste_b->IsTouched()){
            FilesystemHelper::PasteProccess();
        }

        if (delete_b->IsTouched()){
            FilesystemHelper::DeleteProccess();
        }

        if (rename_b->IsTouched()){
            FilesystemHelper::RenameProccess();
        }

        DialogHelper::RenderIfDialogExists();

        if (SWKBD::IsShown()){
            SWKBD::Render();
            nn::swkbd::DrawDRC();
            nn::swkbd::DrawTV();
        }

        //Debug fps draw
        Utils::DrawFPS();
	    SDL_RenderPresent(renderer);
        
        //Debug screenshot
        if (vpad.trigger & VPAD_BUTTON_TV){
            SDLH::TakeScreenshot("/vol/external01/screen.bmp");
            LOG("Screenshot saved as /vol/external01/screen.bmp");
        }
    }
    FilesystemHelper::ClearPathDir();
    delete main_m;
}
