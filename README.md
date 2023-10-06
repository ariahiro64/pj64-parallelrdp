# pj64-parallelrdp

Implementation of Themaister's Parallel-RDP simulator for PJ64.
Vulkan is used as the GPU compute backend, and OpenGL 3.3 is 
used for the blitting of image buffers captured from the backend.

Made after a few days of messing around with GCC and some misc. sources.

To compile using MSYS2 
1) pacman -S mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-make mingw-w64-i686-dlfcn mingw-w64-i686-SDL2 git 
2) clone the repo
3) pull the submodules with "git submodule update --init --recursive"
4) use "make all V=1 OPTFLAGS="-static -O2" -j6"

If you dont have a git.h you can make one and "#define GIT_HEAD_SHA1"'
downstream uses cmake but i could never get that to work.

For best results: use Parallel RSP plugin. Zilmar's RSP has several LLE GFX bugs.
