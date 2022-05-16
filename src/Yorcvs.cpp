
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "Yorcvs.h"
[[maybe_unused]]
static yorcvs::Application *m_app;
[[maybe_unused]]
static void run_emscripten()
{
    m_app->run();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) // NOLINT
{
    yorcvs::Application app;
    m_app = &app;
#ifdef __EMSCRIPTEN__
    yorcvs::log("running EMSCRIPTEM");
    emscripten_set_main_loop(run_emscripten, 0, 1);
#else
    while (app.is_active())
    {
        app.run();
    }
#endif

    return 0;
}
