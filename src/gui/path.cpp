#include "path.hpp"
#include "../input.hpp"
#include "../SDL_Helper.hpp"
#include "../menus/menu_main.hpp"
#include "../dialog_helper.hpp"
#include "../udplog.hpp"
#include "../filesystem_helper.hpp"

std::string path = "";
SDL_Texture* path_tex;
SDL_Texture* path_type_tex;
float pathTimer = 0.0;
float pathX = 389.0;
float pathAnimSpeed = 100.0;
uint8_t pathAlpha = 0;
bool pathAnimation;
int pathTextW = 0;
int pathAnimationPhase = 0;

PathType pathType;

std::vector<std::string> previousPaths;
uint32_t previousPathPos = 0;

void clearPath(){
    if (path_tex){
        SDL_DestroyTexture(path_tex);
        path_tex = nullptr;
    }
}

void Path::SetPath(std::string newPath){
    //Add previous path
    if (path != ""){
        if (previousPaths.size() <= 0){
            back_b->SetActive(true);
        }
        else if (previousPaths.size() >= 50){
            previousPaths.erase(previousPaths.begin());
        }
        previousPaths.push_back(path);
        if (previousPathPos == previousPaths.size())
            previousPathPos++;
        else{
            previousPathPos = previousPaths.size();
            next_b->SetActive(false);
        }
    }

    clearPath();
    //Set the new path
    path = newPath;

    if (path.size() > 0){
        if (path[0] != '/')
            path = '/' + path;
        if (path[path.size() - 1] != '/')
            path += '/';
    }
    else{
        path = "/";
    }

    //Create textures for the new path
    std::string texturedPath = "";
    for (uint32_t i = 0; i < path.size(); i++){
        if (path[i] != '\n')
            texturedPath += path[i];
    }
    path_tex =  SDLH::GetText(arial50_font, black_col, texturedPath.c_str());
    TTF_SizeText(arial50_font, path.c_str(), &pathTextW, NULL);
    pathAnimation = pathTextW > 880;
    pathAnimationPhase = 0;
    pathTimer = 0.0;
    pathX = 389.0;
    pathAlpha = 0.0;

    //Read new directory
    int res = FSChangeDir(cli, block, path.c_str(), FS_ERROR_FLAG_ALL);
    if (res < 0)
        LOG_E("FSChangeDir returned (%d)", res);
    FilesystemHelper::ReadPathDir();
}

std::string Path::GetPath(){
    return path;
}

void clearPathType(){
    if (path_type_tex)
        SDL_DestroyTexture(path_type_tex);
}

void Path::SetPathType(PathType newType){
    clearPathType();
    switch(pathType){
        case PathType::REAL:
            path_type_tex = nullptr;
            break;
        case PathType::VIRTUAL:
            path_type_tex = SDLH::GetText(arial25_font, dark_red_col, "Virtual directory");
            break;
        case PathType::IOSUHAX:
            path_type_tex = SDLH::GetText(arial25_font, dark_red_col, "IOSUHAX directory");
            break;
        default:
            LOG_E("[filesystem.cpp]>Error: Unknown pathType value (%d)", pathType);
            path_type_tex = SDLH::GetText(arial25_font, dark_red_col, "Unknown directory type");
            break;

    }
}

PathType Path::GetPathType(){
    return pathType;
}

void Path::PreviousPath(){
    previousPathPos--;
    SetPath(previousPaths[previousPathPos]);

    next_b->SetActive(true);
    if (previousPathPos <= 0) {
        back_b->SetActive(false);
    }
}

void Path::NextPath(){
    previousPathPos++;
    SetPath(previousPaths[previousPathPos]);

    back_b->SetActive(true);
    if (previousPathPos >= previousPaths.size()){
        next_b->SetActive(false);
    }
}

void changePathCallback(std::string result){
    FilesystemHelper::SetPathDir(result);
}

void Path::Render(){
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
        SDLH::DrawImage(path_tex, (int)pathX, (pathType == PathType::REAL) ? 25 : 10);
        SDLH::DrawImage(path_shadow, 370, 0);
    }
    else
        SDLH::DrawImage(path_tex, 389, (pathType == PathType::REAL) ? 25 : 10);

    if (path_type_tex)
        SDLH::DrawImage(path_type_tex, 379, 60);

    if (touch.status == TouchStatus::TOUCHED_DOWN &&
        !DialogHelper::DialogExists() &&
        !SWKBD::IsShown() &&
        touch.x > 370 && touch.x < 1280 &&
        touch.y > 0 && touch.y < 100){
            SWKBD::Appear(Path::GetPath(), "Enter the new path here", (void*)changePathCallback);
    }
}

void Path::Shutdown(){
    clearPath();
    clearPathType();
}