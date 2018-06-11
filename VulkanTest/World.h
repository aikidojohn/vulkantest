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
		static constexpr float GRAVITY = 2.0f;
		static constexpr float TERMINAL_VELOCITY = 5.0f;

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