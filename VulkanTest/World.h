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
		static constexpr float GRAVITY = 0.25f;
		static constexpr float TERMINAL_VELOCITY = 0.50f;

		std::optional<Block> getBlockAt(glm::vec3 position) {
			return chunkManager.getBlockAt(position);
		}

		std::optional<Block> getBlockAt(float x, float y, float z) {
			return chunkManager.getBlockAt({ x, y, z });
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