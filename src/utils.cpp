#include "utils.hpp"
#include "menus/menu_main.hpp"
#include "SDL_Helper.hpp"

#include <cstdarg>

bool Utils::StartsWith(std::string str, std::string start){
    for (uint32_t i = 0; i < start.size() && i < str.size(); i++){
        if (str[i] == start[i]){
            continue;
        }
        return false;
    }
    return true;
}

bool Utils::AlphabeticalSort(FileButton* a, FileButton* b){
    if (a->IsDirectory() && !b->IsDirectory())
        return true;
    else if (!a->IsDirectory() && b->IsDirectory())
        return false;
    return a->GetName() < b->GetName();
}

std::string Utils::GetFilename(std::string file){
    std::string name = "";
    for (int i = file.length() - 1; i > 0; i--){
        if (file[i] == '/'){
            return name;
        }

        name = file[i] + name;
    }
    return name;
}

int frames = 0;
uint64_t lastTick = 0;
SDL_Texture* fps_tex1;
SDL_Texture* fps_tex2;
void Utils::DrawFPS(){
    if (OSGetTick() - lastTick >= OSSecondsToTicks(1)){
        if (fps_tex1)
            SDL_DestroyTexture(fps_tex1);
        if (fps_tex2)
            SDL_DestroyTexture(fps_tex2);
        SDLH::GetOutlineTextf(&fps_tex1, &fps_tex2, arial25_font, arial25_outline_font, white_col, black_col, "%dfps", frames);

        lastTick = OSGetTick();
        frames = 0;
    }
    frames++;
    SDLH::DrawImageAligned(fps_tex1, 1280 - 5, 720 - 40, AlignmentsX::RIGHT);
    SDLH::DrawImageAligned(fps_tex2, 1280 - 5 - 2, 720 - 40, AlignmentsX::RIGHT);
}

std::string Utils::IntToHex(int n){
    const int size = sizeof(n) * 2 + 2 + 1;
    char buffer[size];
    snprintf(buffer, size, "0x%X", n);
    return std::string(buffer, size);
}