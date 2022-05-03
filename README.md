
# Hellium File Explorer

This is a WIP file explorer for the WiiU made with [wut](https://github.com/devkitPro/wut), [libiosuhax](https://github.com/yawut/libiosuhax) and [SDL2](https://github.com/yawut/SDL). It allows you to navigate over console's internal memory, discs, SD card, USB, and more!

The project is still in development so, although it compiles and runs, is not ready for general use and I want to add some features as well as fix some bugs before making an official release. You can compile and test it yourself (Check-out the [Building](#Building) section)

<!--![Logo](https://user-images.githubusercontent.com/24766260/166570126-04a03e89-5784-4fa8-a928-0310bbaf16c3.png)-->


## Features

- Travel through folders from SD, USB, etc...
- Mount and read/edit/create files on:
    * SD Card
    * Plugged-in USBs
    * MLC (WiiU's internal memory)
    * SLC (WiiU's system partition)
    * SLCCMPT (vWii NAND)
    * RAMDISK
    * ODD01/ODD02/ODD03/ODD04 (Optical disc)
- Show metadata such as file type, size, creation date...
- Multiple selection of items
- Copy/Paste files and folders between SD card and internal memory or USBs
- Searching custom paths with software keyboard
- Go back/forward history of paths
- Fancy background bubbles
## Installation

The project in development so no official releases are providen yet

Check-out for new releases at [Hellium-File-Explorer/Releases](https://github.com/Pokes303/Hellium-File-Explorer/releases) tab or at [HBAppStore](https://apps.fortheusers.org/wiiu) page
## Building

This section is made to compile the project in a Linux-based machine. To build it on another SO like Windows check [WSL](https://docs.microsoft.com/en-us/windows/wsl/install) (Recommended) or [MSYS2](https://www.msys2.org/).

Follow the guides to install these individual packages:
- [wut](https://github.com/devkitPro/wut)
- [libiosuhax](https://github.com/yawut/libiosuhax) (wut static library)
- [SDL2](https://github.com/yawut/SDL) (wiiu version)

I recommended to clone and build the latest wut because some functions are from non-released changes that aren't available for [1.0.0-beta12](https://github.com/devkitPro/wut/releases/) (Latest version on the day I write this)

After that, clone and build the project with:

```bash
  git clone https://github.com/Pokes303/Hellium-File-Explorer.git
  cd Hellium-File-Explorer
  make
```

Then `Hellium-File-Explorer.wuhb` file should have been generated. You can run it with any WiiU cfw ([Tiramisu](https://tiramisu.foryour.cafe/) environment is recommended) through the [Homebrew Launcher](https://github.com/dimok789/homebrew_launcher) or install the `Hellium-File-Explorer.rpx` as a channel into Home Menu with a tool such as [NUSPackerPlus](https://github.com/Pokes303/NUSPackerPlus)
## TODO

- ~~Create files/folders~~
- ~~Fix copy/pastes~~
- ~~Filesystem class rework~~
- ~~Add IOSUHAX handling if FS fails~~
- Fix dialog random crashes
- Implement ProcUI instead of WHBProc
- File info menu
- File text/hex menu
- Fix Mix_CloseAudio softlock when shutting down the console
- Fix text memory leaks (Caused by SDL2_ttf. Impossible?)
- Handling exceptions
- Settings menu to edit GMT, save file, color and more
<!--> # ## Screenshots

![App Screenshot](https://via.placeholder.com/468x300?text=App+Screenshot+Here)-->


## Authors

- [Pokes303](https://github.com/Pokes303) - main developer
- [devkitPRO contributors](https://github.com/devkitPro/wut/graphs/contributors) - wut
- [yawut team](https://github.com/yawut) - libiosuhax fork and SDL2 port
- [SDL2 authors](https://github.com/libsdl-org/SDL) - project is made in pure SDL

