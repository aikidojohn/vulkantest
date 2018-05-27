#ifndef blok_model_h
#define blok_model_h

#include <vulkan/vulkan.h>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <array>

namespace blok {
	template <class Vert>
	class Model {
	private:
		std::vector<Vert> vertices;
		std::vector<uint32_t> indices;
	};
};
#endif // !blok_model_h
