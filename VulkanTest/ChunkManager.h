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
			//carve a hole for testing face culling
			for (int x = 3; x < 7; x++) {
				for (int z = 0; z < 6; z++) {
					mBlocks[x][4][z].setActive(false);
					mBlocks[x][5][z].setActive(false);
					mBlocks[x+1][4][z].setActive(false);
					mBlocks[x+1][5][z].setActive(false);
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
							bool renderLeft = true;
							if (x < CHUNK_SIZE - 1)
								renderLeft = !mBlocks[x+1][y][z].isActive();

							bool renderRight = true;
							if (x > 0)
								renderRight = !mBlocks[x-1][y][z].isActive();

							bool renderFront = true; 
							if (z > 0)
								renderFront = !mBlocks[x][y][z-1].isActive();

							bool renderBack = true;
							if (z < CHUNK_SIZE - 1)
								renderBack = !mBlocks[x][y][z+1].isActive();

							bool renderTop = true;
							if (y > 0)
								renderTop = !mBlocks[x][y-1][z].isActive();

							bool renderBottom= true;
							if (y < CHUNK_SIZE - 1)
								renderBottom = !mBlocks[x][y+1][z].isActive();

							renderBlock(x * -cubeSize + xOffset, y * -cubeSize + yOffset, z * -cubeSize, rand() % 2, cubeSize, renderLeft, renderFront, renderRight, renderBack, renderTop, renderBottom);
						}
					}
				}
			}
		}

	private:
		Block * ** mBlocks;

		void renderBlock(float x, float y, float z, float textureIndex, float w, bool renderLeft, bool renderFront, bool renderRight, bool renderBack, bool renderTop, bool renderBottom) {
			float x0 = x;
			float y0 = y;
			float x1 = x + w;
			float y1 = y + w;
			uint32_t v0, v1, v2, v3;
			//Front;
			if (renderFront) {
				v0 = mesh.addVertex({ { x0, y0, z },{ 2.0f, 1.0f, textureIndex } });
				v1 = mesh.addVertex({ { x1, y0, z },{ 1.0f, 1.0f, textureIndex } });
				v2 = mesh.addVertex({ { x1, y1, z },{ 1.0f, 0.0f, textureIndex } });
				v3 = mesh.addVertex({ { x0, y1, z },{ 2.0f, 0.0f, textureIndex } });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Back
			if (renderBack) {
				v0 = mesh.addVertex({ { x0, y0, z - w },{ 1.0f, 1.0f, textureIndex } });
				v1 = mesh.addVertex({ { x0, y1, z - w },{ 1.0f, 0.0f, textureIndex } });
				v2 = mesh.addVertex({ { x1, y1, z - w },{ 0.0f, 0.0f, textureIndex } });
				v3 = mesh.addVertex({ { x1, y0, z - w },{ 0.0f, 1.0f, textureIndex } });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Top
			if (renderTop) {
				v0 = mesh.addVertex({ { x0, y1, z },{ 2.0f, 1.0f, textureIndex } });
				v1 = mesh.addVertex({ { x1, y1, z },{ 1.0f, 1.0f, textureIndex } });
				v2 = mesh.addVertex({ { x1, y1, z - w },{ 1.0f, 0.0f, textureIndex } });
				v3 = mesh.addVertex({ { x0, y1, z - w },{ 2.0f, 0.0f, textureIndex } });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Bottom
			if (renderBottom) {
				v0 = mesh.addVertex({ { x1, y0, z },{ 1.0f, 0.0f, textureIndex } });
				v1 = mesh.addVertex({ { x0, y0, z },{ 2.0f, 0.0f, textureIndex } });
				v2 = mesh.addVertex({ { x0, y0, z - w },{ 2.0f, 1.0f, textureIndex } });
				v3 = mesh.addVertex({ { x1, y0, z - w },{ 1.0f, 1.0f, textureIndex } });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Left
			if (renderLeft) {
				v0 = mesh.addVertex({ { x0, y0, z },{ 1.0f, 1.0f, textureIndex } });
				v1 = mesh.addVertex({ { x0, y1, z },{ 1.0f, 0.0f, textureIndex } });
				v2 = mesh.addVertex({ { x0, y1, z - w },{ 0.0f, 0.0f, textureIndex } });
				v3 = mesh.addVertex({ { x0, y0, z - w },{ 0.0f, 1.0f, textureIndex } });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Right
			if (renderRight) {
				v0 = mesh.addVertex({ { x1, y1, z },{ 2.0f, 0.0f, textureIndex } });
				v1 = mesh.addVertex({ { x1, y0, z },{ 2.0f, 1.0f, textureIndex } });
				v2 = mesh.addVertex({ { x1, y0, z - w },{ 1.0f, 1.0f, textureIndex } });
				v3 = mesh.addVertex({ { x1, y1, z - w },{ 1.0f, 0.0f, textureIndex } });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}
		}
	};
}

#endif // !blok_chunk_manager_h