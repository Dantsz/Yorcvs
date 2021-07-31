
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#include "common/timer.h"
#include "window/windowSDL2.h"
#include <cstdlib>


static yorcvs::Window<yorcvs::SDL2> r;
yorcvs::Text<yorcvs::SDL2> *text;
static int count = 0;
yorcvs::Texture<yorcvs::SDL2> tee2;

/// Test
static int init()
{

    r.Init("TEst", 960, 500);
    text = new yorcvs::Text<yorcvs::SDL2>(
        r.createText("assets/font.ttf", "TEST111\n11", 255, 255, 255, 255, 32, 100)); // NOLINT
    r.registerCallback({[&](const SDL_Event &e) {
        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
        {
            yorcvs::log(std::string("MOUSE CLICKED AT ") + std::to_string(r.getCursorPosition().x) +
                        std::to_string(r.getCursorPosition().y));
        }
    }});
    return 0;
}

void run()
{
    yorcvs::Timer timy;
    timy.start();

    count++;

    r.handleEvents();

    yorcvs::Rect<float> dst = {0, 0, 100, 100};
    if (r.isKeyPressed(SDL_SCANCODE_W))
    {
        dst.x += 100;
        dst.y += 100;
    }

    yorcvs::Rect<size_t> src = {0, 0, 212, 229};
    r.clear();
    r.drawSprite("assets/lettuce.png", dst, src);

    yorcvs::Rect<float> textdst = {0, 0, 100, 100};
    r.setTextMessage(*text, std::to_string(count));

    r.drawText(*text, textdst);
    r.setTextColor(*text, rand() % 255, rand() % 255, rand() % 255, 255);

    r.present();
    yorcvs::log(timy.getTicks());
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
