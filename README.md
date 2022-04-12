# Yorcvs 
[![Linux Build](https://github.com/Dantsz/Yorcvs/actions/workflows/cmake.yml/badge.svg)](https://github.com/Dantsz/Yorcvs/actions/workflows/cmake.yml)
[![Emscripten Build](https://github.com/Dantsz/Yorcvs/actions/workflows/EmscriptenBuild.yml/badge.svg)](https://github.com/Dantsz/Yorcvs/actions/workflows/EmscriptenBuild.yml)
[![Windows Build](https://github.com/Dantsz/Yorcvs/actions/workflows/windows.yml/badge.svg)](https://github.com/Dantsz/Yorcvs/actions/workflows/windows.yml)
[![Ctests](https://github.com/Dantsz/Yorcvs/actions/workflows/CTests.yml/badge.svg)](https://github.com/Dantsz/Yorcvs/actions/workflows/CTests.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/dantsz/yorcvs/badge?s=c9aff80c39c79e9d970cb65e1572ff8096881f19)](https://www.codefactor.io/repository/github/dantsz/yorcvs)


C++ RPG/sandbox game project
[[demo]](https://dantsz.github.io/resources/Yorcvs.html)
![image](https://user-images.githubusercontent.com/19265585/160299653-ce730e2c-0307-4b0e-9bec-c16e8f078cde.png)
## Features
* ECS based design
* Tmx map format
* Lua scripting

## Dependencies
|Dependency|Minimum Version|
--- |---
|[SDL2](https://www.libsdl.org/download-2.0.php)|2.0.10|
|[SDL2_image](https://www.libsdl.org/projects/SDL_image/)|2.0.10|
|[SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)|2.0.10|
|[nlohmann/json](https://github.com/nlohmann/json)|
|[tmxlite](https://github.com/fallahn/tmxlite)|
|[lua-cmake](https://github.com/lubgr/lua-cmake)|
|[sol2](https://sol2.readthedocs.io/en/latest/)|v3.2.2

## Compiling
Cmake (at least version 3.13) is required for all platforms
### Linux
1. Install SDL2 :
   ```
   sudo apt-get install libsdl2-dev
   sudo apt-get install libsdl2-image-dev
   sudo apt-get install libsdl2-ttf-dev
   ```
2. Configure and build with cmake
   ```
   cmake -B
   cmake --build
   ```
### Windows
1. Install and setup vcpkg from https://vcpkg.io/en/index.html
2. Configure and build the cmake project and specify the vcpkg toolchain file
   ```
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE= <path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake
   cmake --build
   ```
### Emscripten
1. install and configure the Emscripten SDK from https://emscripten.org/
2. configure cmake with emcmake and set the option to preload the assets folder
   ```
      emcmake cmake -B build -DYORCVS_EMSCRIPTEN_PRELOAD_ASSETS_FOLDER=ON
   ```
3. copy the assets folder into the build folder
4. build the project
   ```
      cmake --build build
   ```
5. To the the build run 
   ```
   emrun /build/Yorcvs.html
   ```


   

