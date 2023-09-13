#include "input.hpp"
#include "udplog.hpp"
#include "gui/path.hpp"

VPADStatus vpad;
VPADReadError vpaderror;
TouchData touch;

nn::swkbd::CreateArg carg;
FSClient* swkbd_cli;
void (*okCallback)(std::string result) = nullptr;
int disappearingFrames = -1;

void Input::ReadInput(){
	VPADRead(VPAD_CHAN_0, &vpad, 1, &vpaderror);
	switch(vpaderror){
		case VPAD_READ_SUCCESS:
			break;
		case VPAD_READ_NO_SAMPLES:
			return;
		case VPAD_READ_INVALID_CONTROLLER:
			LOG_E("VPAD invalid controller");
			break;
		default:
			LOG_E("VPAD unknown error: %d", vpaderror);
			break;
	}

	switch (touch.status)
	{
		case NOT_TOUCHED: //didn't touch
			if (vpad.tpNormal.touched)
				touch.status = TOUCHED_DOWN;
			break;
		case TOUCHED_DOWN: //touched once
			touch.status = TOUCHED_HELD;
			//LOG("Touch down");
			break;
		case TOUCHED_HELD:
			//LOG("Touch held");
			if (!vpad.tpNormal.touched)
				touch.status = TOUCHED_UP;
			break;
		case TOUCHED_UP:
			touch.status = NOT_TOUCHED;
			//LOG("Touch up");
			break;
		default: //unknown status
			touch.status = NOT_TOUCHED;
			LOG_E("Unknown touch status value <%d>", touch.status);
			break;
	}
    if (vpad.tpNormal.touched) {
	    VPADGetTPCalibratedPoint(VPAD_CHAN_0, &vpad.tpNormal, &vpad.tpNormal);
        touch.x = vpad.tpNormal.x;
        touch.y = vpad.tpNormal.y;
        //LOG("Touch %d, %d", touch.x, touch.y);
    }
}

void SWKBD::Init(){
	swkbd_cli = (FSClient*)MEMAllocFromDefaultHeap(sizeof(FSClient));
	FSAddClient(swkbd_cli, FS_ERROR_FLAG_NONE);

	carg.regionType = nn::swkbd::RegionType::Europe;
	carg.workMemory = MEMAllocFromDefaultHeap(nn::swkbd::GetWorkMemorySize(0));
	carg.fsClient = cli;
	if (!nn::swkbd::Create(carg)){
		LOG_E("nn::swkbd::Create failed");
		return;
	}

	nn::swkbd::MuteAllSound(false);
}

void SWKBD::Shutdown(){
	nn::swkbd::Destroy();
	MEMFreeToDefaultHeap(carg.workMemory);

	FSDelClient(swkbd_cli, FS_ERROR_FLAG_NONE);
	MEMFreeToDefaultHeap(swkbd_cli);
}

void SWKBD::Appear(std::string text, std::string hint, void* callback){
	LOG("Appearing SWKBD...");
	
	nn::swkbd::AppearArg aarg;
	aarg.keyboardArg.configArg.languageType = nn::swkbd::LanguageType::English;
	aarg.keyboardArg.configArg.controllerType = nn::swkbd::ControllerType::DrcGamepad;
	aarg.keyboardArg.configArg.keyboardMode = nn::swkbd::KeyboardMode::Utf8;
	aarg.keyboardArg.configArg.okString = u"GO";
	aarg.keyboardArg.configArg.showWordSuggestions = false;
	aarg.keyboardArg.configArg.disableNewLine = true;
	
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	std::u16string u16text = convert.from_bytes(text);
	aarg.inputFormArg.initialText = u16text.c_str();
	std::u16string u16hint = convert.from_bytes(text);
	aarg.inputFormArg.hintText = u16hint.c_str();
	aarg.inputFormArg.showCopyPasteButtons = true;
	if (!nn::swkbd::AppearInputForm(aarg)){
		LOG_E("Failed appearing SWKBD");
		return;
	}
	
	okCallback = (void (*)(std::string result))callback;
}

bool SWKBD::IsShown(){
	return okCallback != nullptr;
}

void SWKBD::Render(){
	nn::swkbd::ControllerInfo cinfo;
	cinfo.vpad = &vpad;
	cinfo.kpad[0] = nullptr;
	cinfo.kpad[1] = nullptr;
	cinfo.kpad[2] = nullptr;
	cinfo.kpad[3] = nullptr;
	nn::swkbd::Calc(cinfo);

	if (nn::swkbd::IsNeedCalcSubThreadFont()) {
		nn::swkbd::CalcSubThreadFont();
	}

	if (nn::swkbd::IsNeedCalcSubThreadPredict()) {
		nn::swkbd::CalcSubThreadPredict();
	}

	if (disappearingFrames >= 0){
		if (disappearingFrames > 14){
			disappearingFrames = -1;
			okCallback = nullptr;
			return;
		}
		disappearingFrames++;
	}

	if (nn::swkbd::IsDecideCancelButton(nullptr)) {
		nn::swkbd::DisappearInputForm();
		disappearingFrames = 0;
	}
	else if (nn::swkbd::IsDecideOkButton(nullptr)) {
		nn::swkbd::DisappearInputForm();
		disappearingFrames = 0;
		okCallback(GetResult());
	}
}

std::string SWKBD::GetResult(){
	const char16_t *str = nn::swkbd::GetInputFormString();
	std::u16string u16res = str;
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(u16res);
}