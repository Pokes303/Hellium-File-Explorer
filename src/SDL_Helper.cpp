#include "SDL_Helper.hpp"

std::vector<SDL_Texture*> images;
std::vector<TTF_Font*> fonts;

void SDL_LoadImage(SDL_Texture **texture, std::string path) {
	SDL_Surface *loaded_surface = NULL;
	loaded_surface = IMG_Load(path.c_str());

	if (!loaded_surface)
		WHBLogPrintf("[SDL_Helper.cpp]>Error: SDL_LoadImage() failed with file: %s, and error: %s\n", path.c_str(), SDL_GetError());

	*texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
	images.push_back(*texture);

	SDL_FreeSurface(loaded_surface);
}

void SDL_UnloadImages() {
	for (uint32_t i = 0; i < images.size(); i++){
		SDL_DestroyTexture(images[i]);
		images[i] = nullptr;
	}
	images.clear();
}

void SDL_LoadFont(TTF_Font **font, int size, std::string path) {
	*font = TTF_OpenFont(path.c_str(), size);
	if (!*font)
		WHBLogPrintf("[SDL_Helper.cpp]>Error: SDL_LoadFont() failed with file: %s, and error: %s\n", path.c_str(), SDL_GetError());
	else
		fonts.push_back(*font);
}

void SDL_UnloadFonts() {
	for (uint32_t i = 0; i < fonts.size(); i++){
		TTF_CloseFont(fonts[i]);
		fonts[i] = nullptr;
	}
	fonts.clear();
}

void SDL_TakeScreenshot(std::string path){
	SDL_Surface *screenshot = SDL_CreateRGBSurface(0, 1280, 720, 32, 0, 0, 0, 0);
	if (SDL_RenderReadPixels(renderer, NULL, screenshot->format->format, screenshot->pixels, screenshot->pitch) != 0){
		WHBLogPrintf("[SDL_Helper.cpp]>Error: SDL_RenderReadPixels failed with file: %s, and error: %s\n", path.c_str(), SDL_GetError());
		SDL_FreeSurface(screenshot);
	}
	if (IMG_SavePNG(screenshot, path.c_str()) != 0){
		WHBLogPrintf("[SDL_Helper.cpp]>Error: IMG_SavePNG failed with file: %s, and error: %s\n", path.c_str(), SDL_GetError());
	}
	SDL_FreeSurface(screenshot);
}

void SDL_ClearScreen(SDL_Color colour) {
	SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
	SDL_RenderClear(renderer);
}

void SDL_DrawRect(int x, int y, int w, int h, SDL_Color colour) {
	SDL_Rect rect;
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
	SDL_RenderFillRect(renderer, &rect);
	SDL_DrawImage(void_tex, 0, 0); //For some reason, this fixes a bug
	SDL_RenderFillRect(renderer, &rect);
}


void SDL_DrawImage(SDL_Texture *texture, int x, int y) {
	SDL_Rect position;
	position.x = x; position.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
	SDL_RenderCopy(renderer, texture, NULL, &position);
}

void SDL_DrawImageCut(SDL_Texture *texture, int x, int y, int cutW, int cutH) {
	SDL_Rect position;
	position.x = x; position.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
	SDL_Rect cut;
	cut.x = cut.y = 0;
	cut.w = cutW; cut.h = cutH;
	SDL_RenderCopy(renderer, texture, &cut, &position);
}

void SDL_DrawImageScale(SDL_Texture *texture, int x, int y, int w, int h) {
	SDL_Rect position;
	position.x = x; position.y = y; position.w = w; position.h = h;
	SDL_RenderCopy(renderer, texture, NULL, &position);
}
void SDL_DrawImageScale(SDL_Texture *texture, SDL_Rect position) {
	SDL_RenderCopy(renderer, texture, NULL, &position);
}
void SDL_DrawImageRotate(SDL_Texture *texture, int x, int y, double angle) {
	SDL_Rect position;
	position.x = x; position.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
	SDL_RenderCopyEx(renderer, texture, NULL, &position, angle, NULL, SDL_FLIP_NONE);
}

void SDL_DrawImageRotateAndScale(SDL_Texture *texture, int x, int y, int w, int h, double angle) {
	SDL_Rect position;
	position.x = x; position.y = y; position.w = w; position.h = h;
	SDL_RenderCopyEx(renderer, texture, NULL, &position, angle, NULL, SDL_FLIP_NONE);
}

