#pragma once
#include "main.hpp"

//SDL_Helper based on the HBAppStore implementation by vgmoose & more
enum Alignments { LEFT, MIDDLE,	RIGHT };

void SDL_LoadImage(SDL_Texture **texture, std::string path);
void SDL_UnloadImages();
void SDL_GetImageLoadFails();
void SDL_LoadFont(TTF_Font **font, int size, std::string path);
void SDL_UnloadFonts();

void SDL_TakeScreenshot(std::string path);

void SDL_ClearScreen(SDL_Color colour);
void SDL_DrawRect(int x, int y, int w, int h, SDL_Color colour);

void SDL_DrawImage(SDL_Texture *texture, int x, int y);
void SDL_DrawImageCut(SDL_Texture *texture, int x, int y, int cutW, int cutH);
void SDL_DrawImageScale(SDL_Texture *texture, int x, int y, int w, int h);
void SDL_DrawImageScale(SDL_Texture *texture, SDL_Rect position);
void SDL_DrawImageRotate(SDL_Texture *texture, int x, int y, double angle);
void SDL_DrawImageRotateAndScale(SDL_Texture *texture, int x, int y, int w, int h, double angle);
void SDL_DrawImageCutAndScale(SDL_Texture *texture, int x, int y, int w, int h, int cutW, int cutH);
void SDL_DrawImageRect(SDL_Texture *texture, SDL_Rect src_rct);
void SDL_DrawImageRect(SDL_Texture *texture, SDL_Rect src_rct, SDL_Rect dst_rct);
void SDL_DrawAlignedImage(SDL_Texture *texture, int x, int y, Alignments align);

void SDL_DrawAlphaText(TTF_Font *font, int x, int y, Alignments align, SDL_Color colour, uint8_t alpha, const char *text);
void SDL_DrawText(TTF_Font *font, int x, int y, Alignments align, SDL_Color colour, const char *text);
void SDL_DrawTextf(TTF_Font *font, int x, int y, Alignments align, SDL_Color colour, const char* text, ...);
void SDL_DrawOutlineTextf(TTF_Font *font, TTF_Font *outline_font, int x, int y, Alignments align, SDL_Color colour, SDL_Color outline_colour, const char* text, ...);
void SDL_DrawOutlineAlphaTextf(TTF_Font *font, TTF_Font *outline_font, int x, int y, Alignments align, SDL_Color colour, SDL_Color outline_colour, uint8_t alpha, const char* text, ...);

SDL_Texture* SDL_GetAlphaText(TTF_Font *font, SDL_Color colour, uint8_t alpha, const char *text);
SDL_Texture* SDL_GetText(TTF_Font *font, SDL_Color colour, const char *text);
SDL_Texture* SDL_GetTextf(TTF_Font *font, SDL_Color colour, const char* text, ...);