Write-Host "Building..."
g++ src/main.cpp src/graphics/graphics.cpp src/audio/audio.cpp src/parser/parser.cpp src/handler.cpp -lSDL3 -lSDL3_ttf -lSDL3_image -o build/build.exe
./build/build.exe