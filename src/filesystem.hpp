#include "main.hpp"
#include "gui/filebutton.hpp"

typedef uint64_t FSTime;
extern std::vector<FileButton*> files;
extern std::vector<std::string> clipboard;

namespace Filesystem{
    bool Init();
    void Shutdown();

    void FSTimeToCalendarTime(FSTime time, OSCalendarTime* ct);
    void AddPreviousPath(std::string where);
    void TryToMount(std::string dev, std::string vol);

    void ReadDir();
    void ClearDir();

    void ChangeDir(std::string dir);
    void Rewind();

    void CreateFile();
    void CreateFolder();

    void Copy(bool _deleteClipboardAtEnd);
    void Paste();
    
    void Delete();
    void Rename();
}