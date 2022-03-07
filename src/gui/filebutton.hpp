#pragma once
#include "../main.hpp"

class FileButton{
    private:
        IOSUHAX_FSA_DirectoryEntry entry;
        bool selected;

        SDL_Texture* icon;

        std::string name;
        SDL_Texture* name_tex;

        std::string type;
        SDL_Texture* type_tex;

        char size[128];
        SDL_Texture* size_tex;

        char date[128];
        SDL_Texture* date_tex;
        
        bool isDirectory;
        bool isDrive;
        std::string devPath; //Device path
    public:
        FileButton(FSDirectoryEntry _entry);
        FileButton(std::string _name, std::string _type, SDL_Texture* _icon, bool _isDirectory);
        FileButton(std::string _name, std::string _dev, std::string _type, SDL_Texture* _icon);
        ~FileButton();

        void Render(int pos);
        void CheckSelection(int pos);
        void SetSelection(bool _selected);
        
        int GetY(int pos);
        std::string GetName();
        bool IsDirectory();
        bool IsSelected();
};