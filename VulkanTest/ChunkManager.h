#ifndef blok_chunk_manager_h
#define blok_chunk_manager_h


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <vulkan/vulkan.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>
#include <array>
#include <chrono>
#include "Model.h"

namespace blok {

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, texCoord);
			return attributeDescriptions;
		}
	};

	class Block {
	private:
		bool mActive = true;

	public:
		bool isActive() {
			return mActive;
		}
		void setActive(bool active) {
			mActive = active;
		}
	};

	class Chunk {
	public:
		static const int CHUNK_SIZE = 16;
		Mesh<Vertex> mesh;

		Chunk() {
			mBlocks = new Block**[CHUNK_SIZE];
			for (int i = 0; i < CHUNK_SIZE; i++) {
				mBlocks[i] = new Block*[CHUNK_SIZE];
				for (int j = 0; j < CHUNK_SIZE; j++) {
					mBlocks[i][j] = new Block[CHUNK_SIZE];
				}
			}
		}

		~Chunk() {
			for (int i = 0; i < CHUNK_SIZE; i++) {
				for (int j = 0; j < CHUNK_SIZE; j++) {
					delete[] mBlocks[i][j];
				}
				delete[] mBlocks[i];
			}
			delete[] mBlocks;
		}

		void render() {
			mesh.clear();
			float cubeSize = 0.25f;
			float xOffset = 0.25f;
			float yOffset = 0.25f;
			for (int x = 0; x < CHUNK_SIZE; x++) {
				for (int y = 0; y < CHUNK_SIZE; y++) {
					for (int z = 0; z < CHUNK_SIZE; z++) {
						if (mBlocks[x][y][z].isActive()) {
							renderBlock(x * -cubeSize + xOffset, y * -cubeSize + yOffset, z * -cubeSize, rand() % 2, cubeSize);
						}
					}
				}
			}
		}

	private:
		Block * ** mBlocks;

		void renderBlock(float x, float y, float z, float textureIndex, float w) {
			float x0 = x;
			float y0 = y;
			float x1 = x + w;
			float y1 = y + w;
			uint32_t v0, v1, v2, v3;
			//Front;
			v0 = mesh.addVertex({ { x0, y0, z },{ 2.0f, 1.0f, textureIndex } });
			v1 = mesh.addVertex({ { x1, y0, z },{ 1.0f, 1.0f, textureIndex } });
			v2 = mesh.addVertex({ { x1, y1, z },{ 1.0f, 0.0f, textureIndex } });
			v3 = mesh.addVertex({ { x0, y1, z },{ 2.0f, 0.0f, textureIndex } });
			mesh.addTriangle(v0, v1, v2);
			mesh.addTriangle(v2, v3, v0);

			//Back
			v0 = mesh.addVertex({ { x0, y0, z - w },{ 1.0f, 1.0f, textureIndex } });
			v1 = mesh.addVertex({ { x0, y1, z - w },{ 1.0f, 0.0f, textureIndex } });
			v2 = mesh.addVertex({ { x1, y1, z - w },{ 0.0f, 0.0f, textureIndex } });
			v3 = mesh.addVertex({ { x1, y0, z - w },{ 0.0f, 1.0f, textureIndex } });
			mesh.addTriangle(v0, v1, v2);
			mesh.addTriangle(v2, v3, v0);

			//Top
			v0 = mesh.addVertex({ { x0, y1, z },{ 2.0f, 1.0f, textureIndex } });
			v1 = mesh.addVertex({ { x1, y1, z },{ 1.0f, 1.0f, textureIndex } });
			v2 = mesh.addVertex({ { x1, y1, z - w },{ 1.0f, 0.0f, textureIndex } });
			v3 = mesh.addVertex({ { x0, y1, z - w },{ 2.0f, 0.0f, textureIndex } });
			mesh.addTriangle(v0, v1, v2);
			mesh.addTriangle(v2, v3, v0);

			//Bottom
			v0 = mesh.addVertex({ { x1, y0, z },{ 1.0f, 0.0f, textureIndex } });
			v1 = mesh.addVertex({ { x0, y0, z },{ 2.0f, 0.0f, textureIndex } });
			v2 = mesh.addVertex({ { x0, y0, z - w },{ 2.0f, 1.0f, textureIndex } });
			v3 = mesh.addVertex({ { x1, y0, z - w },{ 1.0f, 1.0f, textureIndex } });
			mesh.addTriangle(v0, v1, v2);
			mesh.addTriangle(v2, v3, v0);

			//Left
			v0 = mesh.addVertex({ { x0, y0, z },{ 1.0f, 1.0f, textureIndex } });
			v1 = mesh.addVertex({ { x0, y1, z },{ 1.0f, 0.0f, textureIndex } });
			v2 = mesh.addVertex({ { x0, y1, z - w },{ 0.0f, 0.0f, textureIndex } });
			v3 = mesh.addVertex({ { x0, y0, z - w },{ 0.0f, 1.0f, textureIndex } });
			mesh.addTriangle(v0, v1, v2);
			mesh.addTriangle(v2, v3, v0);

			//Right
			v0 = mesh.addVertex({ { x1, y1, z },{ 2.0f, 0.0f, textureIndex } });
			v1 = mesh.addVertex({ { x1, y0, z },{ 2.0f, 1.0f, textureIndex } });
			v2 = mesh.addVertex({ { x1, y0, z - w },{ 1.0f, 1.0f, textureIndex } });
			v3 = mesh.addVertex({ { x1, y1, z - w },{ 1.0f, 0.0f, textureIndex } });
			mesh.addTriangle(v0, v1, v2);
			mesh.addTriangle(v2, v3, v0);
		}
	};
}

#endif // !blok_chunk_manager_h