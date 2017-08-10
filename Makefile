
shaders: shaders/src/shader.vert shaders/src/shader.frag shaders/src/shader.tesc shaders/src/shader.tese
	glslangValidator -V shaders/src/shader.vert -o shaders/vert.spv
	glslangValidator -V shaders/src/shader.frag -o shaders/frag.spv
	glslangValidator -V shaders/src/shader.tesc -o shaders/tesc.spv
	glslangValidator -V shaders/src/shader.tese -o shaders/tese.spv

program: src/main.cpp src/image_loader.cpp src/glfw.cpp src/utilities.cpp src/loaders.cpp src/pipeline.cpp
	cd src && g++ -std=c++17 main.cpp image_loader.cpp glfw.cpp utilities.cpp loaders.cpp pipeline.cpp -lvulkan -lglfw -lSDL2 -lSDL2_image -Og -g -pipe -march=native -o ../a.out # -fsanitize=thread -pie

all: shaders program

clean:
	rm -rf shaders/*.spv
	rm -rf src/*.o
	rm -rf src/*.gch
	rm -rf a.out
