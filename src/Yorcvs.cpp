
#include <iostream>
#ifdef __EMSCRIPTEM__
	#include <emscripten.h>
#endif
#include <atomic>
#include "assetmanager/assetmanager.h"
//Initialize

#include "common/types.h"
#include "render/renderSDL2.h"



static yorcvs::Window<yorcvs::SDL2> r;
yorcvs::Texture<yorcvs::SDL2>* text;
bool isRunning = true;
std::shared_ptr<TTF_Font> fontest;

yorcvs::Texture<yorcvs::SDL2> tee2;
///Test
static int init()
{

	r.Init("TEst",960,500);
	text = new yorcvs::Texture<yorcvs::SDL2>(r.createText(std::string("assets/font.ttf"),"TEST111\n11",255,255,255,255,32,100));
	
	return 0;
}

void run()
{
	
	
	r.handleEvents();
	yorcvs::Rect<float> dst = {0,0,100,100};
	yorcvs::Rect<size_t> src = {0,0,212,229};
	r.clear();
	r.drawSprite("assets/lettuce.png",dst,src);

	yorcvs::Rect<float> textdst = {100,200,100,100};
	r.drawText(*text,textdst);



	r.present();
		
}


int cleanup()
{	
	delete text;
	r.cleanup();
}

int main(int argc,char** argv)
{
	
 	init();
	#ifdef __EMSCRIPTEM__
		std::cout<<"running EMSCRIPTEM";
		emscripten_set_main_loop(run,0,1);
	#else
		while(r.isActive)
		{
			run();
		}
	#endif
	cleanup();
	return 0;
}




