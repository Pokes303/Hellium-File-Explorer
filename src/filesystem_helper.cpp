#include "filesystem_gui.hpp"
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

#define IOSUHAX_END_OF_DIR (int32_t)0xFFFCFFFC

std::vector<FileButton*> files;
std::vector<std::string> clipboard;
std::string clipboardPath;
bool deleteClipboardAtEnd = false; //Copy/cut
#define COPY_BUFFER_SIZE 1024 * 1024 * 2 //2MB
void* copyBuffer;

std::vector<std::string> mountedDrives;

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

bool fs_ReadDir(std::string _path, std::string& errorMessage, std::string& extraInfo){
    LOG("Reading with FS...");

    Path::SetPathType(PathType::REAL);
    FSDirectoryHandle handle;
    int status = FSOpenDir(cli, block, _path.c_str(), &handle, FS_ERROR_FLAG_ALL);
    if (status < 0){
        //errorMessage = "FSOpenDir failed: " + Utils::IntToHex(status);
        LOG_E("FSOpenDir failed (%d)", status);
        
        switch(status){
            case FS_STATUS_NOT_FOUND: //iosuhax?
                return false;
            case FS_STATUS_NOT_DIR:
                extraInfo = "This is not a directory";
                break;
            case FS_STATUS_CORRUPTED:
                extraInfo = "Directory is corrupted";
                break;
            default:
                break;
        }
        return true;
    }

    FSDirectoryEntry entry;
    int readRes;
    while((readRes = FSReadDir(cli, block, handle, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
        files.push_back(new FileButton(entry));
    }
    if (readRes != FS_STATUS_END)
        LOG_E("FSReadDir ended with unknown value (%d)", readRes);

    FSCloseDir(cli, block, handle, FS_ERROR_FLAG_ALL);

    FSStat stat;
    int statRes = FSGetStat(cli, block, _path.c_str(), &stat, FS_ERROR_FLAG_ALL);
    if (statRes < 0){
        LOG_E("FSGetStat error (%d)", statRes);
    }
    else
        setPermissionInfo(stat.mode);

    return true;
}

void iosuhax_ReadDir(std::string _path, std::string& errorMessage, std::string& extraInfo){
    LOG("Reading with IOSUHAX");
    Path::SetPathType(PathType::IOSUHAX);

    int dirHandle = 0;
    int res = IOSUHAX_FSA_OpenDir(fsaFd, _path.c_str(), &dirHandle);
    if (res < 0) {
        LOG_E("IOSUHAX_FSA_OpenDir failed (%d)", res);
        errorMessage = "IOSUHAX_FSA_OpenDir failed: " + Utils::IntToHex(res);
        switch(res){
            case (int32_t)0xFFFCFFE9:
                extraInfo = "Directory doesn't exists or drive couldn't be mounted";
                break;
            case (int32_t)0xFFFCFFD7:
                extraInfo = "This is not a directory";
                break;
            default:
                break;
        }
    }

    IOSUHAX_FSA_DirectoryEntry entry;
    int readRes = 0;
    while ((readRes = IOSUHAX_FSA_ReadDir(fsaFd, dirHandle, &entry)) == 0){
        files.push_back(new FileButton(entry));
    }
    if (readRes != (int32_t)IOSUHAX_END_OF_DIR) //IOSUHAX read end result
        LOG_E("IOSUHAX_FSA_ReadDir ended with unknown value (%d)", readRes);

    IOSUHAX_FSA_CloseDir(fsaFd, dirHandle);
    
    IOSUHAX_FSA_Stat stat;
    int statRes = IOSUHAX_FSA_GetStat(fsaFd, _path.c_str(), &stat);
    if (statRes < 0){
        LOG_E("IOSUHAX_FSA_GetStat error (%d)", statRes);
    }
    else
        setPermissionInfo(stat.flags);
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
        if (!fs_ReadDir(actualPath, errorMessage, extraInfo)){
            iosuhax_ReadDir(actualPath, errorMessage, extraInfo);
        }
        rewind_b->SetActive(true);
    }
    LOG("Total files: %d", files.size());
    if (files.size() <= 0){
        if (errorMessage == ""){
            directoryInfo1 = SDLH::GetText(arial40_font, black_col, "This directory is empty");
            directoryInfo2 = nullptr;
        }
        else{
            directoryInfo1 = SDLH::GetText(arial40_font, black_col, "Unable to open this directory");
            directoryInfo2 = SDLH::GetText(arial40_font, black_col, errorMessage.c_str());
            directoryInfoExtra = SDLH::GetText(arial40_font, black_col, extraInfo.c_str());
        }
    }
    
    checkedItems_tex = SDLH::GetTextf(arial50_font, black_col, "%d/%d", selectedItems, files.size());
    permissions_tex = SDLH::GetText(arial50_font, black_col, folderPerms.c_str());

    //Alphabetical sort
    std::sort(files.begin(), files.end(), Utils::AlphabeticalSort);
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
    if (directoryInfoExtra)
        SDL_DestroyTexture(directoryInfoExtra);
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

    FSFileHandle newitem_fs_fh = 0;
    int fs_openRes = FSOpenFile(cli, block, item.c_str(), "w", &newitem_fs_fh, FS_ERROR_FLAG_ALL);
    if (fs_openRes < 0){
        LOG_E("FSOpenFile for creating a file failed (%d) with file (%s)", fs_openRes, item.c_str());

        int newitem_iosuhax_fh = 0;
        int iosuhax_openRes = IOSUHAX_FSA_OpenFile(fsaFd, item.c_str(), "w", &newitem_iosuhax_fh);
        if (iosuhax_openRes < 0){
            LOG_E("IOSUHAX_FSA_OpenFile for creating a file failed (%d) with file (%s)", iosuhax_openRes, item.c_str());

            DialogHelper::SetDialog(new DialogDefault(
                    "Operation failed",
                    "Error trying to create a file: " + item,
                    "Error code: " + Utils::IntToHex(fs_openRes) + "-" + Utils::IntToHex(iosuhax_openRes),
                    DialogButtons::OK));
            DialogHelper::WaitForDialogResponse();
            return;
        }
        else
            IOSUHAX_FSA_CloseFile(fsaFd, newitem_iosuhax_fh);
    }
    else
        FSCloseFile(cli, block, newitem_fs_fh, FS_ERROR_FLAG_ALL);

    DialogHelper::SetDialog(new DialogDefault(
        "Operation ended",
        "Created file is at: " + item,
        "Click OK to continue",
        DialogButtons::OK));
    DialogHelper::WaitForDialogResponse();
}
void FilesystemHelper::CreateFileProccess(){
    std::thread(internal_createFile).detach();
}

void FilesystemHelper::CreateFolderProccess(){

}

void internal_copyClipboard(bool cut){
    clipboard.clear();

    std::string path = Path::GetPath();
    for (uint32_t i = 0; i < files.size(); i++){
        if (files[i]->IsSelected()){
            files[i]->SetSelection(false);
            clipboard.push_back(path + files[i]->GetName() + (files[i]->IsDirectory() ? "/" : ""));
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

bool pasteRecursive(std::string item, std::string where){
    DialogProgressbar* dp = (DialogProgressbar*)DialogHelper::GetDialog();
    if (dp->GetDialogResult() == DialogResult::CANCELLED_RES){
        DialogHelper::SetDialog(new DialogDefault(
            "Operation cancelled",
            "Copied files before cancelling: " + std::to_string(*info.copyCounter) +
            "\nRemaining files: " + std::to_string(info.copyTotal - *info.copyCounter),
            "Click OK to continue",
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return false;
    }

    std::string copy = clipboardPath + dir;
    std::string paste = Path::GetPath() + dir;

    LOG_E("Copying (%s) to (%s)", copy.c_str(), paste.c_str());

    DialogProgressbar* d = (DialogProgressbar*)DialogHelper::GetDialog();
    d->SetDescription("Copied " + std::to_string(*info.copyCounter) + " of " + std::to_string(info.copyTotal) + " files\nDelete items when copied: " + (info.deleteAtCopyEnd ? "yes" : "no"));
    d->SetFooter("<" + copy + ">");
    d->SetProgressBar((float)*info.copyCounter / (float)info.copyTotal);

    FSDirectoryHandle FS_dh;
    int openRes = FSOpenDir(cli, block, copy.c_str(), &FS_dh, FS_ERROR_FLAG_ALL);
    if (openRes >= 0){ //It's a directory, continue recursive
        int makeRes = FSMakeDir(cli, block, paste.c_str(), FS_ERROR_FLAG_ALL);
        if (makeRes < 0) {
            LOG_E("FSMakeDir failed (%d) with path (%s)", makeRes, paste.c_str());
        }

        (*info.copyCounter)++;

        int readRes;
        FSDirectoryEntry entry;
        while((readRes = FSReadDir(cli, block, FS_dh, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
            if (!pasteRecursive(dir + std::string(entry.name) + std::string((entry.info.flags & FS_STAT_DIRECTORY) ? "/" : ""), info))
                break;
        }

        FSCloseDir(cli, block, FS_dh, FS_ERROR_FLAG_ALL);
    }
    else if (openRes == -8) { //It's a file
        FSFileHandle copy_fh = 0;
        FSFileHandle paste_fh = 0;
        FSStat stat;

        //Open original file
        int openRes = FSOpenFile(cli, block, copy.c_str(), "rb", &copy_fh, FS_ERROR_FLAG_ALL);
        if (openRes < 0){
            LOG_E("FSOpenFile for copy failed (%d) with file (%s)", openRes, copy.c_str());

            DialogHelper::SetDialog(new DialogDefault(
                "Operation failed",
                "Error trying to open original file: " + copy + 
                "\n\nCopied files before failed: " + std::to_string(*info.copyCounter) + 
                "\nRemaining files: " + std::to_string(info.copyTotal - *info.copyCounter),
                "Error code: " + std::to_string(openRes) + ", FS: " + (info.copyIsFS ? "yes" : "no"),
                DialogButtons::OK));
            DialogHelper::WaitForDialogResponse();
            FSCloseFile(cli, block, copy_fh, FS_ERROR_FLAG_ALL);

            return false;
        }
        int statRes;
        if ((statRes = FSGetStatFile(cli, block, copy_fh, &stat, FS_ERROR_FLAG_ALL)) < 0) //Get file size
            LOG_E("FSGetStatFile for copy failed (%d) with file (%s)", statRes, copy.c_str());

        //Open copy file
        int openRes2 = FSOpenFile(cli, block, paste.c_str(), "wb", &paste_fh, FS_ERROR_FLAG_ALL);
        if (openRes2 < 0){
            LOG_E("FSOpenFile for paste failed (%d) with file (%s)", openRes2, paste.c_str());

            DialogHelper::SetDialog(new DialogDefault(
                "Operation failed",
                "Error trying to open pasted file: " + paste +
                "\n\nCopied files before failed: " + std::to_string(*info.copyCounter) +
                "\nRemaining files: " + std::to_string(info.copyTotal - *info.copyCounter),
                "Error code: " + std::to_string(openRes2) + ", FS: " + (info.pasteIsFS ? "yes" : "no"),
                DialogButtons::OK));
            DialogHelper::WaitForDialogResponse();
            FSCloseFile(cli, block, copy_fh, FS_ERROR_FLAG_ALL);
            FSCloseFile(cli, block, copy_fh, FS_ERROR_FLAG_ALL);

            return false;
        }

        //Start read/write
        uint32_t bytesToCopy = COPY_BUFFER_SIZE;
        uint32_t bytesCopied = 0;
        bool success = true;
        while(bytesCopied < stat.size){
            if (stat.size - bytesCopied < COPY_BUFFER_SIZE)
                bytesToCopy = stat.size - bytesCopied;

            LOG("Reading...");
            int readRes = FSReadFileWithPos(cli, block, (uint8_t*)copyBuffer, bytesToCopy, 1, bytesCopied, copy_fh, 0, FS_ERROR_FLAG_ALL);
            if (readRes < 0){
                LOG_E("FSReadFileWithPos failed (%d) with file (%s)", readRes, copy.c_str());

                DialogHelper::SetDialog(new DialogDefault(
                    "Operation failed",
                "Error while reading file: " + copy +
                "\n\nCopied files before failed: " + std::to_string(*info.copyCounter) +
                "\nRemaining files: " + std::to_string(info.copyTotal - *info.copyCounter),
                "Error code: " + std::to_string(readRes) + ", FS: " + (info.copyIsFS ? "yes" : "no"),
                DialogButtons::OK));
                DialogHelper::WaitForDialogResponse();
                success = false;
                break;
            }

            LOG("Writing...");
            int writeRes = FSWriteFileWithPos(cli, block, (uint8_t*)copyBuffer, bytesToCopy, 1, bytesCopied, paste_fh, 0, FS_ERROR_FLAG_ALL);
            if (writeRes < 0){
                LOG_E("FSWriteFileWithPos failed (%d) with file (%s)", writeRes, paste.c_str());

                DialogHelper::SetDialog(new DialogDefault(
                    "Operation failed",
                    "Error while writing to file: " + paste +
                    "\n\nCopied files before failed: " + std::to_string(*info.copyCounter) +
                    "\nRemaining files: " + std::to_string(info.copyTotal - *info.copyCounter),
                    "Error code: " + std::to_string(writeRes) + ", FS: " + (info.pasteIsFS ? "yes" : "no"),
                    DialogButtons::OK));
                DialogHelper::WaitForDialogResponse();
                success = false;
                break;
            }

            bytesCopied += bytesToCopy;
            LOG("Copied bytes %d/%d", bytesCopied, stat.size);
        }
        FSCloseFile(cli, block, paste_fh, FS_ERROR_FLAG_ALL);
        FSCloseFile(cli, block, copy_fh, FS_ERROR_FLAG_ALL);

        LOG("Copied! Success: %d", success);
        
        if (info.deleteAtCopyEnd){
            LOG("Deleting file...");
            int delRes = FSRemove(cli, block, copy.c_str(), FS_ERROR_FLAG_ALL);
            if (delRes < 0){
                LOG_E("FSRemove failed (%d) with path (%s)", delRes, copy.c_str());

                DialogHelper::SetDialog(new DialogDefault("Operation failed",
                    "Error deleting the item " + copy + "\nCopied files before failed: " + std::to_string(*info.copyCounter) + "\nRemaining files: " + std::to_string(info.copyTotal - *info.copyCounter),
                    "Error code: " + std::to_string(delRes) + ", FS: " + (info.copyIsFS ? "yes" : "no"),
                    DialogButtons::OK));
                DialogHelper::WaitForDialogResponse();
                return false;
            }
        }
        
        (*info.copyCounter)++;
        return success;
    }
    else{ //Unknown error
        LOG_E("FSOpenDir failed (%d) with path (%s)", openRes, copy.c_str());

        DialogHelper::SetDialog(new DialogDefault("Operation failed",
            "Error opening the item " + copy +
            "\n\nCopied files before failed: " + std::to_string(*info.copyCounter) +
            "\nRemaining files: " + std::to_string(info.copyTotal - *info.copyCounter),
            "Error code: " + std::to_string(openRes) + ", FS: " + (info.copyIsFS ? "yes" : "no"),
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return false;
    }
    return true;
}

void internal_paste(){
    DialogHelper::SetDialog(new DialogDefault(
        "Fetching files...",
        "Calculating number of files\n" +
        "Wait for the confirmation to start...",
        ""
        DialogButtons::NONE));

    int nfiles = 0, nfolders = 0;
    for (uint32_t i = 0; i < clipboard.size(); i++){
        getFilesRecursive(clipboard[i], &nfiles, &nfolders);
    }

    DialogHelper::SetDialog(new DialogDefault(
        "Confirm operation",
        "Found " + std::to_string(nfiles) + " files and " + std::to_string(nfolders) + " folders?" + 
        std::string("Do you want to ") deleteClipboardAtEnd ? "copy" + "cut" + " those items?\n" +
        "\nFrom: " + clipboardPath +
        "\nTo: " + Path::GetPath(),
        "Choose an option",
        DialogButtons::NO_YES));
    if (DialogHelper::WaitForDialogResponse() == DialogResult::NO_RES)
        return;

    int copiedFiles = 0;
    int copyTotal = nfiles + nfolders;

    DialogHelper::SetDialog(new DialogProgressbar(
        "Copying " + std::to_string(nfiles + nfolders) + " items...",
        "Copied 0 of " + std::to_string(info.copyTotal) + " files" + 
        "\nDelete items when copied: " + (info.deleteAtCopyEnd ? "yes" : "no"),
        "",
        true));

    bool success = true;
    for (uint32_t i = 0; i < clipboard.size(); i++){
        success = pasteRecursive(Utils::GetFilename(clipboard[i]), info);
        if (!success)
            break;
    }
    //Operation ended
    LOG("Copy ended!");
    FilesystemHelper::ReadPathDir();

    if (info.deleteAtCopyEnd){
        clipboard.clear();
        paste_b->SetActive(false);
    }

    if (success) {
        DialogHelper::SetDialog(new DialogDefault(
            "Operation ended",
            "Items copied: " + std::to_string(copiedFiles) + "/" + std::to_string(info.copyTotal),
            "Click OK to continue",
            DialogButtons::OK));
            
        DialogHelper::WaitForDialogResponse();
    }
}

void FilesystemHelper::Paste(){
    if (clipboard.size() <= 0)
        return;
    
    std::thread(internal_paste).detach();
}

typedef struct {
    int* deleteCounter;
    int deleteTotal;
    bool deleteIsFS;
} DeleteInfo;

bool deleteRecursive(std::string dir, DeleteInfo& info){ //Dir must be with path!!
    DialogProgressbar* d = (DialogProgressbar*)DialogHelper::GetDialog();
    if (d->GetDialogResult() == DialogResult::CANCELLED_RES){
        DialogHelper::SetDialog(new DialogDefault(
            "Operation cancelled",
            "Deleted files before cancelling: " + std::to_string(*info.deleteCounter) +
            "\nRemaining files: " + std::to_string(info.deleteTotal - *info.deleteCounter),
            "Click OK to continue",
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return false;
    }

    std::string del = Path::GetPath() + dir;

    d->SetDescription("Deleted " + std::to_string(*info.deleteCounter) + " of " + std::to_string(info.deleteTotal) + " files");
    d->SetFooter("<" + del + ">");
    d->SetProgressBar((float)*info.deleteCounter / (float)info.deleteTotal);

    FSDirectoryHandle FS_dh;
    int openRes = FSOpenDir(cli, block, del.c_str(), &FS_dh, FS_ERROR_FLAG_ALL);
    if (openRes >= 0){ //It's a directory, continue recursive

        int readRes;
        FSDirectoryEntry entry;
        bool success = true;
        while((readRes = FSReadDir(cli, block, FS_dh, &entry, FS_ERROR_FLAG_ALL)) == FS_STATUS_OK){
            success = deleteRecursive(dir + std::string(entry.name) + std::string((entry.info.flags & FS_STAT_DIRECTORY) ? "/" : ""), info);
            if (!success)
                break;
        }
        FSCloseDir(cli, block, FS_dh, FS_ERROR_FLAG_ALL);

        if (!success)
            return false;
    }
    else if (openRes < 0 && openRes != -8){ //Unknown error
        LOG_E("FSOpenDir failed (%d) with path (%s)", openRes, dir.c_str());
        
        DialogHelper::SetDialog(new DialogDefault(
            "Operation failed",
            "Error opening the item " + del +
            "\nDeleted files before failed: " + std::to_string(*info.deleteCounter) +
            "\nRemaining files: " + std::to_string(info.deleteTotal - *info.deleteCounter),
            "Error code: " + std::to_string(openRes) + ", FS: " + (info.deleteIsFS ? "yes" : "no"),
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return false;
    }

    int delRes = FSRemove(cli, block, dir.c_str(), FS_ERROR_FLAG_ALL);
    if (delRes < 0){
        LOG_E("FSRemove failed (%d) with path (%s)", delRes, dir.c_str());

        DialogHelper::SetDialog(new DialogDefault(
            "Operation failed",
            "Error deleting the item " + del +
            "\nDeleted files before failed: " + std::to_string(*info.deleteCounter) +
            "\nRemaining files: " + std::to_string(info.deleteTotal - *info.deleteCounter),
            "Error code: " + std::to_string(delRes) + ", FS: " + (info.deleteIsFS ? "yes" : "no"),
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
        return false;
    }
    (*info.deleteCounter)++;

    return true;
}

void deleteProccess(){
    DialogHelper::SetDialog(new DialogDefault(
        "Deleting files...",
        "Calculating number of files",
        "Waiting for the operation to start...",
        DialogButtons::NONE));

    int nfiles = 0, nfolders = 0;
    for (uint32_t i = 0; i < files.size(); i++){
        if (files[i]->IsSelected()){
            if (!getFilesRecursive(Path::GetPath() + files[i]->GetName(), &nfiles, &nfolders))
            return;
        }
    }

    DialogHelper::SetDialog(new DialogDefault(
        "Confirm operation",
        "Are you sure do you want to delete " + std::to_string(nfiles) + " files and " + std::to_string(nfolders) + " folders?",
        "Choose an option",
        DialogButtons::NO_YES));
    DialogResult result = DialogHelper::WaitForDialogResponse();
    if (result == DialogResult::NO_RES)
        return;

    int deleteCounter = 0;

    DeleteInfo info;
    info.deleteCounter = &deleteCounter;
    info.deleteTotal = nfiles + nfolders;
    info.deleteIsFS = Utils::StartsWith(Path::GetPath(), "/vol/external01/");

    DialogHelper::SetDialog(new DialogProgressbar(
        "Deleting files...",
        "Deleted 0 of " + std::to_string(info.deleteTotal) + " files",
        "",
        true));

    bool success = true;
    for (uint32_t i = 0; i < files.size(); i++){
        if (files[i]->IsSelected()){
            success = deleteRecursive(files[i]->GetName(), info);
            if (!success)
                break;
        }
    }
    LOG("Delete ended!");
    FilesystemHelper::ReadPathDir();

    if (success) {
        DialogHelper::SetDialog(new DialogDefault(
            "Operation ended",
            "Items deleted: " + std::to_string(deleteCounter),
            "Click OK to continue",
            DialogButtons::OK));
        DialogHelper::WaitForDialogResponse();
    }
}

void FilesystemHelper::Delete(){
    std::thread(deleteProccess).detach();
}

void FilesystemHelper::Rename(){
    
}