# DGEngine

![DGEngine Logo](https://github.com/dimecoin/dge/blob/master/res/images/dge_lbig.bmp "DGEngine Logo")

A "modern" **D**OS **G**ame **E**ngine written in C that is fast, flexiable and has clean API.

## Screen shots

![screenshot_3](https://github.com/dimecoin/dge/blob/master/screenshots/screenshot_3.png "screen_shot3")
![screenshot_4](https://github.com/dimecoin/dge/blob/master/screenshots/screenshot_4.png "screen_shot4")
![screenshot_5](https://github.com/dimecoin/dge/blob/master/screenshots/screenshot_5.png "screen_shot5")

More screenshots in [Screen Shots Directory](https://github.com/dimecoin/dge/tree/master/screenshots)

## Feature List

* Runs on most DOS versions and emulators.  (see Compatiablity below).  
* Compiles cleanly in most DOS compilers.  (see Compatiablity below).
* No assembly required, easy to use C API that abstracts BIOS/System calls, etc.
* mode 13h: VGA 320x200 @ 256 colors  (ie, Classic "Chunky Graphics")
* Rendering systems: Direct BIOS writes, Memory Map and Double Buffer.  Can easily create "retro" effects using BIOS/MEMMAP or get smooth graphics from DOUBLEBUFF.
* Primative shapes (points, lines, rectangles, polys, circles and fill/color modes).
* FPS/Delta counters.
* PC Speaker sounds.
* Text mode fonts for debugging.

**More features to come!**
Please see [todo.txt](https://github.com/dimecoin/dge/blob/master/todo.txt) for details.


## Compatiablity

Operating Systems:
* MS DOS
* FreeDOS
* DOS Box
* Probably many others!

Compilers:

* DJGpp 6.3.0 (cross compile and native)
* OpenWatcom 1.9 (cross compile and native)

CPUs
* 486
* Pentium and above!
* maybe others!

## Usage

Please see [wiki pages](https://github.com/dimecoin/dge/wiki) for details.



## Credits

Lots of code is  taken from Dave Brackken's awesome [VGA tutorial](http://www.brackeen.com/vga/) .  This game engine won't have been possiable without this great resource.

Logo font is from [DOS VGA 437](http://www.dafont.com/perfect-dos-vga-437.font)


## License

**Rendering Code**

A lot of the rendering code is taken from Dave Breckeen.  His license is as follows:
* From source files: _"Please feel free to copy this source code."_
* From private email: _"Feel free to do whatever you want with it, no need for credit. Consider it public domain."_
* His tutorials are marked as [CC BY-SA 2.5](https://creativecommons.org/licenses/by-sa/2.5/)
* Please check out this site at : http://www.brackeen.com/vga/

**DOS/32 Advanced**

_"This product uses DOS/32 Advanced DOS Extender technology."_

**libkb**

This product uses libkb Version 1.01, 03 Jun 1998.  Please see [kb.h](https://github.com/dimecoin/dge/blob/master/src/libkb/kb.h) for more details.  (A very small modification was made to kb.h and \_handler.h to get OpenWatcom 1.9 to compile.)

**Runtimes**

Please see runtimes directory for 3rd party licenses for DJGPP and OpenWatcom (DOS32/A).

**DOSEngine license :** 

* MIT License (see license file)
* We ask that you include "Made with DGEngine" in splash screen or credits, but this is 100% optional and NOT required.

(**TLDR:** do whatever you want with it, no warranty)




