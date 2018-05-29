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

	public:
		void add(Vert v) {
			vertices.push_back(v);
		}

		void addIndex(uint32_t i) {
			indices.push_back(i);
		}
	};
};
#endif // !blok_model_h
