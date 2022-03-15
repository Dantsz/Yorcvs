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
find_package(nlohmann_json CONFIG REQUIRED)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2-image CONFIG REQUIRED)
find_package(SDL2-ttf CONFIG REQUIRED)

add_executable(ECSTestEntitiesAndComponents src/ECSTestEntitiesAndComponents.cpp )
add_test(ECSTestEntitiesAndComponents ECSTestEntitiesAndComponents )

add_executable(ECSComponentCreationDeletion src/ECSComponentCreationDeletion.cpp)
add_test(ECSComponentCreationDeletion ECSComponentCreationDeletion)

add_executable(ECSTestComponentManagerCopyConstructor src/ECSTestComponentManagerCopyConstructor.cpp)
add_test(ECSTestComponentManagerCopyConstructor ECSTestComponentManagerCopyConstructor)

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
target_link_libraries(YorcvsLoadMap PRIVATE nlohmann_json::nlohmann_json tmxlite  SDL2::SDL2 SDL2::SDL2main  SDL2::SDL2_image SDL2::SDL2_ttf)

add_executable(WorldTestnegativeHealthRegen src/WorldTestnegativeHealthRegen.cpp)
add_test(NAME WorldTestnegativeHealthRegen COMMAND WorldTestnegativeHealthRegen WORKING_DIRECTORY ${test_dir})
target_link_libraries(WorldTestnegativeHealthRegen PRIVATE nlohmann_json::nlohmann_json tmxlite  SDL2::SDL2 SDL2::SDL2main  SDL2::SDL2_image SDL2::SDL2_ttf)

add_executable(GameTestLoadingEntity src/GameTestLoadingEntity.cpp)
add_test(NAME GameTestLoadingEntity COMMAND GameTestLoadingEntity WORKING_DIRECTORY ${test_dir})
target_link_libraries(GameTestLoadingEntity PRIVATE nlohmann_json::nlohmann_json tmxlite  SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_image SDL2::SDL2_ttf)


add_executable(UtilitiesTestSpiral src/UtilitiesTestSpiral.cpp)
add_test(NAME UtilitiesTestSpiral COMMAND UtilitiesTestSpiral  WORKING_DIRECTORY ${test_dir})

