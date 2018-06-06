#pragma once
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <functional>
#include <optional>
#include <cstdlib>
#include <vector>
#include <cstring>

#include "ChunkManager.h"
namespace blok {
	class World {
	public:

		std::optional<Block> getBlockAt(glm::vec3 position) {
			return chunkManager.getBlockAt(position);
		}

		const Mesh<Vertex>* getMesh() {
			return &chunkManager.mesh;
		}

		void render() {
			chunkManager.render();
		}
	private:
		ChunkManager chunkManager;
	};
}