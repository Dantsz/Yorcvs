
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
yorcvs::Entity tim{&world};


yorcvs::Timer timy;
size_t FT = 0;
/// Test
static int init()
{
    
    r.Init("TEst", 960, 500);

    world.addComponent<hitboxComponent>(tim.id,{{10,10,200,200}});
    world.addComponent<positionComponent>(tim.id,{{100,100}});

    return 0;
    
}

void run()
{
    timy.start();
   



    r.handleEvents();
    
    r.clear();
    collisionS.update(1.0f,&r);

    r.present();
    FT = timy.getTicks();
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
