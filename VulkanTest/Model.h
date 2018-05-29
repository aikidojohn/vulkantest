#ifndef blok_model_h
#define blok_model_h

#include <vulkan/vulkan.h>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <array>

namespace blok {
	template <class Vert>
	class Mesh {

	public:
		std::vector<Vert> vertices;
		std::vector<uint32_t> indices;

		/*
		Adds vertext to mesh and returns it's index.
		*/
		uint32_t addVertex(Vert const& v) {
			vertices.push_back(v);
			return static_cast<uint32_t>(vertices.size())-1;
		}

		void addTriangle(uint32_t i, uint32_t j, uint32_t k) {
			indices.push_back(i);
			indices.push_back(j);
			indices.push_back(k);
		}

		void clear() {
			vertices.clear();
			indices.clear();
		}
	};
};
#endif // !blok_model_h
