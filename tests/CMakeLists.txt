cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR} @ONLY)
enable_testing()

project(ECSTest)
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
find_package(nlohmann_json CONFIG REQUIRED)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)
find_package(SDL2-ttf CONFIG REQUIRED)

add_executable(ECSTestEntitiesAndComponents ECSTestEntitiesAndComponents.cpp)
add_test(ECSTestEntitiesAndComponents ECSTestEntitiesAndComponents)

add_executable(ECSComponentCreationDeletion ECSComponentCreationDeletion.cpp)
add_test(ECSComponentCreationDeletion ECSComponentCreationDeletion)

add_executable(ECSTestComponentManagerCopyConstructor ECSTestComponentManagerCopyConstructor.cpp)
add_test(ECSTestComponentManagerCopyConstructor ECSTestComponentManagerCopyConstructor)

add_executable(ECSEntityManagerCopyMove ECSEntityManagerCopyMove.cpp)
add_test(ECSEntityManagerCopyMove ECSEntityManagerCopyMove)

add_executable(ECSSystemManagerCopyMove ECSSystemManagerCopyMove.cpp)
add_test(ECSSystemManagerCopyMove ECSSystemManagerCopyMove)

add_executable(ECSsystems ECSsystems.cpp)
add_test(ECSsystems ECSsystems)

add_executable(ECSmainclassCopyMove ECSmainclassCopyMove.cpp)
add_test(ECSmainclassCopyMove ECSmainclassCopyMove)

add_executable(YorcvsLoadMap YorcvsLoadMap.cpp)
add_test(YorcvsLoadMap YorcvsLoadMap)
target_link_libraries(YorcvsLoadMap PRIVATE nlohmann_json::nlohmann_json tmxlite  SDL2::SDL2 SDL2::SDL2main  SDL2::SDL2_image SDL2::SDL2_ttf)

