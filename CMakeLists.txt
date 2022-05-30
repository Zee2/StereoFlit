# cmake for flutterkit-native embedder.
# Windows-only right now... 

cmake_minimum_required(VERSION 3.11)
project(flutterkit_embedder VERSION "0.1.0" LANGUAGES CXX C)

# Fancy new cpp, please
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20" )
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Yoink StereoKit from public git repo
include(FetchContent)
set(SK_BUILD_TESTS       OFF CACHE INTERNAL "")
set(SK_BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
FetchContent_Declare(
  StereoKitC
  GIT_REPOSITORY https://github.com/StereoKit/StereoKit.git
  GIT_TAG        v0.3.5
)
FetchContent_MakeAvailable(StereoKitC)

# Grab a snap of the Flutter engine + embedder headers.
# Make sure the hash matches your flutter engine.version file.
# Why? Who knows... some versions seem compatible, some crash and complain
# about binary formats... wild!
# Todo: make **not** windows-x64...
FetchContent_Declare(
  Flutter
  URL https://storage.googleapis.com/flutter_infra_release/flutter/caaafc5604ee9172293eb84a381be6aadd660317/windows-x64/windows-x64-embedder.zip
)
FetchContent_MakeAvailable(Flutter)

# We force OpenGL here because Flutter doesn't speak DirectX
add_definitions("-DSKG_FORCE_OPENGL")

# Add source files to our target executable
add_executable( flutterkit_embedder
  src/main.cpp
)

# Link to dependencies
target_link_libraries( flutterkit_embedder
  PRIVATE StereoKitC
  PRIVATE ${flutter_SOURCE_DIR}/flutter_engine.dll.lib
)

# Include the embedder header.
target_include_directories(flutterkit_embedder PRIVATE ${flutter_SOURCE_DIR})

# Need to copy our icudtl.dat and flutter_engine.dll into our destination dir.
add_custom_command(
        TARGET flutterkit_embedder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/external/icudtl.dat ${flutter_SOURCE_DIR}/flutter_engine.dll
                $<TARGET_FILE_DIR:flutterkit_embedder>)