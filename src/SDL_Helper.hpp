#pragma once
#include "main.hpp"

//Helper based on the HBAppStore implementation by vgmoose & more
enum AlignmentsX { LEFT, MIDDLE_X, RIGHT };
enum AlignmentsY { UP, MIDDLE_Y, DOWN };
namespace SDLH{
    void LoadImage(SDL_Texture **texture, std::string path);
    void UnloadImages();
    void GetImageLoadFails();
    void LoadFont(TTF_Font **font, std::string path, int size);
    void UnloadFonts();
    void SetFontOutline(TTF_Font **font, int size);

    void TakeScreenshot(std::string path);

    void ClearScreen(SDL_Color colour);
    void DrawRect(int x, int y, int w, int h, SDL_Color colour);

    void DrawImage(SDL_Texture *texture, int x, int y);
    void DrawImageCut(SDL_Texture *texture, int x, int y, int cutW, int cutH);
    void DrawImageScale(SDL_Texture *texture, int x, int y, int w, int h);
    void DrawImageScale(SDL_Texture *texture, SDL_Rect position);
    void DrawImageRotate(SDL_Texture *texture, int x, int y, double angle);
    void DrawImageRotateAndScale(SDL_Texture *texture, int x, int y, int w, int h, double angle);
    void DrawImageCutAndScale(SDL_Texture *texture, int x, int y, int w, int h, int cutW, int cutH);
    void DrawImageRect(SDL_Texture *texture, SDL_Rect src_rct);
    void DrawImageRect(SDL_Texture *texture, SDL_Rect src_rct, SDL_Rect dst_rct);
    void DrawImageAligned(SDL_Texture *texture, int x, int y, AlignmentsX alignx);
    void DrawImageAligned(SDL_Texture *texture, int x, int y, AlignmentsX alignx, AlignmentsY aligny);

    void DrawAlphaText(TTF_Font *font, int x, int y, AlignmentsX align, SDL_Color colour, uint8_t alpha, const char *text);
    void DrawText(TTF_Font *font, int x, int y, AlignmentsX align, SDL_Color colour, const char *text);
    void DrawTextf(TTF_Font *font, int x, int y, AlignmentsX align, SDL_Color colour, const char* text, ...);
    void DrawOutlineTextf(TTF_Font *font, TTF_Font *outline_font, int x, int y, AlignmentsX align, SDL_Color colour, SDL_Color outline_colour, const char* text, ...);
    void DrawOutlineAlphaTextf(TTF_Font *font, TTF_Font *outline_font, int x, int y, AlignmentsX align, SDL_Color colour, SDL_Color outline_colour, uint8_t alpha, const char* text, ...);

    SDL_Texture* GetAlphaText(TTF_Font *font, SDL_Color colour, uint8_t alpha, const char *text);
    SDL_Texture* GetText(TTF_Font *font, SDL_Color colour, const char *text);
    SDL_Texture* GetTextf(TTF_Font *font, SDL_Color colour, const char* text, ...);
    void GetOutlineTextf(SDL_Texture** tex1, SDL_Texture** tex2, TTF_Font *font, TTF_Font *outline_font, SDL_Color colour, SDL_Color outline_colour, const char* text, ...);
    void DrawOutlinedText(SDL_Texture** array, int x, int y);
}