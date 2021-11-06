#include "SDL_Helper.hpp"
#include "menus/menu_main.hpp"
#include "filesystem.hpp"
//#include "exception_handler.h"

//GENERAL
SDL_Renderer* renderer;
VPADStatus vpad;
VPADReadError vpaderror;

TouchStatus touchStatus = NOT_TOUCHED;
int layer = 0;

std::string path;
bool pathAnimation;
int pathTextW = 0;
int pathAnimationPhase = 0;
std::vector<std::string> previousPaths;
uint32_t previousPathPos = 0;
int pathType = 0;
float slider = 0;
int sliderY = 100;
int nfiles = 0;
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

SDL_Color dark_red_col = { 192, 0, 0, 255 };

//IMAGES
SDL_Texture* void_tex;

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

SDL_Texture* dialog_tex;
SDL_Texture* dialog_progress_bar_tex;
SDL_Texture* dialog_progress_bar_status_tex;

SDL_Texture* loading_tex;

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

SDL_Texture* dialog_ok_tex;
SDL_Texture* dialog_cancel_tex;
SDL_Texture* dialog_yes_tex;
SDL_Texture* dialog_no_tex;

//FONTS
TTF_Font* arial25_font;
TTF_Font* arial28_font;
TTF_Font* arial30_font;
TTF_Font* arial40_font;
TTF_Font* arial50_font;
TTF_Font* arialBold35_font;
TTF_Font* arialBold80_font;

//SOUNDS
Mix_Music* bg_music;
Mix_Chunk* click_sound;

