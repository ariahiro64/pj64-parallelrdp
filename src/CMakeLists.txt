# Compiler flags mostly copied from parallel-rdp upstream CMakeLists.txt.
# MSVC: Added /O2, /fp:fast for significant performance increase, /MT for static linking against runtime library.
if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
    set(PJ64_PARALLEL_RDP_CXX_FLAGS -Wall -Wextra -Wno-missing-field-initializers -Wno-empty-body -ffast-math -Wno-unused-parameter -pthread -fwrapv)
elseif (MSVC)
    set(PJ64_PARALLEL_RDP_CXX_FLAGS /O2 /fp:fast /MT /D_CRT_SECURE_NO_WARNINGS /wd4267 /wd4244 /wd4309 /wd4005 /MP /DNOMINMAX)
endif()

# Include CMakeLists.txt for parallel-rdp-standalone source.
include(${CMAKE_CURRENT_SOURCE_DIR}/parallel-rdp-standalone.cmake)

set(src_files
    parallel_imp.cpp
    config_gui.c
    config.c
    gfx_1.3.c
    gl_core_3_3.c
    glguts.c
    ini.c
    wgl_ext.c
)

set(header_files
    config_gui_resources.h
    config_gui.h
    config.h
    gfx_1.3.h
    gfx_m64p.h
    gfxstructdefs.h
    gl_core_3_3.h
    glguts.h
    ini.h
    parallel_imp.h
    wgl_ext.h
)

set(res_files
    config_gui_resources.rc
)

# Windows *.lib file dependencies.
set(libs comctl32 gdi32 opengl32)

add_library(pj64-parallel-rdp SHARED ${src_files} ${res_files})
target_link_libraries(pj64-parallel-rdp PUBLIC parallel-rdp-standalone ${libs})
target_compile_options(pj64-parallel-rdp PRIVATE ${PJ64_PARALLEL_RDP_CXX_FLAGS})
target_include_directories(pj64-parallel-rdp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/parallel-rdp)
target_compile_definitions(pj64-parallel-rdp PRIVATE NOMINMAX)
set_target_properties(pj64-parallel-rdp PROPERTIES PREFIX "" SUFFIX ".dll")
