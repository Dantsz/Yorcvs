cmake_minimum_required(VERSION 3.13)
project(Yorcvs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS ${CMAKE_SYSTEM_NAME})
#set SDL for windows
if(WIN32)
    set(SDL2MAIN_LIBRARY "C:/SDL2/lib/x64/SDL2main.lib")
    set(SDL2_INCLUDE_DIR "C:/SDL2/include")
    set(SDL2_LIBRARY "C:/SDL2/lib/x64/SDL2.lib")
    set(SDL2_IMAGE_INCLUDE_DIR "C:/SDL2_image/include")
    set(SDL2_IMAGE_LIBRARY "C:/SDL2_image/lib/x64/SDL2_image.lib")
    set(SDL2_TTF_INCLUDE_DIR "C:/SDL2_ttf/include")
    set(SDL2_TTF_LIBRARY "C:/SDL2_ttf/lib/x64/SDL2_ttf.lib")
endif()
 
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)



find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

set(YorcvsALLFILES "src/Yorcvs.cpp"
                
                  "src/assetmanager/assetmanager.h"

                  "src/common/types.h"
                  "src/common/log.h"
                 

                  "src/render/render.h"
                  "src/render/renderSDL2.h")

add_executable(Yorcvs ${YorcvsALLFILES})

target_link_libraries(${PROJECT_NAME} SDL2::Main SDL2::Image SDL2::TTF)



target_compile_options(Yorcvs PRIVATE
$<$<CXX_COMPILER_ID:MSVC>:/Wall /fsanitize=address /Zi >
$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op>
$<$<CXX_COMPILER_ID:Clang>:-Weverything -Wshadow -Wformat=2 -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wpedantic>
)