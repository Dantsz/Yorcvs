cmake_minimum_required(VERSION 3.13)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
enable_testing()

option(YORCVS_EMSCRIPTEN_PRELOAD_ASSETS_FOLDER "Whether to include the assets folder in the emscripten build or not"  OFF)
#build lua as shared for windows
if(WIN32)
SET(LUA_CMAKE_BUILD_SHARED FALSE CACHE BOOL "Build as shared library" FORCE)
else()
SET(LUA_CMAKE_BUILD_SHARED TRUE CACHE BOOL "Build as shared library" FORCE)
endif()

function(SET_DEFAULT_YORCVS_USE_VCPKG)
    if(WIN32)
        set(DEFAULT_YORCVS_USE_VCPKG TRUE PARENT_SCOPE)
    elseif(UNIX)
        set(DEFAULT_YORCVS_USE_VCPKG FALSE PARENT_SCOPE)
    else()
        set(DEFAULT_YORCVS_USE_VCPKG FALSE PARENT_SCOPE)
    endif()
endfunction()

SET_DEFAULT_YORCVS_USE_VCPKG()
set(YORCVS_USE_VCPKG ${DEFAULT_YORCVS_USE_VCPKG} CACHE BOOL "Use the name of the libraries as they are in vcpkg")
message(STATUS "USING VCPKG: ${YORCVS_USE_VCPKG}")
include(FetchContent)
include(ExternalData)
#link SDL
if(${YORCVS_USE_VCPKG})
  message(STATUS "Linking SDL2 from vcpkg")
  #set SDL for vcpkg
  find_package(SDL2 CONFIG REQUIRED)
  find_package(SDL2-image CONFIG REQUIRED)
  find_package(SDL2-ttf CONFIG REQUIRED)
  set(SDL2lib SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_image SDL2::SDL2_ttf)
elseif(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  SET(CMAKE_CXX_FLAGS "-std=c++2a -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=\"[\"\"png\"\"]\"  -s USE_SDL_TTF=2  -s ALLOW_MEMORY_GROWTH=1 -s NO_DISABLE_EXCEPTION_CATCHING -o build/Yorcvs.html" ) #commented because emscriptem requires the assets folder to be in special place and it's not necesary for compilation for CI
else()
  message(STATUS "LINKING SDL2")
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
  set(SDL2lib SDL2::Main SDL2::Image SDL2::TTF)
endif()



#INSTALL JSON LIBRARY
message(STATUS "LINKING  JSON LIBRARY")
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)
message(STATUS "JSON SOURCE DIR : " ${json_SOURCE_DIR})
message(STATUS "JSONBUILD DIR : " ${json_BUILD_DIR} )

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
#INSTALL SOL2
FetchContent_Declare(sol2
 GIT_REPOSITORY https://github.com/ThePhD/sol2.git
 GIT_TAG "main" 
)
FetchContent_GetProperties(sol2)
if (NOT sol2_POPULATED)
   FetchContent_Populate(sol2)
endif()
#INSTALL IMGUI BASE
FetchContent_Declare(imgui
 GIT_REPOSITORY https://github.com/ocornut/imgui.git
 GIT_TAG "master" 
)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
   FetchContent_Populate(imgui)
  message(STATUS "imgui :" ${imgui_SOURCE_DIR})
  add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
  target_link_libraries(imgui ${SDL2lib})
  endif()
#INSTALL SDL2 BACKEND
FetchContent_Declare(imguiSDL2
 GIT_REPOSITORY https://github.com/Tyyppi77/imgui_sdl.git
 GIT_TAG "master" 
)
FetchContent_GetProperties(imguiSDL2)
if (NOT imguiSDL2_POPULATED)
  FetchContent_Populate(imguiSDL2)
  message(STATUS "imguiSDL2 :" ${imguisdl2_SOURCE_DIR})
  add_library(imgui-SDL2 STATIC ${imguisdl2_SOURCE_DIR}/imgui_sdl.cpp )
  target_link_libraries(imgui-SDL2 ${SDL2lib})
  target_link_libraries(imgui-SDL2  imgui)
  target_include_directories(imgui-SDL2 PUBLIC ${imgui_SOURCE_DIR})
endif()

add_subdirectory(yorcvs)
if(EMSCRIPTEN)
    if(YORCVS_EMSCRIPTEN_PRELOAD_ASSETS_FOLDER)
    message(STATUS "Assets folder will be preloaded")
    set_target_properties(Yorcvs PROPERTIES LINK_FLAGS  "--preload-file ./assets")
  endif()
endif()
add_subdirectory(tests)