void SDL_DrawImageCutAndScale(SDL_Texture *texture, int x, int y, int w, int h, int cutW, int cutH) {
	SDL_Rect position;
	position.x = x; position.y = y;
	position.w = w; position.h = h;
	SDL_Rect cut;
	cut.x = cut.y = 0;
	cut.w = cutW; cut.h = cutH;
	SDL_RenderCopy(renderer, texture, &cut, &position);
}

void SDL_DrawImageRect(SDL_Texture *texture, SDL_Rect dst_rct) {
	SDL_RenderCopy(renderer, texture, NULL, &dst_rct);
}

void SDL_DrawImageRect(SDL_Texture *texture, SDL_Rect src_rct, SDL_Rect dst_rct) {
	SDL_RenderCopy(renderer, texture, &dst_rct, &src_rct);
}

void SDL_DrawAlignedImage(SDL_Texture *texture, int x, int y, Alignments align){
	int textW = 0;
	if (align != LEFT) {
		SDL_QueryTexture(texture, NULL, NULL, &textW, NULL);
		if (align == MIDDLE)
			textW /= 2;
	}
	
	SDL_DrawImage(texture, x - textW, y);
}



void SDL_DrawAlphaText(TTF_Font *font, int x, int y, Alignments align, SDL_Color colour, uint8_t alpha, const char *text) {
	SDL_Surface *surface = TTF_RenderText_Blended(font, text, colour);
	SDL_SetSurfaceAlphaMod(surface, colour.a);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	int textW = 0;
	if (align != LEFT) {
		TTF_SizeText(font, text, &textW, NULL);
		if (align == MIDDLE)
			textW /= 2;
	}

	SDL_SetTextureAlphaMod(texture, alpha);

	SDL_Rect position;
	position.x = x - textW; position.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
	SDL_RenderCopy(renderer, texture, NULL, &position);
	SDL_DestroyTexture(texture);
}

void SDL_DrawText(TTF_Font *font, int x, int y, Alignments align, SDL_Color colour, const char *text) {
	SDL_DrawAlphaText(font, x, y, align, colour, 255, text);
}

void SDL_DrawTextf(TTF_Font *font, int x, int y, Alignments align, SDL_Color colour, const char* text, ...) {
	char buffer[512];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 512, text, args);
	SDL_DrawText(font, x, y, align, colour, buffer);
	va_end(args);
}

void SDL_DrawOutlineTextf(TTF_Font *font, TTF_Font *outline_font, int x, int y, Alignments align, SDL_Color colour, SDL_Color outline_colour, const char* text, ...) {
	int plusX = 0;//, plusY = 0; //This needs a change
	char buffer[512];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 512, text, args);
	SDL_DrawText(outline_font, x + plusX, y, align, outline_colour, buffer);
	SDL_DrawText(font, x, y /*lfl_outline_font outline size*/ + 2, align, colour, buffer);
	va_end(args);
}

void SDL_DrawOutlineAlphaTextf(TTF_Font *font, TTF_Font *outline_font, int x, int y, Alignments align, SDL_Color colour, SDL_Color outline_colour, uint8_t alpha, const char* text, ...) {
	/*int plusX = 0, plusY = 0; //This needs a change
	if (outline_font == debug_arial_outline_font)
		plusX = -2;*/
	int outline = TTF_GetFontOutline(outline_font);
	char buffer[512];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 512, text, args);
	SDL_DrawAlphaText(outline_font, x - outline, y - outline, align, outline_colour, alpha, buffer);
	SDL_DrawAlphaText(font, x, y /*lfl_outline_font outline size*/ + 2, align, colour, alpha, buffer);
	va_end(args);
}


SDL_Texture* SDL_GetAlphaText(TTF_Font *font, SDL_Color colour, uint8_t alpha, const char *text) {
	SDL_Surface *surface = TTF_RenderText_Blended(font, text, colour);
	SDL_SetSurfaceAlphaMod(surface, colour.a);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	SDL_SetTextureAlphaMod(texture, alpha);
	return texture;
}

SDL_Texture* SDL_GetText(TTF_Font *font, SDL_Color colour, const char *text){
	return SDL_GetAlphaText(font, colour, 255, text);
}

SDL_Texture* SDL_GetTextf(TTF_Font *font, SDL_Color colour, const char* text, ...){
	char buffer[512];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, 512, text, args);
	SDL_Texture* texture = SDL_GetText(font, colour, buffer);
	va_end(args);
	return texture;
}