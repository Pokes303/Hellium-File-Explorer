#include "filesystem_helper.hpp"
#include "menus/menu_main.hpp"
#include "utils.hpp"
#include "gui/dialog/dialog.hpp"
#include "gui/dialog/dialog_default.hpp"
#include "gui/dialog/dialog_progressbar.hpp"
#include "gui/dialog/dialog_textbox.hpp"
#include "SDL_Helper.hpp"
#include "udplog.hpp"
#include "dialog_helper.hpp"
#include "gui/path.hpp"
#include "filesystem.hpp"

std::vector<FileButton*> files;
std::vector<std::string> clipboard;
std::string clipboardPath;
bool clipboardCut = false;

void setPermissionInfo(uint32_t perms){
    folderPerms = (perms &  FS_MODE_READ_OWNER) ? "r" : "-";
    folderPerms += (perms &  FS_MODE_WRITE_OWNER) ? "w" : "-";
    folderPerms += (perms &  FS_MODE_EXEC_OWNER) ? "x" : "-";
    folderPerms += " | ";
    folderPerms += (perms &  FS_MODE_READ_GROUP) ? "r" : "-";
    folderPerms += (perms &  FS_MODE_WRITE_GROUP) ? "w" : "-";
    folderPerms += (perms &  FS_MODE_EXEC_GROUP) ? "x" : "-";
    folderPerms += " | ";
    folderPerms += (perms &  FS_MODE_READ_OTHER) ? "r" : "-";
    folderPerms += (perms &  FS_MODE_WRITE_OTHER) ? "w" : "-";
    folderPerms += (perms &  FS_MODE_EXEC_OTHER) ? "x" : "-";
}

void FilesystemHelper::ReadPathDir(){
    ClearPathDir();
    LOG("Reading directory: %s", actualPath.c_str());

    //Slider resetting
    slider = 0.0;
    sliderY = 100;
    touchedFile = -1;

    //Declare handlers
    std::string errorMessage = "";
    std::string extraInfo = "";

    std::string actualPath = Path::GetPath();
    if (actualPath == "/"){
        Path::SetPathType(PathType::VIRTUAL);

        files.push_back(new FileButton("dev", "Device and hardware links", FileButtonType::FOLDER));
        files.push_back(new FileButton("vol", "Main volume directory", FileButtonType::FOLDER));

        setPermissionInfo(FS_MODE_READ_OWNER | FS_MODE_READ_GROUP | FS_MODE_READ_OTHER);
        rewind_b->SetActive(false);
    }
    else if (actualPath == "/vol/"){
        Path::SetPathType(PathType::VIRTUAL);

        //If doesn't works, add '01' to mlc, slc...
        files.push_back(new FileButton("external01", "External SD card", FileButtonType::DRIVE_SD));
        files.push_back(new FileButton("usb", "External USB drive", FileButtonType::DRIVE_USB));
        files.push_back(new FileButton("mlc", "WiiU NAND", FileButtonType::DRIVE_NAND));
        files.push_back(new FileButton("slc", "WiiU system partition", FileButtonType::DRIVE_NAND));
        files.push_back(new FileButton("slccmpt", "vWii NAND", FileButtonType::DRIVE_NAND));
        files.push_back(new FileButton("ramdisk", "Virtual memory disk", FileButtonType::DRIVE_NAND));
        files.push_back(new FileButton("odd01", "Disk tickets", FileButtonType::DRIVE_DISK));
        files.push_back(new FileButton("odd02", "Disk updates", FileButtonType::DRIVE_DISK));
        files.push_back(new FileButton("odd03", "Disk contents", FileButtonType::DRIVE_DISK));
        files.push_back(new FileButton("odd04", "Disk contents", FileButtonType::DRIVE_DISK));

        setPermissionInfo(FS_MODE_READ_OWNER | FS_MODE_READ_GROUP | FS_MODE_READ_OTHER);
        rewind_b->SetActive(true);
    }
    else {
        std::vector<FSDirectoryEntry> items;
        FSStat dirStat;
        bool err = Filesystem::ReadDir(&items, &dirStat, actualPath);
        if (err){
            directoryInfo1 = SDLH::GetText(arial40_font, black_col, "Unable to open this directory");
            directoryInfo2 = SDLH::GetText(arial40_font, red_col, Filesystem::GetLastError().c_str());
        }
        else{
            if (items.size() > 0){
                for (uint32_t i = 0; i < items.size(); i++){
                    files.push_back(new FileButton(items[i]));
                }
        
                //Alphabetical sort
                std::sort(files.begin(), files.end(), Utils::AlphabeticalSort);
            }
            else{
                directoryInfo1 = SDLH::GetText(arial40_font, black_col, "Empty folder");
                SDLH::ClearTexture(&directoryInfo2);
            }
            setPermissionInfo(dirStat.mode);
        }
        rewind_b->SetActive(true);
    }
    LOG("Total files: %d", files.size());
    
    permissions_tex = SDLH::GetText(arial50_font, black_col, folderPerms.c_str());
    checkedItems_tex = SDLH::GetTextf(arial50_font, black_col, "%d/%d", selectedItems, files.size());
}

