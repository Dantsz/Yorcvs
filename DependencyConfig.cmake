message(STATUS "USING VCPKG: ${YORCVS_USE_VCPKG}")
include(FetchContent)
include(ExternalData)
#link SDL
if(${YORCVS_USE_VCPKG})
  message(STATUS "Linking SDL2 from vcpkg")
  #set SDL for vcpkg
  find_package(SDL2 CONFIG REQUIRED)
  find_package(SDL2-image CONFIG REQUIRED)
  find_package(SDL2_ttf CONFIG REQUIRED)
  set(SDL2lib SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_image $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
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
add_subdirectory(dependencies)
