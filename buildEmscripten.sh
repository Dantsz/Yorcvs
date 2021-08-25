if [[ ! -d "build/Emscripten" ]]  
then 
    mkdir build/Emscripten
fi
em++ src/Yorcvs.cpp -std=c++2a -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]'  -s USE_SDL_TTF=2  --embed-file assets  -s ALLOW_MEMORY_GROWTH=1 -o build/Emscripten/yorcvs.html

emrun build/Emscripten/yorcvs.html

echo "Done"
sleep 10s