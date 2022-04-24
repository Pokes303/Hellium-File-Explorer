#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <cctype>
#include <math.h>
#include <algorithm>
#include <thread>
#include <locale>
#include <codecvt>
#include <sstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iosuhax.h>
#include <romfs-wiiu.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/ios.h>
#include <coreinit/memory.h>
#include <coreinit/mcp.h>
#include <coreinit/exception.h>
#include <coreinit/filesystem.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/dynload.h>

#include <nn/swkbd/swkbd_cpp.h>
#include <proc_ui/procui.h>
#include <vpad/input.h>

#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <whb/sdcard.h>

#define ROMFS_PATH "romfs:/"

extern SDL_Renderer* renderer;

extern float slider;
extern int sliderY;
extern int touchedFile;

//FILESYSTEM
extern FSClient* cli;
extern FSCmdBlock* block;

extern int mcp_hook_fd;
extern int fsaFd;

//Mount checks
extern bool sdCardMounted;

////SDL2 RESOURCES
//COLOURS
extern SDL_Color black_col;
extern SDL_Color dark_grey_col;
extern SDL_Color light_grey_col;
extern SDL_Color dark_red_col;
extern SDL_Color white_col;

//IMAGES
extern SDL_Texture* void_tex;

extern SDL_Texture* bg_tex;

extern SDL_Texture* menu_left_tex;
extern SDL_Texture* menu_up_tex;

extern SDL_Texture* path_bottom;
extern SDL_Texture* path_shadow;
extern SDL_Texture* checked_items;

extern SDL_Texture* button0_tex;
extern SDL_Texture* button0_deactivated_tex;
extern SDL_Texture* button1_tex;
extern SDL_Texture* button1_deactivated_tex;
extern SDL_Texture* button2_tex;
extern SDL_Texture* button2_deactivated_tex;
extern SDL_Texture* button3_tex;
extern SDL_Texture* button3_deactivated_tex;
extern SDL_Texture* button4_tex;
extern SDL_Texture* button4_deactivated_tex;
extern SDL_Texture* dialog_button1_tex;
extern SDL_Texture* dialog_button1_deactivated_tex;
extern SDL_Texture* dialog_button2_tex;
extern SDL_Texture* dialog_button2_deactivated_tex;

extern SDL_Texture* file_slot_tex;
extern SDL_Texture* file_slot_touched_tex;

extern SDL_Texture* button_slider_tex;
extern SDL_Texture* button_slider_deactivated_tex;
extern SDL_Texture* slider_path_tex;
extern SDL_Texture* slider_path_deactivated_tex;

extern SDL_Texture* dialog_tex;
extern SDL_Texture* dialog_progress_bar_tex;
extern SDL_Texture* dialog_progress_bar_status_tex;

extern SDL_Texture* dialog_textbox_tex;
extern SDL_Texture* textbox_tex;

extern SDL_Texture* bubble1_tex;
extern SDL_Texture* bubble2_tex;
extern SDL_Texture* bubble3_tex;
extern SDL_Texture* bubble4_tex;
extern SDL_Texture* bubble5_tex;
#define MAX_BUBBLE_SIZE 5

//	icons
extern SDL_Texture* file_checkbox_false_tex;
extern SDL_Texture* file_checkbox_true_tex;

extern SDL_Texture* file_tex;
extern SDL_Texture* folder_tex;
extern SDL_Texture* sd_tex;
extern SDL_Texture* nand_tex;
extern SDL_Texture* disk_tex;
extern SDL_Texture* usb_tex;
extern SDL_Texture* unknown_tex;

extern SDL_Texture* arrow_left_tex;
extern SDL_Texture* arrow_right_tex;
extern SDL_Texture* arrow_up_tex;
extern SDL_Texture* op_checkbox_false_tex;
extern SDL_Texture* op_checkbox_true_tex;
extern SDL_Texture* op_checkbox_neutral_tex;

extern SDL_Texture* open_tex;
extern SDL_Texture* copy_tex;
extern SDL_Texture* cut_tex;
extern SDL_Texture* paste_tex;
extern SDL_Texture* delete_tex;
extern SDL_Texture* rename_tex;
extern SDL_Texture* file_new_tex;
extern SDL_Texture* folder_new_tex;
extern SDL_Texture* properties_tex;

extern SDL_Texture* settings_tex;

//FONTS
extern TTF_Font* arial25_font;
extern TTF_Font* arial25_outline_font;
extern TTF_Font* arial28_font;
extern TTF_Font* arial30_font;
extern TTF_Font* arialBold35_font;
extern TTF_Font* arial40_font;
extern TTF_Font* arialItalic40_font;
extern TTF_Font* arialBold48_font;
extern TTF_Font* arial50_font;
extern TTF_Font* arialBold80_font;

//SOUNDS
extern Mix_Music* bg_mus;
extern Mix_Chunk* click_sound_ch;

////FUNCTIONS
int main(int argc, char *argv[]);