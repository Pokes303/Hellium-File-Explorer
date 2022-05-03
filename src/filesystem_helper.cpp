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
bool deleteClipboardAtEnd = false; //Copy/cut

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

    //Slider resetting
    slider = 0.0;
    sliderY = 100;
    touchedFile = -1;

    //Declare handlers
    std::string errorMessage = "";
    std::string extraInfo = "";

    std::string actualPath = Path::GetPath();
    LOG("Reading directory %s", actualPath.c_str());
    if (actualPath == "/"){
        LOG("Path is virtual (/)");
        Path::SetPathType(PathType::VIRTUAL);

        files.push_back(new FileButton("dev", "Device and hardware links", folder_tex, true));
        files.push_back(new FileButton("vol", "Main volume directory", folder_tex, true));

        setPermissionInfo(FS_MODE_READ_OWNER | FS_MODE_READ_GROUP | FS_MODE_READ_OTHER);
        rewind_b->SetActive(false);
    }
    else if (actualPath == "/vol/"){
        LOG("Path is virtual (/vol/)");
        Path::SetPathType(PathType::VIRTUAL);

        files.push_back(new FileButton("external01", "/dev/sdcard01", "External SD card", sd_tex));
        files.push_back(new FileButton("usb", "/dev/usb01", "External USB drive", usb_tex));
        files.push_back(new FileButton("mlc", "/dev/mlc01", "WiiU NAND", nand_tex));
        files.push_back(new FileButton("slc", "/dev/slc01", "WiiU system partition", nand_tex));
        files.push_back(new FileButton("slccmpt", "/dev/slccmpt01", "vWii NAND", nand_tex));
        files.push_back(new FileButton("ramdisk", "/dev/ramdisk01", "Virtual memory disk", nand_tex));
        files.push_back(new FileButton("odd01", "/dev/odd01", "Disk tickets", disk_tex));
        files.push_back(new FileButton("odd02", "/dev/odd02", "Disk updates", disk_tex));
        files.push_back(new FileButton("odd03", "/dev/odd03", "Disk contents", disk_tex));
        files.push_back(new FileButton("odd04", "/dev/odd04", "Disk contents", disk_tex));

        setPermissionInfo(FS_MODE_READ_OWNER | FS_MODE_READ_GROUP | FS_MODE_READ_OTHER);
        rewind_b->SetActive(true);
    }
    else {
        std::vector<FSDirectoryEntry> items;
        FSStat dirStat;
        FError err = Filesystem::ReadDir(&items, &dirStat, actualPath, actualPath == "/dev/" || forceIOSUHAX);

        switch (err){
        case FError::OK:
            for (uint32_t i = 0; i < items.size(); i++){
                files.push_back(new FileButton(items[i]));
            }
            setPermissionInfo(dirStat.mode);
    
            //Alphabetical sort
            std::sort(files.begin(), files.end(), Utils::AlphabeticalSort);
            break;
        case FError::DIR_READ_EMPTY:
            directoryInfo1 = SDLH::GetText(arial40_font, black_col, "This directory is empty");
            SDLH::ClearTexture(&directoryInfo2);
            break;
        case FError::DIR_OPEN_ERROR:
        default:
            directoryInfo1 = SDLH::GetText(arial40_font, black_col, "Unable to open this directory");
            directoryInfo2 = SDLH::GetText(arial40_font, black_col, Filesystem::GetLastError().c_str());
            break;
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
        "Create new file",
        "Enter here the name of the new file...",
        DialogButtons::CANCEL_OK);
    DialogHelper::SetDialog(dtb);

    std::string textboxResult = "";
    DialogResult res = DialogHelper::WaitForTextboxDialogResponse(textboxResult);

    if (res == DialogResult::CANCELLED_RES)
        return;

    std::string item = Path::GetPath() + textboxResult;
    LOG("Trying to create a file named %s: %s", textboxResult.c_str(), item.c_str());

    FError err = Filesystem::MakeFile(item);
    if (err != FError::OK){
        DialogHelper::SetDialog(new DialogDefault(
            "Operation failed",
            "Error trying to create a file: " + item,
            "Error code: " + Filesystem::GetLastError(),
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }
    FilesystemHelper::ReadPathDir();
    
    DialogHelper::SetDialog(new DialogDefault(
        "Operation ended",
        "Created file: " + item,
        "Click OK to continue",
        DialogButtons::OK));
    DialogHelper::WaitForDialogResponse();
}
void FilesystemHelper::CreateFileProccess(){
    std::thread(internal_createFile).detach();
}

void FilesystemHelper::CreateFolderProccess(){
        DialogTextbox* dtb = new DialogTextbox(
        "Create new folder",
        "Enter here the name of the new folder...",
        DialogButtons::CANCEL_OK);
    DialogHelper::SetDialog(dtb);

    std::string textboxResult = "";
    DialogResult res = DialogHelper::WaitForTextboxDialogResponse(textboxResult);

    if (res == DialogResult::CANCELLED_RES)
        return;

    std::string item = Path::GetPath() + textboxResult;
    LOG("Trying to create a folder named %s: %s", textboxResult.c_str(), item.c_str());

    FError err = Filesystem::MakeDir(item);
    if (err != FError::OK){
        DialogHelper::SetDialog(new DialogDefault(
            "Operation failed",
            "Error trying to create a folder: " + item,
            "Error code: " + Filesystem::GetLastError(),
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }
    FilesystemHelper::ReadPathDir();
    
    DialogHelper::SetDialog(new DialogDefault(
        "Operation ended",
        "Created folder: " + item,
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

    paste_b->SetActive(true);
    deleteClipboardAtEnd = cut;
    DialogHelper::SetDialog(new DialogDefault(
        "Copied files to clipboard",
        std::string("Successfully ") + (deleteClipboardAtEnd ? "cutted " : "copied ") + std::to_string(clipboard.size()) + " " +
            (clipboard.size() > 1 ? "items" : "item"),
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
        "Error fetching files",
        "Items are in the same folder",
        "Click OK to continue",
        DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return;
    }

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
    int nfiles = 0, nfolders = 0;
    std::map<std::string, bool>::iterator it = items.begin();
    while (it != items.end()){
        it->second ? nfolders++ : nfiles++;
    }

    DialogHelper::SetDialog(new DialogDefault(
        "Confirm operation",
        "Found " + std::to_string(nfiles) + " files and " + std::to_string(nfolders) + " folders." + 
        "Do you want to " + (deleteClipboardAtEnd ? "copy" : "cut") + " these items?\n" +
        "\nFrom: " + clipboardPath +
        "\nTo: " + Path::GetPath(),
        "Choose an option",
        DialogButtons::NO_YES));
    if (DialogHelper::WaitForDialogResponse() == DialogResult::NO_RES)
        return;

    int copiedFiles = 0;
    int copyTotal = nfiles + nfolders;

    DialogHelper::SetDialog(new DialogProgressbar(
        "Copying " + std::to_string(copyTotal) + " items...",
        "Copied 0 of " + std::to_string(copyTotal) + " files" + 
        "\nRemove copied files: " + (deleteClipboardAtEnd ? "yes" : "no"),
        "",
        true));

    FError err = FError::OK;
    std::string failedItem = "";
    while (it != items.end()){
        if (it->second)
            err = Filesystem::MakeDir(Path::GetPath() + it->first);
        else
            err = Filesystem::CopyFile(clipboardPath + it->first, Path::GetPath() + it->first, deleteClipboardAtEnd);
        if (err != FError::OK){
            failedItem = it->first;
            break;
        }
        
        DialogProgressbar* d = (DialogProgressbar*)DialogHelper::GetDialog();
        d->SetDescription("Copied " + std::to_string(++copiedFiles) + " of " + std::to_string(copyTotal) + " files" +
            "\nRemove copied files: " + (deleteClipboardAtEnd ? "yes" : "no"));
        d->SetFooter(it->first);
        d->SetProgressBar((float)copiedFiles / (float)copyTotal);
    }
    FilesystemHelper::ReadPathDir();

    if (deleteClipboardAtEnd){
        clipboard.clear();
        paste_b->SetActive(false);
    }

    if (err != FError::OK) {
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
    int nfiles = 0, nfolders = 0;
    std::map<std::string, bool>::iterator it = items.begin();
    while (it != items.end()){
        (it->second) ? nfolders++ : nfiles++;
    }

    DialogHelper::SetDialog(new DialogDefault(
        "Confirm operation",
        "Found " + std::to_string(nfiles) + " files and " + std::to_string(nfolders) + " folders." + 
        "Do you want to remove these items?\n" +
        "\nFrom: " + clipboardPath,
        "Choose an option",
        DialogButtons::NO_YES));
    if (DialogHelper::WaitForDialogResponse() == DialogResult::NO_RES)
        return;

    int removedFiles = 0;
    int removedTotal = nfiles + nfolders;

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
        "Error deleting the item " + failedItem +
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