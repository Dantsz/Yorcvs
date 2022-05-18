cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR} @ONLY)
enable_testing()

project(ECSTest)

set(test_dir ${CMAKE_SOURCE_DIR})
include(FetchContent)

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
if(UNIX AND NOT APPLE)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fsanitize-undefined-trap-on-error -fno-omit-frame-pointer  -fsanitize=address,undefined,leak")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize-undefined-trap-on-error -fno-omit-frame-pointer -fsanitize=address,undefined,leak")
endif()
if(WIN32)
else()
   SET(LUA_CMAKE_BUILD_SHARED ON CACHE BOOL "Build as shared library" FORCE)
endif()

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)
message(STATUS "SOURCE DIR : " ${json_SOURCE_DIR})
message(STATUS "BUILD DIR : " ${json_BUILD_DIR} )

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)
find_package(SDL2-ttf CONFIG REQUIRED)

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
#target_link_libraries(${PROJECT_NAME} lua::lib)
#INSTALL SOL2
FetchContent_Declare(sol2
 GIT_REPOSITORY https://github.com/ThePhD/sol2.git
 GIT_TAG "main" 
)
FetchContent_GetProperties(sol2)
if (NOT sol2_POPULATED)
   FetchContent_Populate(sol2)
  #target_include_directories(${PROJECT_NAME} PUBLIC ${sol2_SOURCE_DIR}/include)
endif()
set(SDL2lib  SDL2::SDL2)
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
  target_link_libraries(imgui  ${SDL2lib})
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




add_executable(ECSTestEntitiesAndComponents src/ECSTestEntitiesAndComponents.cpp )
add_test(ECSTestEntitiesAndComponents ECSTestEntitiesAndComponents )

add_executable(ECSComponentCreationDeletion src/ECSComponentCreationDeletion.cpp)
add_test(ECSComponentCreationDeletion ECSComponentCreationDeletion)

add_executable(ECSTestComponentManagerCopyConstructor src/ECSTestComponentManagerCopyConstructor.cpp)
add_test(ECSTestComponentManagerCopyConstrucpptor ECSTestComponentManagerCopyConstructor)

add_executable(ECSEntityManagerCopyMove src/ECSEntityManagerCopyMove.cpp)
add_test(ECSEntityManagerCopyMove ECSEntityManagerCopyMove)

add_executable(ECSSystemManagerCopyMove src/ECSSystemManagerCopyMove.cpp)
add_test(ECSSystemManagerCopyMove ECSSystemManagerCopyMove)

add_executable(ECSsystems src/ECSsystems.cpp)
add_test(ECSsystems ECSsystems)

add_executable(ECSmainclassCopyMove src/ECSmainclassCopyMove.cpp)
add_test(ECSmainclassCopyMove ECSmainclassCopyMove)

add_executable(YorcvsLoadMap src/YorcvsLoadMap.cpp)
add_test(NAME YorcvsLoadMap  COMMAND YorcvsLoadMap WORKING_DIRECTORY ${test_dir})
target_link_libraries(YorcvsLoadMap PRIVATE nlohmann_json::nlohmann_json tmxlite  SDL2::SDL2 SDL2::SDL2main  SDL2::SDL2_image SDL2::SDL2_ttf lua::lib imgui imgui-SDL2)
target_include_directories(YorcvsLoadMap PUBLIC ${sol2_SOURCE_DIR}/include ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends  ${imguisdl2_SOURCE_DIR})

add_executable(WorldTestnegativeHealthRegen src/WorldTestnegativeHealthRegen.cpp)
add_test(NAME WorldTestnegativeHealthRegen COMMAND WorldTestnegativeHealthRegen WORKING_DIRECTORY ${test_dir})
target_link_libraries(WorldTestnegativeHealthRegen PRIVATE nlohmann_json::nlohmann_json tmxlite  SDL2::SDL2 SDL2::SDL2main  SDL2::SDL2_image SDL2::SDL2_ttf lua::lib imgui imgui-SDL2)
target_include_directories(WorldTestnegativeHealthRegen PUBLIC ${sol2_SOURCE_DIR}/include ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends  ${imguisdl2_SOURCE_DIR})

add_executable(GameTestLoadingEntity src/GameTestLoadingEntity.cpp)
add_test(NAME GameTestLoadingEntity COMMAND GameTestLoadingEntity WORKING_DIRECTORY ${test_dir})
target_link_libraries(GameTestLoadingEntity PRIVATE nlohmann_json::nlohmann_json tmxlite  SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_image SDL2::SDL2_ttf lua::lib imgui imgui-SDL2)
target_include_directories(GameTestLoadingEntity PUBLIC ${sol2_SOURCE_DIR}/include  ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends  ${imguisdl2_SOURCE_DIR})

add_executable(UtilitiesTestSpiral src/UtilitiesTestSpiral.cpp)
add_test(NAME UtilitiesTestSpiral COMMAND UtilitiesTestSpiral  WORKING_DIRECTORY ${test_dir})

add_executable(ECStestentityduplicate src/ECStestentityduplicate.cpp)
add_test(NAME ECStestentityduplicate COMMAND ECStestentityduplicate  WORKING_DIRECTORY ${test_dir})