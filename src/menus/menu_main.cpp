#include "menu_main.hpp"
#include "../SDL_Helper.hpp"
#include "../gui/menu.hpp"
#include "../filesystem.hpp"
#include "../udplog.hpp"
#include "../utils.hpp"

int previousTicks = 0;
float timeDelta = 0.0;

SDL_Texture* path_tex;

float pathTimer = 0.0;
float pathX = 389.0;
float pathAnimSpeed = 100.0;
uint8_t pathAlpha = 0;

SDL_Texture* checkedItems_tex;
SDL_Texture* permissions_tex;

bool firstTouchedSlider = false;
float sliderSpeed = 0.0175;

//Variables
uint32_t selectedItems = 0;
std::string folderPerms = "";
std::string directoryInfo = "";

Dialog* d;

uint32_t loadingIconTicks = 0;
double loadingIconAngle = 0;

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
    Menu main_m;

    //Create buttons
    back_b = main_m.AddButton(new Button(10, 10, ButtonTypes::Button0, arrow_left_tex, false, 0));
    next_b = main_m.AddButton(new Button(95, 10, ButtonTypes::Button0, arrow_right_tex, false, 0));
    rewind_b = main_m.AddButton(new Button(190, 10, ButtonTypes::Button0, arrow_up_tex, false, 0));
    checkbox_b = main_m.AddButton(new Button(283, 8, ButtonTypes::Checkbox, op_checkbox_false_tex, true, 0));

    open_b = main_m.AddButton(new Button(10, 250, ButtonTypes::Button3, open_tex, false, 0));

    newFile_b = main_m.AddButton(new Button(10, 350, ButtonTypes::Button2, file_new_tex, true, 0));
    newFolder_b = main_m.AddButton(new Button(155, 350, ButtonTypes::Button2, folder_new_tex, true, 0));

    copy_b = main_m.AddButton(new Button(10, 450, ButtonTypes::Button1, copy_tex, false, 0));
    cut_b = main_m.AddButton(new Button(109, 450, ButtonTypes::Button1, cut_tex, false, 0));
    paste_b = main_m.AddButton(new Button(201, 450, ButtonTypes::Button1, paste_tex, false, 0));

    delete_b = main_m.AddButton(new Button(10, 540, ButtonTypes::Button2, delete_tex, false, 0));
    rename_b = main_m.AddButton(new Button(155, 540, ButtonTypes::Button2, rename_tex, false, 0));

    settings_b = main_m.AddButton(new Button(10, 630, ButtonTypes::Button1, settings_tex, true, 0));
    properties_b = main_m.AddButton(new Button(109, 630, ButtonTypes::Button4, properties_tex, false, 0));

    //Do filesystem read
    Filesystem::ReadDir();

    //Initialize timedelta
    previousTicks = SDL_GetTicks();

    LOG("Entering main menu loop...");
    while(WHBProcIsRunning()){
        timeDelta = (float)(SDL_GetTicks() - previousTicks) / 1000.0;
        previousTicks = SDL_GetTicks();

	    SDL_RenderClear(renderer);
        readInput();

        if (vpad.trigger & VPAD_BUTTON_B || rewind_b->IsTouched()){
            Filesystem::Rewind();
        }

        //Files
        for (uint32_t i = 0; i < files.size(); i++){
            files[i]->Render(i);
            if (layer == 0)
                files[i]->CheckSelection(i);
        }

        //Slider
        SDLH::DrawImage(slider_path_tex, 1180, 100);
        if (files.size() * 100 < 720 - 100){
            SDLH::DrawImage(button_slider_deactivated_tex, 1180, 100);
        }
        else
        {
            SDLH::DrawImage(button_slider_tex, 1180, sliderY);
            if (touchStatus != 0 &&
                vpad.tpNormal.x > 1180 && vpad.tpNormal.x < 1280 &&
                vpad.tpNormal.y > 100 && vpad.tpNormal.y < 720){
                if (touchStatus == 1){
                    firstTouchedSlider = true;
                }
                else if (touchStatus == 2 && firstTouchedSlider && layer == 0){
                    if (vpad.tpNormal.y < 150)
                        slider = 0;
                    else if (vpad.tpNormal.y > 720 - 50)
                        slider = 1;
                    else
                        slider = ((float)vpad.tpNormal.y - 150.0) / (720.0 - 50.0 - 150.0);
                    sliderY = 100 + slider * (720 - 100 - 100);
                }
            }
            else if (touchStatus == 0){
                firstTouchedSlider = false;

                if (abs(vpad.rightStick.y) > 0.25 && layer == 0){
                    slider -= vpad.rightStick.y * sliderSpeed;

                    if (slider > 1)
                        slider = 1;
                    else if (slider < 0)
                        slider = 0;
                    sliderY = 100 + slider * (720 - 100 - 100);
                }
            }
        }

        ////Left menu
        SDLH::DrawImage(menu_left_tex, 0, 100);
        //Draw directory info
        SDLH::DrawImage(checked_items, 0, 100);
        SDLH::DrawImage(checkedItems_tex, 80, 115);
        SDLH::DrawAlignedImage(permissions_tex, 147, 178, Alignments::MIDDLE);

        ////Upper menu
        SDLH::DrawImage(path_bottom, 370, 0);
        //Draw cwd
        if (pathAnimation){
            switch (pathAnimationPhase) {
                case 0: //3 seconds wait
                case 2: //another 3 seconds wait
                    pathTimer += timeDelta;
                    if (pathTimer > 3.0){
                        pathTimer = 0;
                        pathAnimationPhase = (pathAnimationPhase == 0) ? 1 : 3;
                    }
                    break;
                case 1: //Moving leftwards
                    pathX -= timeDelta * pathAnimSpeed;
                    if (pathX + pathTextW <= 1250){
                        pathX = 1250 - pathTextW;
                        pathAnimationPhase = 2;
                    }
                    break;
                case 3:
                    pathTimer += timeDelta;
                    if (pathTimer >= 0.0 && pathTimer < 0.5)
                        pathAlpha = 255 - ((pathTimer * 2) * 255);
                    else if (pathTimer >= 0.5 && pathTimer < 1){
                        pathAlpha = ((pathTimer - 0.5) * 2) * 255;
                        if (pathX != 0)
                            pathX = 389.0;
                    }
                    else if (pathTimer >= 1){
                        pathAlpha = 255;
                        pathAnimationPhase = 0;
                    }
	                SDL_SetTextureAlphaMod(path_tex, pathAlpha);
                    break;
            }
            SDLH::DrawImage(path_tex, (int)pathX, (pathType == 0) ? 25 : 10);
            SDLH::DrawImage(path_shadow, 370, 0);
        }
        else
            SDLH::DrawImage(path_tex, 389, (pathType == 0) ? 25 : 10);
        //Draw menu after the path
        SDLH::DrawImage(menu_up_tex, 0, 0);
        
        switch (pathType)
        {
        case 0:
            break;
        case 1:
            SDLH::DrawText(arial25_font, 379, 65, Alignments::LEFT, dark_red_col, "Virtual directory");
            break;
        case 2:
            SDLH::DrawText(arial25_font, 379, 65, Alignments::LEFT, dark_red_col, "IOSUHAX directory");
        default:
            LOG("[filesystem.cpp]>Error: Unknown pathType value (%d)", pathType);
            break;
        }

        //Buttons
        main_m.RenderAll();

        if (back_b->IsTouched()){
            previousPathPos--;
            path = previousPaths[previousPathPos];

            next_b->SetActive(true);
            if (previousPathPos <= 0) {
                back_b->SetActive(false);
            }
            Filesystem::ReadDir();
        }
        if (next_b->IsTouched()){
            previousPathPos++;
            path = previousPaths[previousPathPos];

            back_b->SetActive(true);
            if (previousPathPos >= previousPaths.size()){
                next_b->SetActive(false);
            }
            Filesystem::ReadDir();
        }
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

        if (copy_b->IsTouched()){
            Filesystem::Copy(false);
        }
        if (cut_b->IsTouched()){
            Filesystem::Copy(true);
        }
        if (paste_b->IsTouched()){
            Filesystem::Paste();
        }

        if (delete_b->IsTouched()){
            Filesystem::Delete();
        }

        /*loadingIconTicks++;
        if (loadingIconTicks >= 5){
            loadingIconTicks = 0;
            loadingIconAngle += 45;
            if (loadingIconAngle >= 360)
                loadingIconAngle = 0;
        }
        SDLH::DrawImageRotate(loading_tex, 1280 - 10 - 100, 720 - 10 - 100, loadingIconAngle);*/

        if (vpad.trigger & VPAD_BUTTON_TV){
            SDLH::TakeScreenshot("/vol/external01/screen.bmp");
            LOG("[menu_main.cpp]>Log: Screenshot saved as /vol/external01/screen.bmp");
        }


        if (d != nullptr){
            d->Render();
        }

        Utils::DrawFPS();
        
	    SDL_RenderPresent(renderer);
    }
    Filesystem::ClearDir();
}
