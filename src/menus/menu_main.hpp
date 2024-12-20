#pragma once
#include "../main.hpp"
#include "../gui/button.hpp"
#include "../gui/dialog/dialog.hpp"
#include "../gui/dialog/dialog_default.hpp"
#include "../gui/menu.hpp"

extern int previousTicks;
extern float timeDelta;

extern SDL_Texture* directoryInfo1;
extern SDL_Texture* directoryInfo2;
extern SDL_Texture* checkedItems_tex;
extern SDL_Texture* permissions_tex;

//Variables
extern uint32_t selectedItems;
extern std::string folderPerms;

//Menu
extern Menu* main_m;

//Buttons
extern Button* back_b;
extern Button* next_b;
extern Button* rewind_b;
extern Button* checkbox_b;

extern Button* open_b;

extern Button* newFile_b;
extern Button* newFolder_b;

extern Button* copy_b;
extern Button* cut_b;
extern Button* paste_b;

extern Button* delete_b;
extern Button* rename_b;

extern Button* settings_b;
extern Button* properties_b;

void loadMenu_Main();