void FilesystemHelper::ClearPathDir(){
    for (uint32_t i = 0; i < files.size(); i++){
        delete files[i];
    }
    files.clear();

    selectedItems = 0;
    checkbox_b->SetTexture(op_checkbox_false_tex);
    copy_b->SetActive(false);
    cut_b->SetActive(false);
    delete_b->SetActive(false);
    rename_b->SetActive(false);
    if (directoryInfo1)
        SDL_DestroyTexture(directoryInfo1);
    if (directoryInfo2)
        SDL_DestroyTexture(directoryInfo2);
    folderPerms = "? | ? | ?";
}

void FilesystemHelper::SetPathDir(std::string dir){
    Path::SetPath(dir);
}

void FilesystemHelper::ChangePathDir(std::string dir){
    SetPathDir(Path::GetPath() + dir + '/');
}

void FilesystemHelper::RewindPath(){
    std::string path = Path::GetPath();

    if (path.size() <= 1)
        return;

    for (int i = path.size() - 1; i > 0; i--){
        path.erase(path.end() - 1, path.end());
        if (path[i - 1] == '/'){
            break;
        }
    }
    Path::SetPath(path);
}

void internal_createFile(){
    DialogTextbox* dtb = new DialogTextbox(
        "Create a new file",
        "Enter here a name for the new file...",
        DialogButtons::CANCEL_OK);
    DialogHelper::SetDialog(dtb);

    std::string textboxResult = "";
    DialogResult res = DialogHelper::WaitForTextboxDialogResponse(textboxResult);

    if (res == DialogResult::CANCELLED_RES)
        return;

    std::string item = Path::GetPath() + textboxResult;
    bool err = Filesystem::MakeFile(item);
    if (err){
        DialogHelper::SetDialog(new DialogDefault(
            "Operation failed",
            "Error creating the file: " + textboxResult,
            "Reason: " + Filesystem::GetLastError(),
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }
    
    DialogHelper::SetDialog(new DialogDefault(
        "Operation succeeded",
        "File created: " + textboxResult,
        "Click OK to continue",
        DialogButtons::OK));
    DialogHelper::WaitForDialogResponse();

    FilesystemHelper::ReadPathDir();
}
void FilesystemHelper::CreateFileProccess(){
    std::thread(internal_createFile).detach();
}

void FilesystemHelper::CreateFolderProccess(){
        DialogTextbox* dtb = new DialogTextbox(
        "Create a new folder",
        "Enter here a name for the new folder...",
        DialogButtons::CANCEL_OK);
    DialogHelper::SetDialog(dtb);

    std::string textboxResult = "";
    DialogResult res = DialogHelper::WaitForTextboxDialogResponse(textboxResult);

    if (res == DialogResult::CANCELLED_RES)
        return;

    std::string item = Path::GetPath() + textboxResult;
    bool err = Filesystem::MakeDir(item);
    if (err){
        DialogHelper::SetDialog(new DialogDefault(
            "Operation failed",
            "Error creating the file: " + textboxResult,
            "Reason: " + Filesystem::GetLastError(),
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }
    FilesystemHelper::ReadPathDir();
    
    DialogHelper::SetDialog(new DialogDefault(
        "Operation succeeded",
        "File created: " + textboxResult,
        "Click OK to continue",
        DialogButtons::OK));
    DialogHelper::WaitForDialogResponse();
}

void internal_copyClipboard(bool cut){
    clipboard.clear();

    std::string path = Path::GetPath();
    for (uint32_t i = 0; i < files.size(); i++){
        if (files[i]->IsSelected()){
            files[i]->SetSelection(false);
            clipboard.push_back(files[i]->GetName() + (files[i]->IsDirectory() ? "/" : ""));
        }
    }
    clipboardPath = path;
    clipboardCut = cut;

    paste_b->SetActive(true);
    DialogHelper::SetDialog(new DialogDefault(
        "Items copied!",
        (cut ? "Cut " : "Copied ") + std::to_string(clipboard.size()) + " " +
            (clipboard.size() > 1 ? "items" : "item") + std::string(" to the clipboard"),
        "Click OK to continue",
        DialogButtons::OK));
    DialogHelper::WaitForDialogResponse();
}
void FilesystemHelper::CopyProccess(bool cut) {
    std::thread(internal_copyClipboard, cut).detach();
}

void internal_paste(){
    if (clipboardPath == Path::GetPath()){
        DialogHelper::SetDialog(new DialogDefault(
        "Operation failed",
        "Items are in the same folder",
        "Click OK to continue",
        DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }

    DialogHelper::SetDialog(new DialogDefault(
        "Calculating files...",
        "",
        "\nWait for the confirmation...",
        DialogButtons::NONE));

    //Iterate clipboard
    std::map<std::string, bool> items;
    for (uint32_t i = 0; i < clipboard.size(); i++){
        bool isDir = Filesystem::DirExists(clipboard[i]);
        items.insert(std::make_pair(clipboard[i], isDir));
        if (isDir)
            Filesystem::ReadDirRecursive(&items, clipboardPath, clipboard[i]);
    }

    //Iterate items
    int nFiles = 0, nFolders = 0;
    std::map<std::string, bool>::iterator it = items.begin();
    while (it != items.end()){
        it->second ? nFolders++ : nFiles++;
    }



    
    //CONTINUE FROM HERE





    DialogHelper::SetDialog(new DialogDefault(
        "Confirm operation",
        "Found " + std::to_string(nFiles) + " files and " + std::to_string(nFolders) + " folders." + 
        "Do you want to paste them?\n" +
        "\nFrom: " + clipboardPath +
        "\nTo: " + Path::GetPath(),
        "Choose an option",
        DialogButtons::NO_YES));
    if (DialogHelper::WaitForDialogResponse() == DialogResult::NO_RES)
        return;

    int copiedFiles = 0;
    int copyTotal = nFiles + nFolders;

    DialogHelper::SetDialog(new DialogProgressbar(
        "Copying items...",
        "Pasted 0 of " + std::to_string(copyTotal) + " files",
        "",
        true));

    std::string failedItem = "";
    while (it != items.end()){
        bool err;
        if (it->second){
            //Folder
            err = Filesystem::MakeDir(Path::GetPath() + it->first);
        }
        else{
            //File
            err = Filesystem::CopyFile(clipboardPath + it->first, Path::GetPath() + it->first, deleteClipboardAtEnd);
        }
        
        if (err){
            failedItem = it->first;
            break;
        }
        
        DialogProgressbar* d = (DialogProgressbar*)DialogHelper::GetDialog();
        d->SetDescription("Pasted " + std::to_string(++copiedFiles) + " of " + std::to_string(copyTotal) + " files");
        d->SetFooter(it->first);
        d->SetProgressBar((float)copiedFiles / (float)copyTotal);
    }
    FilesystemHelper::ReadPathDir();

    if (clipboardCut){
        clipboard.clear();
        paste_b->SetActive(false);
    }

    if (failedItem != "") {
        DialogHelper::SetDialog(new DialogDefault("Operation failed",
        "Error copying the item " + failedItem +
        "\n\nCopied files before failed: " + std::to_string(copiedFiles) +
        "\nRemaining files: " + std::to_string(copyTotal - copiedFiles),
        "Error code: " + Filesystem::GetLastError(),
        DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }
    
    DialogHelper::SetDialog(new DialogDefault(
        "Operation ended",
        "Copied items: " + std::to_string(copiedFiles) + "/" + std::to_string(copyTotal),
        "Click OK to continue",
        DialogButtons::OK));
    DialogHelper::WaitForDialogResponse();
}

void FilesystemHelper::PasteProccess(){
    if (clipboard.size() <= 0)
        return;
    
    std::thread(internal_paste).detach();
}

void internal_delete(){
        DialogHelper::SetDialog(new DialogDefault(
        "Fetching files...",
        "Calculating number of files",
        "\nWait for the confirmation...",
        DialogButtons::NONE));

    //Iterate clipboard
    std::map<std::string, bool> items;
    for (uint32_t i = 0; i < clipboard.size(); i++){
        bool isDir = Filesystem::DirExists(clipboard[i]);
        items.insert(std::make_pair(clipboard[i], isDir));
        if (isDir)
            Filesystem::ReadDirRecursive(&items, clipboardPath, clipboard[i]);
    }

    //Iterate items
    int nFiles = 0, nFolders = 0;
    std::map<std::string, bool>::iterator it = items.begin();
    while (it != items.end()){
        (it->second) ? nFolders++ : nFiles++;
    }

    DialogHelper::SetDialog(new DialogDefault(
        "Confirm operation",
        "Found " + std::to_string(nFiles) + " files and " + std::to_string(nFolders) + " folders." + 
        "Do you want to remove these items?\n" +
        "\nFrom: " + clipboardPath,
        "Choose an option",
        DialogButtons::NO_YES));
    if (DialogHelper::WaitForDialogResponse() == DialogResult::NO_RES)
        return;

    int removedFiles = 0;
    int removedTotal = nFiles + nFolders;

    DialogHelper::SetDialog(new DialogProgressbar(
        "Copying " + std::to_string(removedTotal) + " items...",
        "Removed 0 of " + std::to_string(removedTotal) + " files",
        "",
        true));

    FError err = FError::OK;
    std::string failedItem = "";
    while (it != items.end()){
        err = Filesystem::Delete(clipboardPath + it->first);
        if (err != FError::OK){
            failedItem = it->first;
            break;
        }
        
        DialogProgressbar* d = (DialogProgressbar*)DialogHelper::GetDialog();
        d->SetDescription("Removed " + std::to_string(++removedFiles) + " of " + std::to_string(removedTotal) + " files");
        d->SetFooter(it->first);
        d->SetProgressBar((float)removedFiles / (float)removedTotal);
    }
    FilesystemHelper::ReadPathDir();

    clipboard.clear();
    paste_b->SetActive(false);

    if (err != FError::OK) {
        DialogHelper::SetDialog(new DialogDefault("Operation failed",
        "Error deleting the item " + Utils::GetFilename(failedItem) +
        "\n\nRemoved files before failed: " + std::to_string(removedFiles) +
        "\nRemaining files: " + std::to_string(removedFiles - removedTotal),
        "Error code: " + Filesystem::GetLastError(),
        DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }
    
    DialogHelper::SetDialog(new DialogDefault(
        "Operation ended",
        "Removed items: " + std::to_string(removedFiles) + "/" + std::to_string(removedTotal),
        "Click OK to continue",
        DialogButtons::OK));
    DialogHelper::WaitForDialogResponse();
}

void FilesystemHelper::DeleteProccess(){
    std::thread(internal_delete).detach();
}

void FilesystemHelper::RenameProccess(){
    
}