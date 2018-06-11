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
#include <optional>
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

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 texCoord;
		glm::vec3 normal;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, normal);
			return attributeDescriptions;
		}
	};

	class Block {
	private:
		bool mActive = true;

	public:
		static constexpr float CUBE_SIZE = 1.0f;
		float x;
		float y;
		float z;

		bool isActive() {
			return mActive;
		}
		void setActive(bool active) {
			mActive = active;
		}
	};
	
	static const float CUBE_SIZE = 1.0f;
	class Chunk {
	public:
		static const int CHUNK_SIZE = 16;

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

		void setActive(int x, int y, int z, bool active) {
			mBlocks[x][y][z].setActive(false);
		}

		Block getBlock(int x, int y, int z) {
			return mBlocks[x][y][z];
		}

		void render(Mesh<Vertex> &mesh, float xOffset, float yOffset, float zOffset) {
			static std::mt19937 engine;
			static std::uniform_int_distribution<> dist(3, 3);

			float cubeSize = CUBE_SIZE;
			for (int x = 0; x < CHUNK_SIZE; x++) {
				for (int y = 0; y < CHUNK_SIZE; y++) {
					for (int z = 0; z < CHUNK_SIZE; z++) {
						mBlocks[x][y][z].x = x * cubeSize + xOffset;
						mBlocks[x][y][z].y = y * cubeSize + yOffset;
						mBlocks[x][y][z].z = z * -cubeSize + zOffset;
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

							renderBlock(mesh, x * cubeSize + xOffset, y * cubeSize + yOffset, z * -cubeSize + zOffset, /*dist(engine)*/y % 3, cubeSize, renderLeft, renderFront, renderRight, renderBack, renderTop, renderBottom);
							//renderBlock(mesh, x * cubeSize + xOffset, y * cubeSize + yOffset, z * -cubeSize + zOffset, rand() % 2, cubeSize, true, true, true, true, true, true);
						}
					}
				}
			}
		}

	private:
		Block*** mBlocks;

		void renderBlock(Mesh<Vertex> &mesh, float x, float y, float z, float textureIndex, float w, bool renderLeft, bool renderFront, bool renderRight, bool renderBack, bool renderTop, bool renderBottom) {
			glm::vec3 p0 = {x, y, z};
			glm::vec3 p1 = {x + w, y, z};
			glm::vec3 p2 = {x + w, y + w, z};
			glm::vec3 p3 = {x, y + w, z};
			glm::vec3 p4 = {x, y, z - w};
			glm::vec3 p5 = {x ,y + w, z - w};
			glm::vec3 p6 = {x + w, y + w, z - w};
			glm::vec3 p7 = {x + w, y, z - w};
			uint32_t v0, v1, v2, v3;

			glm::vec3 normalF = {0, 0, 1};
			glm::vec3 normalB = { 0, 0, -1 }; 
			glm::vec3 normalL = { -1, 0, 0 };
			glm::vec3 normalR = { 1, 0, 0 };
			glm::vec3 normalU = { 0, 1, 0 };
			glm::vec3 normalD = { 0, -1, 0 };

			//Front;
			if (renderFront) {
				v0 = mesh.addVertex({ p0, { 2.0f, 1.0f, textureIndex }, normalF });
				v1 = mesh.addVertex({ p1, { 1.0f, 1.0f, textureIndex }, normalF });
				v2 = mesh.addVertex({ p2, { 1.0f, 0.0f, textureIndex }, normalF });
				v3 = mesh.addVertex({ p3, { 2.0f, 0.0f, textureIndex }, normalF });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Back
			if (renderBack) {
				v0 = mesh.addVertex({ p4, { 1.0f, 1.0f, textureIndex }, normalB });
				v1 = mesh.addVertex({ p5, { 1.0f, 0.0f, textureIndex }, normalB });
				v2 = mesh.addVertex({ p6, { 0.0f, 0.0f, textureIndex }, normalB });
				v3 = mesh.addVertex({ p7, { 0.0f, 1.0f, textureIndex }, normalB });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Top
			if (renderBottom) {
				v0 = mesh.addVertex({ p3, { 2.0f, 1.0f, textureIndex }, normalU });
				v1 = mesh.addVertex({ p2, { 1.0f, 1.0f, textureIndex }, normalU });
				v2 = mesh.addVertex({ p6, { 1.0f, 0.0f, textureIndex }, normalU });
				v3 = mesh.addVertex({ p5, { 2.0f, 0.0f, textureIndex }, normalU });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Bottom
			if (renderTop) {
				v0 = mesh.addVertex({ p1, { 1.0f, 0.0f, textureIndex }, normalD });
				v1 = mesh.addVertex({ p0, { 2.0f, 0.0f, textureIndex }, normalD });
				v2 = mesh.addVertex({ p4, { 2.0f, 1.0f, textureIndex }, normalD });
				v3 = mesh.addVertex({ p7, { 1.0f, 1.0f, textureIndex }, normalD });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Right
			if (renderRight) {
				v0 = mesh.addVertex({ p0,{ 1.0f, 1.0f, textureIndex }, normalR });
				v1 = mesh.addVertex({ p3,{ 1.0f, 0.0f, textureIndex }, normalR });
				v2 = mesh.addVertex({ p5,{ 0.0f, 0.0f, textureIndex }, normalR });
				v3 = mesh.addVertex({ p4,{ 0.0f, 1.0f, textureIndex }, normalR });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}

			//Left
			if (renderLeft) {
				v0 = mesh.addVertex({ p2, { 2.0f, 0.0f, textureIndex }, normalL });
				v1 = mesh.addVertex({ p1, { 2.0f, 1.0f, textureIndex }, normalL });
				v2 = mesh.addVertex({ p7, { 1.0f, 1.0f, textureIndex }, normalL });
				v3 = mesh.addVertex({ p6, { 1.0f, 0.0f, textureIndex }, normalL });
				mesh.addTriangle(v0, v1, v2);
				mesh.addTriangle(v2, v3, v0);
			}
		}
	};

	class HeightMap {
	public:
		int size;
		double* data;
		HeightMap(int size) {
			data = new double[size * size];
			memset(data, 0, sizeof(data[0]) * size * size);
			this->size = size;
			std::random_device realRandom;
			std::seed_seq seed{ realRandom(), realRandom(), realRandom(), realRandom(), realRandom(), realRandom(), realRandom(), realRandom() };
			engine = new std::mt19937(seed);
			dist = new std::uniform_real_distribution<>(-1.0f, 1.0f);

			//initialize the four corners
			set(0, 0, rand());
			set(0, size - 1, rand());
			set(size - 1, 0, rand());
			set(size - 1, size - 1, rand());
			//print();
		}

		~HeightMap() {
			delete[] data;
			delete dist;
			delete engine;
		}

		double get(int x, int z) {
			return data[(x % size + size) % size + ((z % size + size) % size) * size];
		}

		void set(int x, int z, double value) {
			data[(x % size + size) % size + ((z % size + size) % size) * size] = value;
		}

		void generate() {
			int step = size;
			double scale = 1.0;
			while (step > 1) {
				diamondSquare(step, scale);
				//print();
				step /= 2;
				scale /= 2.0;
			}
		}
		void print() {
			for (int z = 0; z < size; z++) {
				std::cout << "[ ";
				for (int x = 0; x < size - 1; x++) {
					std::cout << get(x, z) << ", ";
				}
				std::cout << get(size - 1, z) << " ]" << std::endl;
			}
		}

	private:
		std::mt19937* engine;
		std::uniform_real_distribution<>* dist;

		double rand() {
			return (*dist)(*engine);
		}

		void diamondSquare(int step, double scale) {
			int half = step / 2;
			for (int z = half; z < size + half; z += step) {
				for (int x = half; x < size + half; x += step) {
					square(x, z, step, scale);
				}
			}

			for (int z = 0; z < size; z += step) {
				for (int x = 0; x < size; x += step) {
					diamond(x + half, z, step, scale);
					diamond(x, z + half, step, scale);
				}
			}
		}

		void square(int x, int z, int step, double scale) {
			int half = step / 2;
			double a = get(x - half, z - half);
			double b = get(x + half, z - half);
			double c = get(x - half, z + half);
			double d = get(x + half, z + half);
			set(x, z, ((a + b + c + d) / 4.0) + rand() * scale);
		}

		void diamond(int x, int z, int step, double scale) {
			int half = step / 2;
			double a = get(x - half, z);
			double b = get(x + half, z);
			double c = get(x, z - half);
			double d = get(x, z + half);
			set(x, z, ((a + b + c + d) / 4.0) + rand() * scale);
		}
	};

	class ChunkManager {
	public:
		Mesh<Vertex> mesh;
		
		ChunkManager() {
			for (int i = 0; i < worldSize * worldSize * worldHeight; i++) {
				activeList.push_back(new Chunk());
			}
		}

		~ChunkManager() {
			for (int i = 0; i < activeList.size(); i++) {
				delete activeList[i];
			}
		}

		void render() {
			generateTerrain();
			float chunkRenderSize = CUBE_SIZE * Chunk::CHUNK_SIZE;
			float offset = worldSize / 2 * chunkRenderSize;
			for (int cz = 0; cz < worldSize; cz++) {
				for (int cx = 0; cx < worldSize; cx++) {
					for (int cy = 0; cy < worldHeight; cy++) {
						int chunkIndex = cx + cy * worldHeight + cz * worldSize * worldHeight;
						float xOffset = chunkRenderSize * cx - offset;
						float yOffset = chunkRenderSize * (cy - worldHeight);
						float zOffset = chunkRenderSize  *-cz + offset;
						Chunk* chunk = getChunk(cx, cy, cz);
						chunk->render(mesh, xOffset, yOffset, zOffset);
					}
				}
			}
			std::cout << "Vertex Count: " << mesh.vertices.size() << ", Triangles: " << mesh.indices.size() / 3 << std::endl;
		}

		std::optional<Block> getBlockAt(glm::vec3 position) {
			float chunkRenderSize = CUBE_SIZE * Chunk::CHUNK_SIZE;
			float offset = worldSize / 2 * chunkRenderSize;
			int cx = (position.x + offset) / chunkRenderSize;
			int cy = position.y / chunkRenderSize + worldHeight;
			int cz = ((-1.0 * position.z) + offset) / chunkRenderSize;

			Chunk* chunk = getChunk(cx, cy, cz);
			if (chunk == nullptr) {
				return {};
			}
			float xOffset = chunkRenderSize * cx -offset;
			float yOffset = chunkRenderSize * (cy - worldHeight);
			float zOffset = chunkRenderSize * -cz + offset;

			int bx = (position.x - xOffset) / CUBE_SIZE;
			int by = (position.y - yOffset) / CUBE_SIZE;
			int bz = -(position.z - zOffset) / CUBE_SIZE;
			//std::cout << "World (" << position.x << ", " << position.y << ", " << position.z << ") -> Chunk(" << cx << ", " << cy << ", " << cz << ") Block(" << bx << ", " << by << ", " << bz << ")" << std::endl;
			if (bx < 0 || by < 0 || bz < 0 || bx >= Chunk::CHUNK_SIZE || by >= Chunk::CHUNK_SIZE || bz >= Chunk::CHUNK_SIZE) {
				return {};
			}
			return std::optional<Block>{chunk->getBlock(bx, by, bz)};
		}

	private:
		int worldSize = 9;
		int worldHeight = 7;
		bool updateRequired = true;
		std::vector<Chunk*> activeList;
		std::vector<Chunk*> loadList;
		std::vector<Chunk*> unloadList;

		/*
		Generate terrian using diamond square algorithm
		*/
		void generateTerrain() {
			int n = static_cast<int>(ceilf(log2f(worldSize * worldSize)));
			int mapSize = pow(2, n) + 1;
			HeightMap heightMap(mapSize);
			heightMap.generate();

			int terrainHeight = Chunk::CHUNK_SIZE / 2 * worldHeight;
			for (int cz = 0; cz < worldSize; cz++) {
				for (int cx = 0; cx < worldSize; cx++) {
					for (int cy = 0; cy < worldHeight; cy++) {
						Chunk* chunk = getChunk(cx, cy, cz);
						for (int z = 0; z < Chunk::CHUNK_SIZE; z++) {
							for (int x = 0; x < Chunk::CHUNK_SIZE; x++) {
								double yscale = heightMap.get(cx*Chunk::CHUNK_SIZE + x, cz * Chunk::CHUNK_SIZE + z);
								int maxY = clamp(terrainHeight * yscale, -terrainHeight, terrainHeight) + terrainHeight;
								int minCy = maxY / Chunk::CHUNK_SIZE;
								int startY = 0; //default to setting whole chunk to air
								if (cy == minCy) {
									//equal to min chunk so may contain some blocks and some air
									startY = maxY - minCy * Chunk::CHUNK_SIZE;
								}
								else if (cy < minCy) {
									//under min chunck, so it contains blocks
									startY = Chunk::CHUNK_SIZE;
								}
								for (int y = startY; y < Chunk::CHUNK_SIZE; y++) {
									chunk->setActive(x, y, z, false);
								}
							}
						}
					}
				}
			}
		}

		Chunk* getChunk(int x, int y, int z) {
			if (x < 0 || y < 0 || z < 0) {
				return nullptr;
			}
			int index = (y * worldSize * worldSize) + (x * worldSize) + z;
			if (index < 0 || index >= activeList.size()) {
				return nullptr;
			}
			return activeList[index];
		}

		int clamp(int val, int min, int max) {
			if (val < min) {
				return min;
			}
			if (val > max) {
				return max;
			}
			return val;
		}
	};
}

#endif // !blok_chunk_manager_h