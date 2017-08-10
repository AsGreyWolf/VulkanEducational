#include "image_loader.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <memory>
#include <tuple>

ImageLoader::ImageLoader() {
	if (IMG_Init(IMG_INIT_PNG) == 0) {
		std::cerr << "Unable to initialize SDL_IMG: " << std::string(IMG_GetError())
		          << std::endl;
		return;
	}
	SDL_version ver = *IMG_Linked_Version();
	std::cout << "Initialized SDL_IMG " << std::to_string(ver.major) << "."
	          << std::to_string(ver.minor) << "." << std::to_string(ver.patch)
	          << std::endl;
}
ImageLoader::~ImageLoader() { IMG_Quit(); }
std::tuple<uint32_t, uint32_t, std::vector<uint8_t>>
ImageLoader::Load(const std::string &path) {
	SDL_Surface *temp;
	if ((temp = IMG_Load(path.c_str())) == nullptr) {
		std::cerr << "Error Loading Texture: " << path << " : "
		          << std::string(IMG_GetError()) << std::endl;
		return {0, 0, {}};
	}
	SDL_Surface *converted =
	    SDL_ConvertSurfaceFormat(temp, SDL_PIXELFORMAT_ABGR8888, 0);
	SDL_FreeSurface(temp);
	size_t w = converted->w;
	size_t h = converted->h;
	auto pixels = reinterpret_cast<uint8_t *>(converted->pixels);
	std::vector<uint8_t> data(pixels, pixels + w * h * 4);
	SDL_FreeSurface(converted);
	std::cout << "Image loaded " << path << std::endl;
	return {w, h, move(data)};
}
ImageLoader *g_ImageLoader() {
	static std::unique_ptr<ImageLoader> pImageLoader{new ImageLoader{}};
	return pImageLoader.get();
}
