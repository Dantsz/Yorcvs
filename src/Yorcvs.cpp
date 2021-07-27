
#include <iostream>
#ifdef __EMSCRIPTEM__
	#include <emscripten.h>
#endif
#include <atomic>
#include "assetmanager/assetmanager.h"
//Initialize

#include "common/types.h"

SDL_Window* window;
inline SDL_Renderer* renderer;
assetManager<SDL_Texture> texManager;
TTF_Font* font;
SDL_Color fg = {255,255,255,255};
SDL_Color bg = {0,0,0,0};
SDL_Surface* surf;
SDL_Texture* text;
SDL_Surface* lettuce_sur;
std::shared_ptr<SDL_Texture> lettuce_tex;
Vec2<int> vectest;


///Test
int init()
{
	
	std::cin>> vectest;
	 if (SDL_Init(SDL_INIT_VIDEO ) < 0) {
		std::cout << "Error SDL2 Initialization : " << SDL_GetError();
		return 1;
	}
		
	if (IMG_Init(IMG_INIT_PNG) == 0) {
		std::cout << "Error SDL2_image Initialization";
		return 2;
	}

	if(TTF_Init() < 0)
	{
		std::cout<< "Font error" << '\n';
		return 3;
	}

	window  = SDL_CreateWindow("First program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		std::cout << "Error window creation";
		return 3;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		std::cout << "Error renderer creation";
		return 4;
	}

	
	lettuce_tex = texManager.loadFromFile("assets/lettuce.png");
	lettuce_tex = texManager.loadFromFile("assets/lettuce.png");
	texManager.loadFromFile("assets/lettuce.png");
	texManager.loadFromFile("assets/lettuce.png");
	texManager.loadFromFile("assets/lettuce.png");
	if (lettuce_tex == NULL) {
		std::cout << "Error creating texture";
	
	}
	SDL_FreeSurface(lettuce_sur);

	std::cout<< "Opening Fonts\n";
	font  = TTF_OpenFont("assets/font.ttf",1000);

	std::cout<< "Creating textures\n";
	surf = TTF_RenderText_Blended(font,"test test",fg);
	text = SDL_CreateTextureFromSurface(renderer,surf);
	std::cout<<"Running";

	return 0;
}

void run()
{
	
	
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				
			}
		}

		SDL_RenderClear(renderer);
        SDL_Rect rect1 = {0,0,100,100};
		SDL_RenderCopy(renderer, lettuce_tex.get(), nullptr, &rect1);
        rect1 = {100,100,100,100};

        SDL_RenderCopy(renderer, lettuce_tex.get(), nullptr, &rect1);
		
	
		
		SDL_Rect textRect = {vectest.x,vectest.y,300,300};
		
		SDL_RenderCopy(renderer,text,nullptr,&textRect);


			
		SDL_RenderPresent(renderer);
}


int cleanup()
{
	SDL_DestroyTexture(text);
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	SDL_DestroyTexture(lettuce_tex.get());
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	TTF_Quit();	
	return 0;
}

int main(int argc,char** argv)
{
 	init();
	#ifdef __EMSCRIPTEM__
		emscripten_set_main_loop(run,0,1);
	#else
		while(true)
		{
			run();
		}
	#endif
	cleanup();
	return 0;
}




