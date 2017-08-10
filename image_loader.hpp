#pragma once

#include <string>
#include <utility>
#include <vector>

/**
 * SDL Image wrapper from Tee3d
 */
class ImageLoader {
private:
	ImageLoader();
	friend ImageLoader *g_ImageLoader();

public:
	~ImageLoader();
	/**
	 * Read image and parse data to RGBA format
	 * @param  path
	 */
	std::tuple<uint32_t, uint32_t, std::vector<uint8_t>>
	Load(const std::string &path);
};

/**
 * Singleton instance-getter for ImageLoader
 * @return single instance
 */
ImageLoader *g_ImageLoader();
