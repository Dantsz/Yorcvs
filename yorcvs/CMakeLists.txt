project(Yorcvs)

set(YorcvsCORESFILES    "src/common/assetmanager.h"
                        "src/common/types.h"
                        "src/common/ecs.h"

                        "src/common/utilities.h"
                        "src/common/utilities/timer.h"
                        "src/common/utilities/ulamspiral.h"
                        "src/common/utilities/log.h"


                        "src/engine/luaEngine.h"
                        "src/engine/map.h"
                        )
set(YorcvsGAMEFILES     "src/game/components.h"
                        "src/game/component_serialization.h"
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
                        "src/game/systems/items.h"
                        "src/engine/entity_loader.h"
)
set(YorcvsUIFILES "src/ui/debuginfo.h"
                  "src/ui/performancewindow.h"
                  "src/ui/assetmanagerviewer.h"
                  "src/ui/animationeditor.h"
                  "src/ui/entityinteraction.h")
set(YORCVSENGINEFILES   "src/engine/window/window.h"
                        "src/engine/window/eventhandler.h"
                        "src/engine/window/windowsdl2"
                        "src/engine/window/eventhandlersdl2.h")

set(YorcvsALLFILES  "src/Yorcvs.h"
                    ${YorcvsCORESFILES}
                    ${YorcvsGAMEFILES}
                    ${YorcvsUIFILES}
                    ${YORCVSENGINEFILES}
                    )
get_filename_component(yorcvs_include_path src ABSOLUTE)
set(YorcvsIncludeDIRS ${yorcvs_include_path} PARENT_SCOPE)

add_executable(Yorcvs "src/Yorcvs.cpp" ${YorcvsALLFILES})
message(STATUS "BUILDING YORCVS ON " ${CMAKE_SYSTEM_NAME})
#COMPILE FLAGS
target_compile_options(Yorcvs PRIVATE
$<$<CXX_COMPILER_ID:MSVC>:/W4 /Zi /DEBUG /bigobj>
$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -std=c++2a>
$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wno-c++98-compat-pedantic -Wno-c++98-compat>
)
target_include_directories(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json ${sol2_SOURCE_DIR}/include ${IMGUI_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} lua::header ${SDL2lib} nlohmann_json::nlohmann_json tmxlite lua::lib imgui imgui-SDL2)
