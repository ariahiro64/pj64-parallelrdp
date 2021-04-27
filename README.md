# pj64-parallelrdp

Implementation of Themaister's Parallel-RDP simulator for PJ64.
Vulkan is used as the GPU compute backend, and OpenGL 3.3 is 
used for the blitting of image buffers captured from the backend.

Made after a few days of messing around with GCC and some misc. sources.

To compile using MSYS2 
1) Clone/fork the Github repo only.
2) use "make all platform=win"

For best results: use cxd4's RSP plugin. Zilmar's RSP has several LLE GFX bugs.