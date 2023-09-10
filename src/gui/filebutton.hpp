#pragma once
#include "../main.hpp"

enum FileButtonType{
    FILE,
    FOLDER,
    DRIVE_SD,
    DRIVE_USB,
    DRIVE_NAND,
    DRIVE_DISK
}

class FileButton{
    private:
        bool selected;

        FileButtonType type;
        SDL_Texture* icon;

        std::string name;
        SDL_Texture* name_tex;

        std::string desc;
        SDL_Texture* desc_tex;

        char size[128];
        SDL_Texture* size_tex;

        char date[128];
        SDL_Texture* date_tex;
    public:
        FileButton(FSDirectoryEntry _entry);
        FileButton(std::string _name, std::string _type, FileButtonType _type);
        //FileButton(std::string _name, std::string _dev, std::string _type, SDL_Texture* _icon);
        ~FileButton();

        void Render(int pos);
        void CheckSelection(int pos);
        void SetSelection(bool _selected);
        
        int GetY(int pos);
        std::string GetName();
        bool IsDirectory();
        bool IsSelected();
};