int main() {
	WHBLogUdpInit();
	WHBLogPrint("[main.cpp]>Log: Initializing libraries...");
	WHBProcInit();
	romfsInit();

	WHBLogPrint("[main.cpp]>Log: Initializing SDL2...");
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

	SDL_Window* window = SDL_CreateWindow("file explorer", 0, 0, 1280, 720, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	SDL_Color col = { 0, 0, 0, 255 };
	SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

	//Colors

	//Initialise images
	WHBLogPrint("[main.cpp]>Log: Loading images");
	SDL_LoadImage(&menu_left_tex, ROMFS_PATH "menu_left.png");
	SDL_LoadImage(&menu_up_tex, ROMFS_PATH "menu_up.png");
	
	SDL_LoadImage(&path_bottom, ROMFS_PATH "path_bottom.png");
	SDL_LoadImage(&path_shadow, ROMFS_PATH "path_shadow.png");
	SDL_LoadImage(&checked_items, ROMFS_PATH "icons/checked_items.png");

	SDL_LoadImage(&button0_tex, ROMFS_PATH "button0.png");
	SDL_LoadImage(&button0_deactivated_tex, ROMFS_PATH "button0_deactivated.png");
	SDL_LoadImage(&button1_tex, ROMFS_PATH "button1.png");
	SDL_LoadImage(&button1_deactivated_tex, ROMFS_PATH "button1_deactivated.png");
	SDL_LoadImage(&button2_tex, ROMFS_PATH "button2.png");
	SDL_LoadImage(&button2_deactivated_tex, ROMFS_PATH "button2_deactivated.png");
	SDL_LoadImage(&button3_tex, ROMFS_PATH "button3.png");
	SDL_LoadImage(&button3_deactivated_tex, ROMFS_PATH "button3_deactivated.png");
	SDL_LoadImage(&button4_tex, ROMFS_PATH "button4.png");
	SDL_LoadImage(&button4_deactivated_tex, ROMFS_PATH "button4_deactivated.png");
	SDL_LoadImage(&dialog_button1_tex, ROMFS_PATH "dialog_button1.png");
	SDL_LoadImage(&dialog_button1_deactivated_tex, ROMFS_PATH "dialog_button1_deactivated.png");
	SDL_LoadImage(&dialog_button2_tex, ROMFS_PATH "dialog_button2.png");
	SDL_LoadImage(&dialog_button2_deactivated_tex, ROMFS_PATH "dialog_button2_deactivated.png");
	SDL_LoadImage(&file_slot_tex, ROMFS_PATH "file_slot.png");
	SDL_LoadImage(&file_slot_touched_tex, ROMFS_PATH "file_slot_touched.png");
	
	SDL_LoadImage(&button_slider_tex, ROMFS_PATH "button_slider.png");
	SDL_LoadImage(&button_slider_deactivated_tex, ROMFS_PATH "button_slider_deactivated.png");
	SDL_LoadImage(&slider_path_tex, ROMFS_PATH "slider_path.png");
	
	SDL_LoadImage(&dialog_tex, ROMFS_PATH "dialog.png");
	SDL_LoadImage(&dialog_progress_bar_tex, ROMFS_PATH "dialog_progress_bar.png");
	SDL_LoadImage(&dialog_progress_bar_status_tex, ROMFS_PATH "dialog_progress_bar_status.png");
	SDL_LoadImage(&loading_tex, ROMFS_PATH "loading.png");
	

	SDL_LoadImage(&file_checkbox_false_tex, ROMFS_PATH "icons/File-Checkbox_false.png");
	SDL_LoadImage(&file_checkbox_true_tex, ROMFS_PATH "icons/File-Checkbox_true.png");
	
	SDL_LoadImage(&unknown_tex, ROMFS_PATH "icons/File-Unknown.png");
	SDL_LoadImage(&file_tex, ROMFS_PATH "icons/File-File.png");
	SDL_LoadImage(&folder_tex, ROMFS_PATH "icons/File-Folder.png");
	SDL_LoadImage(&sd_tex, ROMFS_PATH "icons/File-SD.png");
	SDL_LoadImage(&nand_tex, ROMFS_PATH "icons/File-NAND.png");
	SDL_LoadImage(&disk_tex, ROMFS_PATH "icons/File-Disk.png");
	SDL_LoadImage(&usb_tex, ROMFS_PATH "icons/File-USB.png");
	
	SDL_LoadImage(&arrow_left_tex, ROMFS_PATH "icons/Op-Arrow_left.png");
	SDL_LoadImage(&arrow_right_tex, ROMFS_PATH "icons/Op-Arrow_right.png");
	SDL_LoadImage(&arrow_up_tex, ROMFS_PATH "icons/Op-Arrow_up.png");
	SDL_LoadImage(&op_checkbox_false_tex, ROMFS_PATH "icons/Op-Checkbox_false.png");
	SDL_LoadImage(&op_checkbox_true_tex, ROMFS_PATH "icons/Op-Checkbox_true.png");
	SDL_LoadImage(&op_checkbox_neutral_tex, ROMFS_PATH "icons/Op-Checkbox_neutral.png");

	SDL_LoadImage(&open_tex, ROMFS_PATH "icons/Op-Open.png");
	SDL_LoadImage(&file_new_tex, ROMFS_PATH "icons/Op-File_new.png");
	SDL_LoadImage(&folder_new_tex, ROMFS_PATH "icons/Op-Folder_new.png");
	SDL_LoadImage(&copy_tex, ROMFS_PATH "icons/Op-Copy.png");
	SDL_LoadImage(&cut_tex, ROMFS_PATH "icons/Op-Cut.png");
	SDL_LoadImage(&paste_tex, ROMFS_PATH "icons/Op-Paste.png");
	SDL_LoadImage(&delete_tex, ROMFS_PATH "icons/Op-Delete.png");
	SDL_LoadImage(&rename_tex, ROMFS_PATH "icons/Op-Rename.png");
	SDL_LoadImage(&settings_tex, ROMFS_PATH "icons/Op-Settings.png");
	SDL_LoadImage(&properties_tex, ROMFS_PATH "icons/Op-File-info.png");
	
	SDL_LoadImage(&dialog_ok_tex, ROMFS_PATH "icons/Dialog-Ok.png");
	SDL_LoadImage(&dialog_cancel_tex, ROMFS_PATH "icons/Dialog-Cancel.png");
	SDL_LoadImage(&dialog_yes_tex, ROMFS_PATH "icons/Dialog-Yes.png");
	SDL_LoadImage(&dialog_no_tex, ROMFS_PATH "icons/Dialog-No.png");

	//Initialise fonts
	WHBLogPrint("[main.cpp]>Log: Loading fonts");
	arial25_font = TTF_OpenFont(ROMFS_PATH "fonts/ArialCE.ttf", 25);
	arial28_font = TTF_OpenFont(ROMFS_PATH "fonts/ArialCE.ttf", 28);
	arial30_font = TTF_OpenFont(ROMFS_PATH "fonts/ArialCE.ttf", 30);
	arial40_font = TTF_OpenFont(ROMFS_PATH "fonts/ArialCE.ttf", 40);
	arial50_font = TTF_OpenFont(ROMFS_PATH "fonts/ArialCE.ttf", 50);
	arialBold35_font = TTF_OpenFont(ROMFS_PATH "fonts/ArialCEBold.ttf", 35);
	arialBold80_font = TTF_OpenFont(ROMFS_PATH "fonts/ArialCEBold.ttf", 80);

	//Initialise music
	WHBLogPrint("[main.cpp]>Log: Loading sound");
	Mix_OpenAudio(48000, AUDIO_S16, 2, 4096);
	Mix_AllocateChannels(1);

	bg_music = Mix_LoadMUS(ROMFS_PATH "bgm.wav");
	Mix_PlayMusic(bg_music, -1);
	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
	if (bg_music == nullptr)
		WHBLogPrintf("[main.cpp]>Error: SDL2_Mix error: %s", Mix_GetError());

	click_sound = Mix_LoadWAV(ROMFS_PATH "click.mp3");

	//Proc-ui
	//ProcUIInit(&OSSavesDone_ReadyToRelease);

	//Filesystem
	Filesystem::Init();

	WHBLogPrint("[main.cpp]>Log: Welcome!");
	//Load main menu
	loadMenu_Main();
	
	//Filesystem
	Filesystem::Shutdown();

	WHBLogPrintf("[main.cpp]>Log: Deinitializing...");
	Mix_FreeChunk(click_sound);
	click_sound = NULL;
	Mix_FreeMusic(bg_music);
	bg_music = NULL;
	Mix_CloseAudio();
	Mix_Quit();

	SDL_UnloadFonts();
	TTF_Quit();

	SDL_UnloadImages();
	IMG_Quit();
	SDL_Quit();

	romfsExit();
	WHBProcShutdown();
	WHBLogPrintf("[main.cpp]>Log: Goodbye!");
	WHBLogUdpDeinit();

	return 0;
}

void readVPAD(){
	VPADRead(VPAD_CHAN_0, &vpad, 1, &vpaderror);
	if (vpaderror != VPAD_READ_SUCCESS)
		WHBLogPrintf("[main.cpp]>Error: VPAD error check returned (%d)", vpaderror);

	switch (touchStatus)
	{
		case NOT_TOUCHED: //didn't touch
			if (vpad.tpNormal.touched)
				touchStatus = TOUCHED_DOWN;
			break;
		case TOUCHED_DOWN: //touched once
			touchStatus = TOUCHED_HELD;
			WHBLogPrint("Touched down");
			break;
		case TOUCHED_HELD:
			WHBLogPrint("Touched held");
			if (!vpad.tpNormal.touched)
				touchStatus = TOUCHED_UP;
			break;
		case TOUCHED_UP:
			touchStatus = NOT_TOUCHED;
			WHBLogPrint("Touched up");
			break;
		default: //unknown status
			touchStatus = NOT_TOUCHED;
			WHBLogPrint("[main.cpp]>Log: Unknown touchStatus value");
			break;
	}

	VPADGetTPCalibratedPoint(VPAD_CHAN_0, &vpad.tpNormal, &vpad.tpNormal);
}

/*bool AppRunning(){
	if (!OSIsMainCore())
		ProcUISubProcessMessages(true);
	else{
		ProcUIStatus status = ProcUIProcessMessages(true);
		switch (status)
		{
		case PROCUI_STATUS_IN_FOREGROUND:
			return true;
		case PROCUI_STATUS_RELEASE_FOREGROUND:
			ProcUIDrawDoneRelease();
		}
	}
}*/