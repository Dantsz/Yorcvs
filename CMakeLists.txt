cmake_minimum_required(VERSION 3.13)
project(Zepyt)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)
find_package(SDL2 REQUIRED)



add_executable(Zepyt "src/Zepyt.cpp")

target_link_libraries(${PROJECT_NAME} SDL2::Main)



target_compile_options(Zepyt PRIVATE
$<$<CXX_COMPILER_ID:MSVC>:/W4>
$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic >
)