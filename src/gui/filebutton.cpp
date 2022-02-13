#include "filebutton.hpp"
#include "../SDL_Helper.hpp"
#include "../filesystem.hpp"
#include "../menus/menu_main.hpp"

/*
        SDL_Texture* name_tex;

        std::string type;
        SDL_Texture* type_tex;

        char size[128];
        SDL_Texture* size_tex;

        char date[128];
        SDL_Texture* date_tex;*/

        /*SDL_DrawText(arialBold35_font, 475, y + 14, Alignments::LEFT, black_col, name.c_str());
        SDL_DrawText(arial28_font, 475, y + 58, Alignments::LEFT, dark_grey_col, type.c_str());
        
        SDL_DrawText(arial30_font, 1180, y + 15, Alignments::RIGHT, light_grey_col, size);
        SDL_DrawText(arial30_font, 1180, y + 55, Alignments::RIGHT, light_grey_col, date);*/

FileButton::FileButton(IOSUHAX_FSA_DirectoryEntry _entry){
    entry = _entry;
    selected = false;

    icon = (entry.info.flags & FS_STAT_DIRECTORY) ? folder_tex : file_tex;
    name = entry.name;
    name_tex = SDL_GetText(arialBold35_font, black_col, name.c_str());

    if (entry.info.flags & FS_STAT_DIRECTORY){
        type = "FOLDER";
        snprintf(size, sizeof(size), " ");
    }
    else{
        std::string extension = "";
        bool point = false;
        for (int i = strlen(entry.name) - 1; i >= 0; i--){
            if (entry.name[i] == '.'){
                point = true;
                break;
            }
            extension = entry.name[i] + extension;
        }

        if (point){
            type = "";
            for (uint32_t i = 0; i < extension.size(); i++)
                type += toupper(extension[i]);
            type += " file";
        }
        else{
            type = "FILE";
        }

        if (entry.info.size < 1024)
            snprintf(size, sizeof(size), "%d B", entry.info.size);
        else if (entry.info.size >= 1024 && entry.info.size < 1024 * 1024)
            snprintf(size, sizeof(size), "%.02f KB", (float)entry.info.size / 1024.0);
        else if (entry.info.size >= 1024 * 1024 && entry.info.size < 1024 * 1024 * 1024)
            snprintf(size, sizeof(size), "%.02f MB", (float)entry.info.size / 1024.0 / 1024.0);
        else
            snprintf(size, sizeof(size), "%.02f GB", (float)entry.info.size / 1024.0 / 1024.0 / 1024.0);
    }
    type_tex = SDL_GetText(arial28_font, dark_grey_col, type.c_str());
    size_tex = SDL_GetText(arial30_font, light_grey_col, size);

    OSCalendarTime ct;
    Filesystem::FSTimeToCalendarTime(entry.info.modified, &ct);
    snprintf(date, sizeof(date), "%02d/%02d/%04d %02d:%02d", ct.tm_mday, ct.tm_mon + 1, ct.tm_year, ct.tm_hour, ct.tm_min);
    date_tex = SDL_GetText(arial30_font, light_grey_col, date);

    isDirectory = entry.info.flags & FS_STAT_DIRECTORY;
    isDrive = 0;
    devPath = "";
}

FileButton::FileButton(std::string _name, std::string _type, SDL_Texture* _icon, bool _isDirectory){
    selected = false;
    icon = _icon;

    name = _name;
    name_tex = SDL_GetText(arialBold35_font, black_col, name.c_str());

    type = _type;
    type_tex = SDL_GetText(arial28_font, dark_grey_col, type.c_str());

    snprintf(size, sizeof(size), " ");
    size_tex = SDL_GetText(arial30_font, light_grey_col, size);

    snprintf(date, sizeof(date), " ");
    date_tex = SDL_GetText(arial30_font, light_grey_col, date);

    isDirectory = _isDirectory;
    isDrive = false;
}

