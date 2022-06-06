cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

project(pj64-parallel-rdp LANGUAGES C CXX)

# Generate git info into header file.
set(PRE_CONFIGURE_FILE "src/git.h.in")
set(POST_CONFIGURE_FILE "src/git.h")
include(cmake-git-version-tracking/git_watcher.cmake)

add_subdirectory(src)
