#ifndef blok_texture_h
#define blok_texture_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <cstring>

namespace blok {

	struct Texture {
		uint32_t width;
		uint32_t height;
		uint32_t layerCount;
		uint32_t mipLevels;
		VkImage texture;
	};

	class TextureManager {
	public:
		Texture loadTextures(std::vector<std::string> texturePaths, bool generateMipmap) {
			Texture texture;
			texture.layerCount = texturePaths.size();
		}
	};
}; // !namespace blok
#endif // !blok_texture_h