FileButton::FileButton(std::string _name, std::string _dev, std::string _type, SDL_Texture* _icon){
    selected = false;
    icon = _icon;

    name = _name;
    name_tex = SDL_GetText(arialBold35_font, black_col, name.c_str());

    type = _type;
    type_tex = SDL_GetText(arial28_font, dark_grey_col, type.c_str());

    snprintf(size, sizeof(size), " ");
    size_tex = SDL_GetText(arial30_font, light_grey_col, size);

    snprintf(date, sizeof(date), " ");
    date_tex = SDL_GetText(arial30_font, light_grey_col, date);

    isDirectory = true;
    isDrive = true;
    devPath = _dev;
}

FileButton::~FileButton(){ 
    SDL_DestroyTexture(name_tex);
    SDL_DestroyTexture(type_tex);
    SDL_DestroyTexture(size_tex);
    SDL_DestroyTexture(date_tex);
}

void FileButton::Render(int pos){
    int y = GetY(pos);
    if (y > -80 && y < 720){
        SDL_DrawImage((touchedFile == pos) ? file_slot_touched_tex : file_slot_tex, 300, y);
        SDL_DrawImage((selected) ? file_checkbox_true_tex : file_checkbox_false_tex, 300, y);
        SDL_DrawImage(icon, 375, y);

        SDL_DrawAlignedImage(name_tex, 475, y + 14, Alignments::LEFT);
        SDL_DrawAlignedImage(type_tex, 475, y + 58, Alignments::LEFT);
        
        SDL_DrawAlignedImage(size_tex, 1180, y + 15, Alignments::RIGHT);
        SDL_DrawAlignedImage(date_tex, 1180, y + 55, Alignments::RIGHT);
    }
}

void FileButton::CheckSelection(int pos){
    //if (layer != 0)return;
    int y = GetY(pos);
    if (touchStatus == TouchStatus::TOUCHED_UP){
        if (vpad.tpNormal.x >= 300 && vpad.tpNormal.x <= 300 + 100 &&
            vpad.tpNormal.y >= y && vpad.tpNormal.y <= y + 100 &&
            vpad.tpNormal.y > 100 && vpad.tpNormal.y <= 720){
            SetSelection(!selected);
        }
        else if (vpad.tpNormal.x >= 400 && vpad.tpNormal.x <= 1280-100 &&
                vpad.tpNormal.y >= y && vpad.tpNormal.y <= y + 100 &&
                vpad.tpNormal.y > 100 && vpad.tpNormal.y <= 720){
            if (touchedFile != pos){
                touchedFile = pos;
            }
            else if (touchedFile == pos){
                if (isDirectory) {
                    if (isDrive){
                        Filesystem::TryToMount(devPath, "/vol/" + name);
                    }
                    Filesystem::ChangeDir(name);
                }
            }
        }
    }

    if (vpad.trigger & VPAD_BUTTON_Y && touchedFile == pos)
        selected = !selected;
}

void FileButton::SetSelection(bool _selected){
    selected = _selected;
    selectedItems += (selected) ? 1 : -1;

    uint32_t f = 0;
    for (uint32_t i = 0; i < files.size(); i++){
        if (files[i]->IsSelected()){
            f++;
        }
    }
    if (f <= 0){
        checkbox_b->SetTexture(op_checkbox_false_tex);

        open_b->SetActive(false);
        copy_b->SetActive(false);
        cut_b->SetActive(false);
        delete_b->SetActive(false);
        rename_b->SetActive(false);
    }
    else if (f >= files.size()){
        checkbox_b->SetTexture(op_checkbox_true_tex);
        
        open_b->SetActive(f == 1);
        copy_b->SetActive(true);
        cut_b->SetActive(true);
        delete_b->SetActive(true);
        rename_b->SetActive(f == 1);
    }
    else {
        checkbox_b->SetTexture(op_checkbox_neutral_tex);
        
        open_b->SetActive(f == 1);
        copy_b->SetActive(true);
        cut_b->SetActive(true);
        delete_b->SetActive(true);
        rename_b->SetActive(f == 1);
    }
    
    checkedItems_tex = SDL_GetTextf(arial50_font, black_col, "%d/%d", selectedItems, files.size());
}

int FileButton::GetY(int pos){
    return 100 + pos * 100 - slider * (nfiles * 100 - 620);
}

std::string FileButton::GetName(){
    return name;
}

bool FileButton::IsDirectory(){
    return isDirectory;
}

bool FileButton::IsSelected(){
    return selected;
}