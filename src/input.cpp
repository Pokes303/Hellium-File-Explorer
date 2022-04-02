#include "input.hpp"
#include "udplog.hpp"

VPADStatus vpad;
VPADReadError vpaderror;
TouchData touch;

nn::swkbd::CreateArg carg;
FSClient* swkbd_cli;
bool isShown = false;
int disappearingFrames = -1;

int swkbd = 0;

void Input::ReadInput(){
	VPADRead(VPAD_CHAN_0, &vpad, 1, &vpaderror);
	if (vpaderror != VPAD_READ_SUCCESS)
		LOG("[main.cpp]>Error: VPAD error check returned (%d)", vpaderror);

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
			LOG_E("Unknown touch status value (%d)", touch.status);
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
		LOG_E("Failed creating SWKBD");
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

void SWKBD::Appear(){
	nn::swkbd::AppearArg aarg;
	aarg.keyboardArg.configArg.languageType = nn::swkbd::LanguageType::English;
	aarg.keyboardArg.configArg.controllerType = nn::swkbd::ControllerType::DrcGamepad;
	aarg.keyboardArg.configArg.keyboardMode = nn::swkbd::KeyboardMode::Utf8;
	aarg.keyboardArg.configArg.okString = u"GO";
	aarg.keyboardArg.configArg.showWordSuggestions = false;
	aarg.keyboardArg.configArg.disableNewLine = true;
	
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	std::u16string u16path = convert.from_bytes(path);
	aarg.inputFormArg.initialText = u16path.c_str();
	aarg.inputFormArg.hintText = u"Enter the new path here";
	aarg.inputFormArg.showCopyPasteButtons = true;
	if (!nn::swkbd::AppearInputForm(aarg)){
		LOG_E("Failed appearing SWKBD");
		return;
	}
	swkbd++;
	isShown = true;
}

bool SWKBD::IsShown(){
	return isShown;
}

SWKBD::Result SWKBD::IsFinished(){
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
			isShown = false;
			return Result::NOT_FINISHED;
		}
		disappearingFrames++;
		return Result::NOT_FINISHED;
	}

	if (nn::swkbd::IsDecideCancelButton(nullptr)) {
		nn::swkbd::DisappearInputForm();
		disappearingFrames = 0;
		return Result::CANCEL;
	}
	else if (nn::swkbd::IsDecideOkButton(nullptr)) {
		nn::swkbd::DisappearInputForm();
		disappearingFrames = 0;
		return Result::OK;
	}
	return Result::NOT_FINISHED;
}

std::string SWKBD::GetResult(){
	const char16_t *str = nn::swkbd::GetInputFormString();
	std::u16string u16res = str;
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
	return convert.to_bytes(u16res);
}