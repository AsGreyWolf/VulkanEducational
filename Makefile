
shaders:
	glslangValidator -V shader.vert
	glslangValidator -V shader.frag
	glslangValidator -V shader.tesc
	glslangValidator -V shader.tese
program:
	g++ -std=c++17 main.cpp image_loader.cpp glfw.cpp utilities.cpp loaders.cpp pipeline.cpp -lvulkan -lglfw -lSDL2 -lSDL2_image -Og -g -pipe -march=native # -fsanitize=thread -pie

all: shaders program
