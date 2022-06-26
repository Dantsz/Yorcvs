cmake_minimum_required(VERSION 3.13)
project(Yorcvs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(YORCVS_EMSCRIPTEN_PRELOAD_ASSETS_FOLDER "Whether to include the assets folder in the emscripten build or not"  OFF)

include(FetchContent)
include(ExternalData)


message(STATUS "BUILDING YORCVS ON " ${CMAKE_SYSTEM_NAME})
set(YorcvsALLFILES  "src/Yorcvs.cpp"
                    "src/Yorcvs.h"
                    
                    "src/ui/debuginfo.h"

                    "src/game/components.h"
                    "src/game/systems.h"

                    "src/game/systems/animation.h"
                    "src/game/systems/behaviour.h"
                    "src/game/systems/collision.h"
                    "src/game/systems/combat.h"
                    "src/game/systems/health.h"
                    "src/game/systems/playercontrol.h"
                    "src/game/systems/spritesystem.h"
                    "src/game/systems/staminasystem.h"
                    "src/game/systems/velocity.h"
                   
                  
                    "src/common/assetmanager.h"
                    "src/common/types.h"
                    "src/common/ecs.h"

                    "src/common/utilities.h"
                    "src/common/utilities/timer.h"
                    "src/common/utilities/ulamspiral.h"
                    "src/common/utilities/log.h"


                    "src/engine/luaEngine.h"
                    "src/engine/map.h"

                    "src/engine/window/window.h"
                    "src/engine/window/eventhandler.h"
                    "src/engine/window/windowsdl2"
                    "src/engine/window/eventhandlersdl2.h"
                    )


add_executable(Yorcvs ${YorcvsALLFILES})

#COMPILE FLAGS
target_compile_options(Yorcvs PRIVATE
$<$<CXX_COMPILER_ID:MSVC>:/W4 /Zi /DEBUG /bigobj>
$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -std=c++2a>
$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wno-c++98-compat-pedantic -Wno-c++98-compat>
)


if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
    SET(CMAKE_CXX_FLAGS "-std=c++2a -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=\"[\"\"png\"\"]\"  -s USE_SDL_TTF=2  -s ALLOW_MEMORY_GROWTH=1 -s NO_DISABLE_EXCEPTION_CATCHING -o build/Yorcvs.html" ) #commented because emscriptem requires the assets folder to be in special place and it's not necesary for compilation for CI
  if(YORCVS_EMSCRIPTEN_PRELOAD_ASSETS_FOLDER)
    message(STATUS "Assets folder will be preloaded")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS  "--preload-file ./assets")
  endif()
else() 

#link SDL
message(STATUS "LINKING SDL2")
if(WIN32)
  #set SDL for windows
  find_package(SDL2 CONFIG REQUIRED)
  find_package(SDL2-image CONFIG REQUIRED)
  find_package(SDL2-ttf CONFIG REQUIRED)
  target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_image SDL2::SDL2_ttf)
  set(SDL2lib SDL2::SDL2)
else()
  SET(LUA_CMAKE_BUILD_SHARED ON CACHE BOOL "Build as shared library" FORCE)
  #download findSDL2 and use it
  FetchContent_Declare(amifindSDL2 GIT_REPOSITORY https://github.com/aminosbh/sdl2-cmake-modules.git GIT_TAG "master")
  FetchContent_GetProperties(amifindSDL2)
  if(NOT amifindSDL2_POPULATED)
    FetchContent_Populate(amifindSDL2)
  endif()
  list(APPEND CMAKE_MODULE_PATH ${amifindsdl2_SOURCE_DIR})
  find_package(SDL2 REQUIRED)
  find_package(SDL2_image REQUIRED)
  find_package(SDL2_ttf REQUIRED)
  target_link_libraries(${PROJECT_NAME} SDL2::Main SDL2::Image SDL2::TTF)
  set(SDL2lib SDL2::Main)
endif()

  
endif()



#INSTALL JSON LIBRARY
message(STATUS "LINKING  JSON LIBRARY")
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)
message(STATUS "JSON SOURCE DIR : " ${json_SOURCE_DIR})
message(STATUS "JSONBUILD DIR : " ${json_BUILD_DIR} )
target_include_directories(${PROJECT_NAME} PUBLIC  nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

#INSTALL TMX LIBRARY
message(STATUS "BUILDING TMX LIBRARY")
FetchContent_Declare(libtmxlite
 GIT_REPOSITORY https://github.com/Dantsz/tmxlite.git
 GIT_TAG "main" 
)
FetchContent_GetProperties(libtmxlite)

if(NOT libtmxlite_POPULATED)
 FetchContent_Populate(libtmxlite)
 set(libtmxlite_SOURCE_DIR  ${libtmxlite_SOURCE_DIR}/tmxlite)
 set(TMXLITE_STATIC_LIB true)
 add_subdirectory(${libtmxlite_SOURCE_DIR} ${libtmxlite_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
target_link_libraries(${PROJECT_NAME}  tmxlite)

#INSTALL LUA Interpreter
FetchContent_Declare(lua
 GIT_REPOSITORY https://github.com/Dantsz/lua-cmake.git
 GIT_TAG "master" 
)
FetchContent_GetProperties(lua)
if (NOT lua_POPULATED)
   FetchContent_Populate(lua)
   add_subdirectory(${lua_SOURCE_DIR} ${lua_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
target_link_libraries(${PROJECT_NAME} lua::lib)
target_include_directories(${PROJECT_NAME} PUBLIC lua::header)
#INSTALL SOL2
FetchContent_Declare(sol2
 GIT_REPOSITORY https://github.com/ThePhD/sol2.git
 GIT_TAG "main" 
)
FetchContent_GetProperties(sol2)
if (NOT sol2_POPULATED)
   FetchContent_Populate(sol2)
  target_include_directories(${PROJECT_NAME} PUBLIC ${sol2_SOURCE_DIR}/include)
endif()
#INSTALL IMGUI BASE
FetchContent_Declare(imgui
 GIT_REPOSITORY https://github.com/ocornut/imgui.git
 GIT_TAG "master" 
)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
   FetchContent_Populate(imgui)
  target_include_directories(${PROJECT_NAME} PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
  message(STATUS "imgui :" ${imgui_SOURCE_DIR})
  add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
  target_link_libraries(imgui  ${SDL2lib})
  target_link_libraries(${PROJECT_NAME} imgui)
  endif()
#INSTALL SDL2 BACKEND
FetchContent_Declare(imguiSDL2
 GIT_REPOSITORY https://github.com/Tyyppi77/imgui_sdl.git
 GIT_TAG "master" 
)
FetchContent_GetProperties(imguiSDL2)
if (NOT imguiSDL2_POPULATED)
  FetchContent_Populate(imguiSDL2)
  target_include_directories(${PROJECT_NAME} PUBLIC ${imguisdl2_SOURCE_DIR} )
  message(STATUS "imguiSDL2 :" ${imguisdl2_SOURCE_DIR})
  add_library(imgui-SDL2 STATIC ${imguisdl2_SOURCE_DIR}/imgui_sdl.cpp )
  target_link_libraries(imgui-SDL2 ${SDL2lib})
  target_link_libraries(imgui-SDL2  imgui)
  target_include_directories(imgui-SDL2 PUBLIC ${imgui_SOURCE_DIR})
  target_link_libraries(${PROJECT_NAME} imgui-SDL2)
  
endif()
