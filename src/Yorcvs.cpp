
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif
#include "Yorcvs.h"
#include "common/timer.h"
#include "window/windowSDL2.h"
#include <cstdlib>
#include "Yorcvs.h"
#include "ecs/ecs.h"
#include "systems.h"
#include "systemsSDL2.h"
static yorcvs::Window<yorcvs::SDL2> r;
static yorcvs::ECS world{};
static CollisionSystem collisionS{&world};
static VelocitySystem velocityS{&world};
static PlayerMovementControl pcS{&world,&r};
static SpriteSystem sprS{&world,&r};
yorcvs::Entity tim{&world};
yorcvs::Entity jim{&world};
yorcvs::Entity dim{&world};
yorcvs::Entity pim{&world};
yorcvs::Timer timy;
float prevTime = 0.0f;
float curTime = 0.0f;
constexpr float msPF = 16.6f;
float lag = 0.0f;

size_t FT;
/// Test
int init()
{
    
    r.init("TEst", 960, 500);
   

    world.add_component<hitboxComponent>(tim.id,{{75,35,20,45}});
    world.add_component<positionComponent>(tim.id,{{100,100}});
    world.add_component<velocityComponent>(tim.id,{{0.0f,0.0f}});
    world.add_component<playerMovementControlledComponent>(tim.id,{});
    world.add_component<spriteComponent>(tim.id,{{0.0f,0.0f},{160.0f,160.0f},{0,0,32,32},r.create_texture("assets/test_player_sheet.png")});

    world.add_component<hitboxComponent>(jim.id,{{0,0,160,160}});
    world.add_component<positionComponent>(jim.id,{{500,100}});
    world.add_component<spriteComponent>(jim.id,{{0.0f,0.0f},{160.0f,160.0f},{0,0,200,200},r.create_texture("assets/lettuce.png")});

    world.add_component<hitboxComponent>(pim.id,{{0,0,160,160}});
    world.add_component<positionComponent>(pim.id,{{660,100}});
    world.add_component<spriteComponent>(pim.id,{{0.0f,0.0f},{160.0f,160.0f},{0,0,200,200},r.create_texture("assets/lettuce.png")});

    world.add_component<hitboxComponent>(dim.id,{{0,0,160,160}});
    world.add_component<positionComponent>(dim.id,{{500,260}});
    world.add_component<spriteComponent>(dim.id,{{0.0f,0.0f},{160.0f,160.0f},{0,0,200,200},r.create_texture("assets/lettuce.png")});
    timy.start();
    return 0;
   
    
}

void run()
{
    float elapsed = timy.get_ticks<float,std::chrono::nanoseconds>();
    elapsed /= 1000000.0f;
   
    timy.start();
    lag += elapsed;
   
    r.handle_events();
    
    while(lag >= msPF)
    {
      pcS.update();
      collisionS.update(lag);
      velocityS.update(lag);
      
      lag -= msPF;
    }


  
    
    r.clear();
    sprS.renderSprites();
    r.present();


    
}

int cleanup()
{
    
    r.cleanup();
    return 0;
}

int main(int argc, char **argv) //NOLINT
{

    init();
#ifdef __EMSCRIPTEN__
    yorcvs::log("running EMSCRIPTEM");
    emscripten_set_main_loop(run, 0, 1);
#else
    while (r.isActive)
    {   
        run();
    }
#endif
    cleanup();
    return 0;
}
