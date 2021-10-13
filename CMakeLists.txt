cmake_minimum_required(VERSION 3.13)
project(Yorcvs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS ${CMAKE_SYSTEM_NAME})


set(YorcvsALLFILES  "src/Yorcvs.cpp"
                    "src/Yorcvs.h"
                    "src/systems.h"
                  
                    "src/common/assetmanager.h"

                    "src/common/types.h"
                    "src/common/utilities.h"
                    "src/common/ecs.h"
                    
                    "src/components.h"

                    "src/common/window.h"
                    "src/windowSDL2.h"
                    )


add_executable(Yorcvs ${YorcvsALLFILES})


target_compile_options(Yorcvs PRIVATE
$<$<CXX_COMPILER_ID:MSVC>:/W4 /fsanitize=address /Zi /DEBUG>
$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -std=c++2a>
$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wno-c++98-compat-pedantic -Wno-c++98-compat>
)

#link SDL
if(EMSCRIPTEN)
set(CMAKE_EXECUTABLE_SUFFIX ".html")
SET(CMAKE_CXX_FLAGS "-std=c++2a -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=\"[\"\"png\"\"]\"  -s USE_SDL_TTF=2  -s ALLOW_MEMORY_GROWTH=1 -o build/Yorcvs.html") #--preload-file ./assets" )
else()
#set SDL for windows
if(WIN32)
    set(SDL2MAIN_LIBRARY "C:/Libs/SDL2/lib/x64/SDL2main.lib")
    set(SDL2_INCLUDE_DIR "C:/Libs/SDL2/include")
    set(SDL2_LIBRARY "C:/Libs/SDL2/lib/x64/SDL2.lib")
    set(SDL2_IMAGE_INCLUDE_DIR "C:/Libs/SDL2_image/include")
    set(SDL2_IMAGE_LIBRARY "C:/Libs/SDL2_image/lib/x64/SDL2_image.lib")
    set(SDL2_TTF_INCLUDE_DIR "C:/Libs/SDL2_ttf/include")
    set(SDL2_TTF_LIBRARY "C:/Libs/SDL2_ttf/lib/x64/SDL2_ttf.lib")
endif()
 
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
target_link_libraries(${PROJECT_NAME} SDL2::Main SDL2::Image SDL2::TTF)
endif()

include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(${PROJECT_NAME}  nlohmann_json::nlohmann_json)


FetchContent_Declare(libtmxlite
 GIT_REPOSITORY https://github.com/fallahn/tmxlite.git
)
FetchContent_GetProperties(libtmxlite)
if(NOT libtmxlite_POPULATED)
 FetchContent_Populate(libtmxlite)
 set(libtmxlite_SOURCE_DIR  ${libtmxlite_SOURCE_DIR}/tmxlite)
 set(TMXLITE_STATIC_LIB true)
 add_subdirectory(${libtmxlite_SOURCE_DIR} ${libtmxlite_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
target_link_libraries(${PROJECT_NAME}  tmxlite)

