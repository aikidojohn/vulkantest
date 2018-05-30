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
#include <queue>
#include <cstring>
#include <set>
#include <algorithm>
#include <array>
#include <chrono>
#include <random>
#include <cmath>
#include "Model.h"

namespace blok {

	struct diamond_params {
		int step;
		int x;
		int z;
		int mag;
	};

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
	
	static const float CUBE_SIZE = 0.25f;
	class Chunk {
	public:
		static const int CHUNK_SIZE = 16;
		//Mesh<Vertex> mesh;

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

		void setActive(int x, int y, int z, bool active) {
			mBlocks[x][y][z].setActive(false);
		}

		void render(Mesh<Vertex> &mesh, float xOffset, float yOffset, float zOffset) {
			float cubeSize = CUBE_SIZE;
			//float xOffset = 0.0f; //0.25f;
			//float yOffset = 0.0f; //0.25f;
			//float zOffset = 0.0f;
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

							renderBlock(mesh, x * -cubeSize + xOffset, y * -cubeSize + yOffset, z * -cubeSize + zOffset, rand() % 2, cubeSize, renderLeft, renderFront, renderRight, renderBack, renderTop, renderBottom);
						}
					}
				}
			}
		}

	private:
		Block*** mBlocks;

		void renderBlock(Mesh<Vertex> &mesh, float x, float y, float z, float textureIndex, float w, bool renderLeft, bool renderFront, bool renderRight, bool renderBack, bool renderTop, bool renderBottom) {
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

	class ChunkManager {
	public:
		Mesh<Vertex> mesh;
		
		ChunkManager() {
			for (int i = 0; i < worldSize * worldSize; i++) {
				activeList.push_back(new Chunk());
			}
		}

		~ChunkManager() {
			for (int i = 0; i < activeList.size(); i++) {
				delete activeList[i];
			}
		}

		void render() {
			diamondSquare();
			for (int i = 0; i < activeList.size(); i++) {
				activeList[i]->render(mesh, (float)(i/ worldSize) * Chunk::CHUNK_SIZE * CUBE_SIZE, 0.0f, (float)(i % worldSize) * Chunk::CHUNK_SIZE * CUBE_SIZE);
			}
			std::cout << "Vertex Count: " << mesh.vertices.size() << ", Triangles: " << mesh.indices.size() / 3 << std::endl;
		}

	private:
		int worldSize = 12;
		bool updateRequired = true;
		std::vector<Chunk*> activeList;
		std::vector<Chunk*> loadList;
		std::vector<Chunk*> unloadList;

		/*
		Generate terrian using diamond square algorithm
		*/
		void diamondSquare() {
			std::random_device realRandom;
			std::seed_seq seed{ realRandom(), realRandom(), realRandom(), realRandom(), realRandom(), realRandom(), realRandom(), realRandom() };
			std::mt19937 engine(seed);
			std::uniform_int_distribution<> rand(0, 16);
			
			int mag = 16;
			int size = 9;
			int steps = log2(9);

			int heightMap[9][9];
			memset(heightMap, 0, sizeof(heightMap[0][0]) * size * size);
			heightMap[0][0] = rand(engine);
			heightMap[0][size -1] = rand(engine);
			heightMap[size -1][0] = rand(engine);
			heightMap[size -1][size -1] = rand(engine);
			std::queue<diamond_params> queue;
			queue.push({ size, 0, 0, 16 });
			while (queue.size() > 0) {
				diamond_params p = queue.front();
				queue.pop();
				diamond(p.step, p.z, p.z, mag, heightMap);
				int next = p.step / 2;
				int nextMag = p.mag - (mag / steps) + 1;
				if (next > 1) {
					queue.push({ next + 1, p.x, p.z, nextMag });
					queue.push({ next + 1, p.x + next, p.z, nextMag });
					queue.push({ next + 1, p.x, p.z + next, nextMag });
					queue.push({ next + 1, p.x + next, p.z + next, nextMag });
				}
			}

			/*for (int cx = 0; cx < worldSize; cx++) {
				for (int cz = 0; cz < worldSize; cz++) {
					Chunk* chunk = activeList[cx * worldSize + cz];
					for (int x = 0; x < Chunk::CHUNK_SIZE; x++) {
						for (int z = 0; z < Chunk::CHUNK_SIZE; z++) {
							int maxY = heightMap[cx*Chunk::CHUNK_SIZE + x][cz*Chunk::CHUNK_SIZE + z];
							for (int y = maxY; y < Chunk::CHUNK_SIZE; y++) {
								chunk->setActive(x, y, z, false);
							}
						}
					}
				}
			}*/
		}

		void diamond(int step, int xOffset, int zOffset, int mag, int (&heightMap)[9][9]) {
			int mid = step / 2;
			int corner = step - 1;
			int size = 9;

			heightMap[xOffset + mid][zOffset + mid] = avg(
				heightMap[xOffset][zOffset], 
				heightMap[xOffset][zOffset + corner],
				heightMap[xOffset + corner][zOffset],
				heightMap[xOffset + corner][zOffset + corner]);

			//Square step
			int wrap = zOffset - mid < 0 ? zOffset - mid + size - 1 : zOffset - mid;
			heightMap[xOffset + mid][zOffset] = avg(
				heightMap[xOffset + mid][zOffset + mid],
				heightMap[xOffset][zOffset],
				heightMap[xOffset + corner][zOffset],
				heightMap[xOffset + mid][wrap]
			); 

			wrap = (zOffset + corner + mid) % size;
			heightMap[xOffset + mid][zOffset + corner] = avg(
				heightMap[xOffset + mid][zOffset + mid],
				heightMap[xOffset][zOffset + corner],
				heightMap[xOffset + corner][zOffset + corner],
				heightMap[xOffset + mid][wrap]
			);

			wrap = xOffset - mid < 0 ? xOffset - mid + size - 1 : xOffset - mid;
			heightMap[xOffset][zOffset + mid] = avg(
				heightMap[xOffset + mid][zOffset + mid],
				heightMap[xOffset][zOffset],
				heightMap[xOffset][zOffset + corner],
				heightMap[wrap][zOffset + mid]
			);

			wrap = (xOffset + corner + mid) % size;
			heightMap[xOffset + corner][zOffset + mid] = avg(
				heightMap[xOffset + mid][zOffset + mid],
				heightMap[xOffset + corner][zOffset],
				heightMap[xOffset + corner][zOffset + corner],
				heightMap[wrap][zOffset + mid]
			);
		}

		int avg(int a, int b, int c, int d) {
			return (int)std::round((float)(a + b + c + d) / 4.0f);
		}

		int avg(int a, int b, int c, int d, int e) {
			return (int)std::round((float)(a + b + c + d + e) / 5.0f);
		}
	};
}

#endif // !blok_chunk_manager_h