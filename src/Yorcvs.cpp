
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "render/renderSDL2.h"

#include <cstdlib>

static yorcvs::Window<yorcvs::SDL2> r;
yorcvs::Text<yorcvs::SDL2> *text;
int count = 0;
yorcvs::Texture<yorcvs::SDL2> tee2;

/// Test
static int init()
{




    r.Init("TEst", 960, 500);
    text = new yorcvs::Text<yorcvs::SDL2>(r.createText(std::string("assets/font.ttf"), "TEST111\n11", 255, 255, 255, 255, 32, 100));//NOLINT

    return 0;
}

void run()
{
    count++;

    r.handleEvents();
    yorcvs::Rect<float> dst = {0, 0, 100, 100};
    yorcvs::Rect<size_t> src = {0, 0, 212, 229};
    r.clear();
    r.drawSprite("assets/lettuce.png", dst, src);

    yorcvs::Rect<float> textdst = {0, 0, 100, 100};
    r.setTextMessage(*text, std::to_string(count));

    r.drawText(*text, textdst);
    r.setTextColor(*text, rand() % 255, rand() % 255, rand() % 255, 255);

    r.present();
}

int cleanup()
{
    delete text;
    r.cleanup();
    return 0;
}

int main(int argc, char **argv)
{

    init();
#ifdef __EMSCRIPTEN__
    std::cout << "running EMSCRIPTEM";
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
