
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#include "common/timer.h"
#include "window/windowSDL2.h"
#include <cstdlib>
#include "Yorcvs.h"
#include "ecs/ecs.h"
#include "systems.h"
static yorcvs::Window<yorcvs::SDL2> r;
static yorcvs::ECS world{};
static CollisionSystem collisionS{&world};
static VelocitySystem velocityS{&world};
yorcvs::Entity tim{&world};


yorcvs::Timer timy;
size_t FT = 0;
/// Test
static int init()
{
    
    r.init("TEst", 960, 500);
   

    world.add_component<hitboxComponent>(tim.id,{{10,10,200,200}});
    world.add_component<positionComponent>(tim.id,{{100,100}});
    world.add_component<velocityComponent>(tim.id,{{1.0f,0.0f},{0.5f,0.5f}});
    return 0;
    
}

void run()
{
    timy.start();
   



    r.handle_events();
    
    r.clear();
    collisionS.render(1.0f,&r);
    velocityS.update(1.0f);
    r.present();
    FT = timy.get_ticks();
}

int cleanup()
{
    
    r.cleanup();
    return 0;
}

int main(int argc, char **argv)
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
