#include "SDL_Helper.hpp"
#include "udplog.hpp"
#include "menus/menu_main.hpp"
#include "filesystem.hpp"
#include "bubbles.hpp"
#include "input.hpp"
#include "dialog_helper.hpp"
//#include "exception_handler.h"

//GENERAL
SDL_Renderer* renderer;

float slider = 0;
int sliderY = 100;
int touchedFile = -1;

//FILESYSTEM
FSClient* cli;
FSCmdBlock* block;

int mcp_hook_fd = -1;
int fsaFd;

//Mount checks
bool sdCardMounted = false;

//SDL2 RESOURCES
//COLOURS
SDL_Color black_col = { 0, 0, 0, 255 };
SDL_Color dark_grey_col = { 64, 64, 64, 255 };
SDL_Color light_grey_col = { 128, 128, 128, 255 };
SDL_Color red_col = { 128, 0, 0, 255 };
SDL_Color dark_red_col = { 192, 0, 0, 255 };
SDL_Color white_col = { 255, 255, 255, 255 };

//IMAGES
SDL_Texture* void_tex;

SDL_Texture* bg_tex;

SDL_Texture* menu_left_tex;
SDL_Texture* menu_up_tex;

SDL_Texture* path_bottom;
SDL_Texture* path_shadow;
SDL_Texture* checked_items;

SDL_Texture* button0_tex;
SDL_Texture* button0_deactivated_tex;
SDL_Texture* button1_tex;
SDL_Texture* button1_deactivated_tex;
SDL_Texture* button2_tex;
SDL_Texture* button2_deactivated_tex;
SDL_Texture* button3_tex;
SDL_Texture* button3_deactivated_tex;
SDL_Texture* button4_tex;
SDL_Texture* button4_deactivated_tex;
SDL_Texture* dialog_button1_tex;
SDL_Texture* dialog_button1_deactivated_tex;
SDL_Texture* dialog_button2_tex;
SDL_Texture* dialog_button2_deactivated_tex;

SDL_Texture* file_slot_tex;
SDL_Texture* file_slot_touched_tex;

SDL_Texture* button_slider_tex;
SDL_Texture* button_slider_deactivated_tex;
SDL_Texture* slider_path_tex;
SDL_Texture* slider_path_deactivated_tex;

SDL_Texture* dialog_tex;
SDL_Texture* dialog_progress_bar_tex;
SDL_Texture* dialog_progress_bar_status_tex;

SDL_Texture* dialog_textbox_tex;
SDL_Texture* textbox_tex;

SDL_Texture* bubble1_tex;
SDL_Texture* bubble2_tex;
SDL_Texture* bubble3_tex;
SDL_Texture* bubble4_tex;
SDL_Texture* bubble5_tex;

//	icons
SDL_Texture* file_checkbox_false_tex;
SDL_Texture* file_checkbox_true_tex;

SDL_Texture* file_tex;
SDL_Texture* folder_tex;
SDL_Texture* sd_tex;
SDL_Texture* nand_tex;
SDL_Texture* disk_tex;
SDL_Texture* usb_tex;
SDL_Texture* unknown_tex;

SDL_Texture* arrow_left_tex;
SDL_Texture* arrow_right_tex;
SDL_Texture* arrow_up_tex;
SDL_Texture* op_checkbox_false_tex;
SDL_Texture* op_checkbox_true_tex;
SDL_Texture* op_checkbox_neutral_tex;

SDL_Texture* open_tex;
SDL_Texture* copy_tex;
SDL_Texture* cut_tex;
SDL_Texture* paste_tex;
SDL_Texture* delete_tex;
SDL_Texture* rename_tex;
SDL_Texture* file_new_tex;
SDL_Texture* folder_new_tex;
SDL_Texture* properties_tex;

SDL_Texture* settings_tex;

//FONTS
TTF_Font* arial25_font;
TTF_Font* arial25_outline_font;
TTF_Font* arial28_font;
TTF_Font* arial30_font;
TTF_Font* arialBold35_font;
TTF_Font* arial40_font;
TTF_Font* arialItalic40_font;
TTF_Font* arialBold48_font;
TTF_Font* arial50_font;
TTF_Font* arialBold80_font;

//SOUNDS
Mix_Music* bg_mus;
Mix_Chunk* click_sound_ch;

