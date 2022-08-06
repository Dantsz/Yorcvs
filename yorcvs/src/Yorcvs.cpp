
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "Yorcvs.h"

[[maybe_unused]]
static void run_emscripten([[maybe_unused]]void* app)
{
    static_cast<yorcvs::Application*>(app)->run();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) // NOLINT
{
    yorcvs::Application app;
   
#ifdef __EMSCRIPTEN__
    yorcvs::log("running EMSCRIPTEM");
    emscripten_set_main_loop_arg(run_emscripten, static_cast<void*>(&app),0, 1);
#else
    while (app.is_active())
    {
        app.run();
    }
#endif

    return 0;
}
