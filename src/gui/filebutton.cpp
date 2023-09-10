#include "filebutton.hpp"
#include "../SDL_Helper.hpp"
#include "../filesystem_helper.hpp"
#include "../filesystem.hpp"
#include "../menus/menu_main.hpp"
#include "../udplog.hpp"
#include "../input.hpp"
#include "../dialog_helper.hpp"

FileButton::FileButton(FSDirectoryEntry entry){
    selected = false;

    icon = (entry.info.flags & FS_STAT_DIRECTORY) ? folder_tex : file_tex;
    name = entry.name;
    name_tex = SDLH::GetText(arialBold35_font, black_col, name.c_str());

    if (entry.info.flags & FS_STAT_DIRECTORY){
        desc = "FOLDER";
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
            desc = "";
            for (uint32_t i = 0; i < extension.size(); i++)
                desc += toupper(extension[i]);
            desc += " file";
        }
        else{
            desc = "FILE";
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
    desc_tex = SDLH::GetText(arial28_font, dark_grey_col, desc.c_str());
    size_tex = SDLH::GetText(arial30_font, light_grey_col, size);

    OSCalendarTime ct;
    FSTimeToCalendarTime(entry.info.created, &ct);
    snprintf(date, sizeof(date), "%02d/%02d/%04d %02d:%02d", ct.tm_mday, ct.tm_mon + 1, ct.tm_year, ct.tm_hour, ct.tm_min);
    date_tex = SDLH::GetText(arial30_font, light_grey_col, date);

    type = entry.info.flags & FS_STAT_DIRECTORY ? FileButtonType::FOLDER : FileButtonType::FILE;
}

FileButton::FileButton(std::string _name, std::string _desc, FileButtonType _type) {
    selected = false;

    name = _name;
    name_tex = SDLH::GetText(arialBold35_font, black_col, name.c_str());

    desc = _desc;
    desc_tex = SDLH::GetText(arial28_font, dark_grey_col, desc.c_str());

    snprintf(size, sizeof(size), " ");
    size_tex = SDLH::GetText(arial30_font, light_grey_col, size);

    snprintf(date, sizeof(date), " ");
    date_tex = SDLH::GetText(arial30_font, light_grey_col, date);

    type = _type;
    switch(type){
    case FILE:
        icon = file_tex;
        break;
    case FOLDER:
        icon = folder_tex;
        break;
    case DRIVE_SD:
        icon = sd_tex;
        break;
    case DRIVE_USB:
        icon = usb_tex;
        break;
    case DRIVE_NAND:
        icon = nand_tex;
        break;
    case DRIVE_DISK:
        icon = disk_tex;
        break;
    default:
        icon = unknown_tex;
        break;
    }
}

/*FileButton::FileButton(std::string _name, std::string _dev, std::string _type, SDL_Texture* _icon){
    selected = false;
    icon = _icon;

    name = _name;
    name_tex = SDLH::GetText(arialBold35_font, black_col, name.c_str());

    type = _type;
    desc_tex = SDLH::GetText(arial28_font, dark_grey_col, type.c_str());

    snprintf(size, sizeof(size), " ");
    size_tex = SDLH::GetText(arial30_font, light_grey_col, size);

    snprintf(date, sizeof(date), " ");
    date_tex = SDLH::GetText(arial30_font, light_grey_col, date);

    isDirectory = true;
    isDrive = true;
    devPath = _dev;
}*/

FileButton::~FileButton(){ 
    SDL_DestroyTexture(name_tex);
    SDL_DestroyTexture(desc_tex);
    SDL_DestroyTexture(size_tex);
    SDL_DestroyTexture(date_tex);
}

void FileButton::Render(int pos){
    int y = GetY(pos);
    if (y > -80 && y < 720){
        SDLH::DrawImage((touchedFile == pos) ? file_slot_touched_tex : file_slot_tex, 300, y);
        SDLH::DrawImage((selected) ? file_checkbox_true_tex : file_checkbox_false_tex, 300, y);
        SDLH::DrawImage(icon, 375, y);

        SDLH::DrawImageAligned(name_tex, 475, y + 14, AlignmentsX::LEFT);
        SDLH::DrawImageAligned(desc_tex, 475, y + 58, AlignmentsX::LEFT);
        
        SDLH::DrawImageAligned(size_tex, 1180, y + 15, AlignmentsX::RIGHT);
        SDLH::DrawImageAligned(date_tex, 1180, y + 55, AlignmentsX::RIGHT);
    }
}

void FileButton::CheckSelection(int pos){
    int y = GetY(pos);
    if (touch.status == TouchStatus::TOUCHED_UP &&
        !DialogHelper::DialogExists() &&
        !SWKBD::IsShown()){
        if (touch.x >= 300 && touch.x <= 300 + 100 &&
            touch.y >= y && touch.y <= y + 100 &&
            touch.y > 100 && touch.y <= 720){
            SetSelection(!selected);
        }
        else if (touch.x >= 400 && touch.x <= 1280-100 &&
                touch.y >= y && touch.y <= y + 100 &&
                touch.y > 100 && touch.y <= 720){
            if (touchedFile != pos){
                touchedFile = pos;
            }
            else if (touchedFile == pos){
                if (type == FileButtonType::DRIVE_SD ||
                    type == FileButtonType::DRIVE_USB ||
                    type == FileButtonType::DRIVE_NAND ||
                    type == FileButtonType::DRIVE_DISK)
                        Filesystem::MountDevice(name);
                FilesystemHelper::ChangePathDir(name);
            }
        }
    }

    if (vpad.trigger & VPAD_BUTTON_Y && touchedFile == pos)
        selected = !selected;
}

void FileButton::SetSelection(bool _selected){
    selected = _selected;
    selectedItems += (selected) ? 1 : -1;
    Mix_PlayChannel(0, click_sound_ch, 0);

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
    
    checkedItems_tex = SDLH::GetTextf(arial50_font, black_col, "%d/%d", selectedItems, files.size());
}

int FileButton::GetY(int pos){
    return 100 + (pos * 100) - (slider * (files.size() * 100 - 620));
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