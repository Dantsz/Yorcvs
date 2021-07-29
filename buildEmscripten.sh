if [[ ! -d "build/Emscripten" ]]  
then 
    mkdir build/Emscripten
fi
em++ src/Yorcvs.cpp -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]'  -s USE_SDL_TTF=2  --embed-file assets  -s ALLOW_MEMORY_GROWTH=1 -o build/Emscripten/a.html

emrun build/Emscripten/a.html

echo "Done"
sleep 10s