# Volcako

## Note: this project has not been tested with Volca FM 2 (released 2022)!

This is a realtime editor for the Korg Volca FM. It exposes all of Volca's parameters in one ~~ugly~~ functional screen. In addition it can load and save entire 32 voice banks in .SYX format. If you have a midi input device connected, the keypresses will be routed to Volca, optionally with note velocity!

![Screenshot](docs/screenshot.png)

How to use:
- Select your midi out device from Settings. The Volca listens to the sysex messages regardless of the channel it's set to.
- Selecting a patch or editing the patch parameters sends it to the currently selected slot on the Volca. You still have to Save the patch manually on the device (Edit->Write, hardware limitation)
- File->Load .SYX can be used to load an entire DX7 patch bank.
- File->Send All Patches will send and save all of the 32 patches to the device.

Some nice to know things:
- Right click on an envelope to copy/paste the values
- Control-click on an input field to type in the value
- The last edited present is auto-saved/loaded into lastpreset.txt

Windows release can be found at: https://github.com/Luomu/volcako/releases/download/r1/volcako-r1.zip

## Release notes

```
R2 (Unreleased)
[New] Allow copying and pasting presets (right-click preset browser)
[New] Save the entire 32-preset bank on shutdown

R1 (November 2019)
Initial release
```

## Building

Only a Windows build has been made so far. However, all the libraries used are cross platform so a Mac/Linux port should be a breeze. You have two choices: [CMake](https://cmake.org/) and [Genie](https://github.com/bkaradzic/genie).

Using CMake is simple with Visual Studio 2022 (Community): Just open the folder and it shoudl run the CMake configuration. Then select Build->Build all.

With genie you need to generate the makefile/solution for the IDE of your choice. To create .sln for VS 2019, run this in the root directory:

```genie vs2019```

Precompiled Genie Windows executable: https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe

## Libraries used
dear imgui - https://github.com/ocornut/imgui

glfw - www.glfw.org/

inipp - https://github.com/mcmtroffaes/inipp

rtmidi - www.music.mcgill.ca/~gary/rtmidi/

tinyfiledialogs - https://sourceforge.net/projects/tinyfiledialogs/

## References
Korg Volca FM Midi implementation - www.korg.com/us/support/download/manual/0/558/2963/

## Thanks to
Dexed - https://asb2m10.github.io/dexed/ - DX7 VST and editor, used to verify Sysex compatibility
