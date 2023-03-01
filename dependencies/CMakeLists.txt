#INSTALL IMGUI BASE
FetchContent_Declare(imgui
 GIT_REPOSITORY https://github.com/ocornut/imgui.git
 GIT_TAG "master"
)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
   FetchContent_Populate(imgui)
  message(STATUS "imgui :" ${imgui_SOURCE_DIR})
  add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
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

set(IMGUI_INCLUDE_DIRS ${IMGUI_INCLUDE_DIRS} ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${imguisdl2_SOURCE_DIR} PARENT_SCOPE)