int main(int argc, char *argv[]) {
	WHBLogUdpInit();
	udplog("--------------------------------------------");
	LOG("Initializing libraries...");
	WHBProcInit();

	LOG("Initializing SDL2...");
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	//Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

	SDL_Window* window = SDL_CreateWindow("file explorer", 0, 0, 1280, 720, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);


	LOG("Loading images...");
	SDLH::LoadImage(&bg_tex, "bg.png");

	SDLH::LoadImage(&menu_left_tex, "menu_left.png");
	SDLH::LoadImage(&menu_up_tex, "menu_up.png");
	
	SDLH::LoadImage(&path_bottom, "path_bottom.png");
	SDLH::LoadImage(&path_shadow, "path_shadow.png");
	SDLH::LoadImage(&checked_items, "icons/checked_items.png");

	SDLH::LoadImage(&button0_tex, "button0.png");
	SDLH::LoadImage(&button0_deactivated_tex, "button0_deactivated.png");
	SDLH::LoadImage(&button1_tex, "button1.png");
	SDLH::LoadImage(&button1_deactivated_tex, "button1_deactivated.png");
	SDLH::LoadImage(&button2_tex, "button2.png");
	SDLH::LoadImage(&button2_deactivated_tex, "button2_deactivated.png");
	SDLH::LoadImage(&button3_tex, "button3.png");
	SDLH::LoadImage(&button3_deactivated_tex, "button3_deactivated.png");
	SDLH::LoadImage(&button4_tex, "button4.png");
	SDLH::LoadImage(&button4_deactivated_tex, "button4_deactivated.png");
	SDLH::LoadImage(&dialog_button1_tex, "dialog_button1.png");
	SDLH::LoadImage(&dialog_button1_deactivated_tex, "dialog_button1_deactivated.png");
	SDLH::LoadImage(&dialog_button2_tex, "dialog_button2.png");
	SDLH::LoadImage(&dialog_button2_deactivated_tex, "dialog_button2_deactivated.png");
	SDLH::LoadImage(&file_slot_tex, "file_slot.png");
	SDLH::LoadImage(&file_slot_touched_tex, "file_slot_touched.png");
	
	SDLH::LoadImage(&button_slider_tex, "button_slider.png");
	SDLH::LoadImage(&button_slider_deactivated_tex, "button_slider_deactivated.png");
	SDLH::LoadImage(&slider_path_tex, "slider_path.png");
	SDLH::LoadImage(&slider_path_deactivated_tex, "slider_path_deactivated.png");
	
	SDLH::LoadImage(&dialog_tex, "dialog.png");
	SDLH::LoadImage(&dialog_progress_bar_tex, "dialog_progress_bar.png");
	SDLH::LoadImage(&dialog_progress_bar_status_tex, "dialog_progress_bar_status.png");

	SDLH::LoadImage(&dialog_textbox_tex, "dialog_textbox.png");
	SDLH::LoadImage(&textbox_tex, "textbox.png");
	
	SDLH::LoadImage(&bubble1_tex, "bubble1.png");
	SDL_SetTextureAlphaMod(bubble1_tex, 128);
	SDLH::LoadImage(&bubble2_tex, "bubble2.png");
	SDL_SetTextureAlphaMod(bubble2_tex, 144);
	SDLH::LoadImage(&bubble3_tex, "bubble3.png");
	SDL_SetTextureAlphaMod(bubble3_tex, 160);
	SDLH::LoadImage(&bubble4_tex, "bubble4.png");
	SDL_SetTextureAlphaMod(bubble4_tex, 176);
	SDLH::LoadImage(&bubble5_tex, "bubble5.png");
	SDL_SetTextureAlphaMod(bubble5_tex, 192);
	

	SDLH::LoadImage(&file_checkbox_false_tex, "icons/File-Checkbox_false.png");
	SDLH::LoadImage(&file_checkbox_true_tex, "icons/File-Checkbox_true.png");
	
	SDLH::LoadImage(&unknown_tex, "icons/File-Unknown.png");
	SDLH::LoadImage(&file_tex, "icons/File-File.png");
	SDLH::LoadImage(&folder_tex, "icons/File-Folder.png");
	SDLH::LoadImage(&sd_tex, "icons/File-SD.png");
	SDLH::LoadImage(&nand_tex, "icons/File-NAND.png");
	SDLH::LoadImage(&disk_tex, "icons/File-Disk.png");
	SDLH::LoadImage(&usb_tex, "icons/File-USB.png");
	
	SDLH::LoadImage(&arrow_left_tex, "icons/Op-Arrow_left.png");
	SDLH::LoadImage(&arrow_right_tex, "icons/Op-Arrow_right.png");
	SDLH::LoadImage(&arrow_up_tex, "icons/Op-Arrow_up.png");
	SDLH::LoadImage(&op_checkbox_false_tex, "icons/Op-Checkbox_false.png");
	SDLH::LoadImage(&op_checkbox_true_tex, "icons/Op-Checkbox_true.png");
	SDLH::LoadImage(&op_checkbox_neutral_tex, "icons/Op-Checkbox_neutral.png");

	SDLH::LoadImage(&open_tex, "icons/Op-Open.png");
	SDLH::LoadImage(&file_new_tex, "icons/Op-File_new.png");
	SDLH::LoadImage(&folder_new_tex, "icons/Op-Folder_new.png");
	SDLH::LoadImage(&copy_tex, "icons/Op-Copy.png");
	SDLH::LoadImage(&cut_tex, "icons/Op-Cut.png");
	SDLH::LoadImage(&paste_tex, "icons/Op-Paste.png");
	SDLH::LoadImage(&delete_tex, "icons/Op-Delete.png");
	SDLH::LoadImage(&rename_tex, "icons/Op-Rename.png");
	SDLH::LoadImage(&settings_tex, "icons/Op-Settings.png");
	SDLH::LoadImage(&properties_tex, "icons/Op-File-info.png");


	LOG("Loading fonts...");
	SDLH::LoadFont(&arial25_font, "fonts/ArialCE.ttf", 25);
	SDLH::LoadFont(&arial25_outline_font, "fonts/ArialCE.ttf", 25);
	SDLH::SetFontOutline(&arial25_outline_font, 2); 
	SDLH::LoadFont(&arial28_font, "fonts/ArialCE.ttf", 28);
	SDLH::LoadFont(&arial30_font, "fonts/ArialCE.ttf", 30);
	SDLH::LoadFont(&arialBold35_font, "fonts/ArialCEBold.ttf", 35);
	SDLH::LoadFont(&arial40_font, "fonts/ArialCE.ttf", 40);
	SDLH::LoadFont(&arialItalic40_font, "fonts/ArialCE.ttf", 40);
	TTF_SetFontStyle(arialItalic40_font, TTF_STYLE_ITALIC);
	SDLH::LoadFont(&arialBold48_font, "fonts/ArialCEBold.ttf", 48);
	SDLH::LoadFont(&arial50_font, "fonts/ArialCE.ttf", 50);
	SDLH::LoadFont(&arialBold80_font, "fonts/ArialCEBold.ttf", 80);


	LOG("Loading music...");
	/*Mix_OpenAudio(48000, AUDIO_S16, 2, 4096);
	Mix_AllocateChannels(1);
	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

	bg_mus = Mix_LoadMUS("bgm.ogg");
	if (bg_mus == nullptr)
		LOG_E("SDL2_Mix error: %s", Mix_GetError());
	else
		Mix_PlayMusic(bg_mus, -1);

	click_sound_ch = Mix_LoadWAV("click.ogg");*/

	//Proc-ui
	//ProcUIInit(&OSSavesDone_ReadyToRelease);

	//Filesystem
	LOG("Initializing filesystem...");
	Filesystem::Init();
	LOG("Initializing bubbles...");
	initBubbles();

	//SWKBD
	SWKBD::Init();

	LOG("Welcome!");
	//Load main menu
	loadMenu_Main();
	
	LOG("Exiting...");
	destroyBubbles();
	LOG("Deinitializing dialogs...");
	DialogHelper::DestroyDialog();
	LOG("Deinitializing SWKBD...");
	SWKBD::Shutdown();
	LOG("Deinitializing filesystem...");
	Filesystem::Shutdown();

	LOG("Deinitializing music...");
	Mix_FreeChunk(click_sound_ch);
	Mix_FreeMusic(bg_mus);
	Mix_CloseAudio(); //Causes a softlock at shutdown

	LOG("Deinitializing fonts...");
	SDLH::UnloadFonts();

	LOG("Deinitializing images...");
	SDLH::UnloadImages();
	
	LOG("Deinitializing mix...");
	//Mix_Quit(); //Unnecessary?
	LOG("Deinitializing ttf...");
	TTF_Quit();
	LOG("Deinitializing img...");
	IMG_Quit();
	LOG("Deinitializing sdl2...");
	SDL_Quit();

	LOG("Deinitializing WHBProc...");
	WHBProcShutdown();
	LOG("Goodbye!");
	WHBLogUdpDeinit();

	return 0;
}