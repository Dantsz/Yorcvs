configure_file(${CMAKE_CURRENT_LIST_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR} @ONLY)
project(ECSTest)
set(test_dir ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "Yorcvs source for tests: " ${YorcvsIncludeDIRS})

if(UNIX AND NOT APPLE)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fsanitize-undefined-trap-on-error -fno-omit-frame-pointer  -fsanitize=address,undefined,leak")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize-undefined-trap-on-error -fno-omit-frame-pointer -fsanitize=address,undefined,leak")
endif()

add_compile_definitions(TEST_MAP_FILE="${YorcvsAssetsFolder}/map.tmx")
add_compile_definitions(TEST_TEMP_ENTITY_FILE="${YorcvsAssetsFolder}/entities/bleeding_duck/bleeding_duck.json")
add_compile_definitions(TEST_LOAD_ENTITY_FILE="${YorcvsAssetsFolder}/entities/duck/duck.json")

add_executable(ECSTestEntitiesAndComponents src/ECSTestEntitiesAndComponents.cpp )
target_include_directories(ECSTestEntitiesAndComponents PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestEntitiesAndComponents ECSTestEntitiesAndComponents )

add_executable(ECSComponentCreationDeletion src/ECSComponentCreationDeletion.cpp)
target_include_directories(ECSComponentCreationDeletion PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSComponentCreationDeletion ECSComponentCreationDeletion)

add_executable(ECSTestComponentManagerCopyConstructor src/ECSTestComponentManagerCopyConstructor.cpp)
target_include_directories(ECSTestComponentManagerCopyConstructor PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestComponentManagerCopyConstrucpptor ECSTestComponentManagerCopyConstructor)

add_executable(ECSEntityManagerCopyMove src/ECSEntityManagerCopyMove.cpp)
target_include_directories(ECSEntityManagerCopyMove PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSEntityManagerCopyMove ECSEntityManagerCopyMove)

add_executable(ECSSystemManagerCopyMove src/ECSSystemManagerCopyMove.cpp)
target_include_directories(ECSSystemManagerCopyMove PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSSystemManagerCopyMove ECSSystemManagerCopyMove)

add_executable(ECSsystems src/ECSsystems.cpp)
target_include_directories(ECSsystems PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSsystems ECSsystems)

add_executable(ECSmainclassCopyMove src/ECSmainclassCopyMove.cpp)
target_include_directories(ECSmainclassCopyMove PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSmainclassCopyMove ECSmainclassCopyMove)

add_executable(ECSTestEntityDestruction src/ECSTestEntityDestruction.cpp)
target_include_directories(ECSTestEntityDestruction PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestEntityDestruction ECSTestEntityDestruction)

add_executable(ECSTestSystemUnregistering src/ECSTestSystemUnregistering.cpp)
target_include_directories(ECSTestSystemUnregistering PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestSystemUnregistering ECSTestSystemUnregistering)

add_executable(ECSTestInvalidEntitySignature src/ECSTestInvalidEntitySignature.cpp)
target_include_directories(ECSTestInvalidEntitySignature PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestInvalidEntitySignature ECSTestInvalidEntitySignature)

add_executable(ECSTestAddingComponentToEntityWithComponentBehavior src/ECSTestAddingComponentToEntityWithComponentBehavior.cpp)
target_include_directories(ECSTestAddingComponentToEntityWithComponentBehavior PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestAddingComponentToEntityWithComponentBehavior ECSTestAddingComponentToEntityWithComponentBehavior)

add_executable(ECSTestAddEntityChecked src/ECSTestAddEntityChecked.cpp)
target_include_directories(ECSTestAddEntityChecked PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestAddEntityChecked ECSTestAddEntityChecked)

add_executable(ECSTestSystemAddSetCriteria src/ECSTestSystemAddSetCriteria.cpp)
target_include_directories(ECSTestSystemAddSetCriteria PUBLIC ${YorcvsIncludeDIRS})
add_test(ECSTestSystemAddSetCriteria ECSTestSystemAddSetCriteria)

add_executable(TypesTestRectContains src/TypesTestRectContains.cpp)
target_include_directories(TypesTestRectContains PUBLIC ${YorcvsIncludeDIRS})
add_test(NAME TypesTestRectContains COMMAND TypesTestRectContains WORKING_DIRECTORY ${test_dir} )


add_executable(UtilitiesTestSpiral src/UtilitiesTestSpiral.cpp)
target_include_directories(UtilitiesTestSpiral PUBLIC ${YorcvsIncludeDIRS})
add_test(NAME UtilitiesTestSpiral COMMAND UtilitiesTestSpiral WORKING_DIRECTORY ${test_dir} )

add_executable(ECStestentityduplicate src/ECStestentityduplicate.cpp)
target_include_directories(ECStestentityduplicate PUBLIC ${YorcvsIncludeDIRS})
add_test(NAME ECStestentityduplicate COMMAND ECStestentityduplicate  WORKING_DIRECTORY ${test_dir} )

add_executable(YorcvsLoadMap src/YorcvsLoadMap.cpp)
add_test(NAME YorcvsLoadMap  COMMAND YorcvsLoadMap WORKING_DIRECTORY ${test_dir})
target_link_libraries(YorcvsLoadMap PRIVATE lua::header nlohmann_json::nlohmann_json tmxlite  ${SDL2lib} lua::lib imgui imgui-SDL2)
target_include_directories(YorcvsLoadMap PUBLIC ${YorcvsIncludeDIRS} ${sol2_SOURCE_DIR}/include ${IMGUI_INCLUDE_DIRS})

add_executable(YorcvsLoadCharacter src/YorcvsLoadCharacter.cpp)
add_test(NAME YorcvsLoadCharacter  COMMAND YorcvsLoadCharacter WORKING_DIRECTORY ${test_dir})
target_link_libraries(YorcvsLoadCharacter PRIVATE  lua::header nlohmann_json::nlohmann_json tmxlite  ${SDL2lib} lua::lib imgui imgui-SDL2)
target_include_directories(YorcvsLoadCharacter PUBLIC ${YorcvsIncludeDIRS} ${sol2_SOURCE_DIR}/include ${IMGUI_INCLUDE_DIRS})

add_executable(WorldTestnegativeHealthRegen src/WorldTestnegativeHealthRegen.cpp)
add_test(NAME WorldTestnegativeHealthRegen COMMAND WorldTestnegativeHealthRegen WORKING_DIRECTORY ${test_dir})
target_link_libraries(WorldTestnegativeHealthRegen PRIVATE lua::header nlohmann_json::nlohmann_json tmxlite  ${SDL2lib} lua::lib imgui imgui-SDL2)
target_include_directories(WorldTestnegativeHealthRegen PUBLIC  ${YorcvsIncludeDIRS} ${sol2_SOURCE_DIR}/include ${IMGUI_INCLUDE_DIRS})

add_executable(GameTestLoadingEntity src/GameTestLoadingEntity.cpp)
add_test(NAME GameTestLoadingEntity COMMAND GameTestLoadingEntity WORKING_DIRECTORY ${test_dir} )
target_link_libraries(GameTestLoadingEntity PRIVATE lua::header nlohmann_json::nlohmann_json tmxlite  ${SDL2lib} lua::lib imgui imgui-SDL2)
target_include_directories(GameTestLoadingEntity PUBLIC  ${YorcvsIncludeDIRS} ${sol2_SOURCE_DIR}/include  ${IMGUI_INCLUDE_DIRS})



