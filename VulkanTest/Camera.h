#ifndef blok_camera_h
#define blok_camera_h

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include "ChunkManager.h"
#include "Player.h"
#include "World.h"
#include "Floats.h"

namespace blok {

	class Camera {
	public:
		float Zoom = 45.0f;
		float Speed = 7.0f;
		float Sensitivity = 0.05f;

		Camera(Player& aPlayer, World& aWorld) : player(aPlayer), world(aWorld) {
			glm::vec3 p = player.getPosition();
			cameraPos.x = p.x;
			cameraPos.y = p.y + 1;
			cameraPos.z = p.z;
			playerPos = { p.x, p.y, p.z };
			
		}

		~Camera() {
		}
		
		Camera& operator=(const Camera& other) {
			if (this == &other) {
				return *this;
			}
			world = other.world;
			player = other.player;
			cameraPos = other.cameraPos;
			cameraFront = other.cameraFront;
			cameraUp = other.cameraUp;
			playerPos = other.playerPos;
			playerFront = other.playerFront;
			yaw = other.yaw;
			pitch = other.pitch;
			return *this;
		}

		glm::mat4 getModelView() {
			updateCamera();
			return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		}

		glm::mat4 getSkyboxModelView() {
			return glm::lookAt(cameraPosSkybox, cameraPosSkybox + cameraFront, cameraUp);
		}

		void onMouseMove(float xOffset, float yOffset) {
			yaw += xOffset * Sensitivity;
			pitch = glm::clamp((pitch + yOffset * Sensitivity), -89.0f, 89.0f);
			updateFrontVectors();
		}

		void onKeyboardInput(glm::vec3 directionValues) {
			direction.x = directionValues.x;
			direction.y = directionValues.y;
			direction.z = directionValues.z;
		}

		void onKeyboardInput(float x, float y, float z) {
			direction.x = x;
			direction.y = y;
			direction.z = z;
			//std::cout << "direction " << glm::to_string(direction) << std::endl;
		}
		void onJumpKey() {
			if (!isInAir && !isJumping) {
				isJumping = true;
				flightTime = 0;
				direction.y = -1.0f;
			}
		}

		void setFlying(bool flightMode) {
			this->flightMode = flightMode;
			this->isInAir = !flightMode;
		}

	private:
		Player& player;
		World& world;
		glm::vec3 cameraPosSkybox = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cameraPos = glm::vec3(0.0f, 120.0f, 2.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 playerFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 playerPos;
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		float yaw = -90;
		float pitch = 0;
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
		float jumpVelocity = 8.0f;
		bool isJumping = false;
		bool isInAir = true;
		float flightTime = 0;
		bool flightMode = false;

		void updateFrontVectors() {
			//fix the player y position so player doesn't move toward y when looking up or down. Lock player in y plane until y is moved.
			//this is used for camera motion
			glm::vec3 playerf;
			playerf.x = cos(glm::radians(yaw));
			playerf.y = 1.0f;
			playerf.z = sin(glm::radians(yaw));

			//this is used for camera target
			glm::vec3 front;
			front.x = playerf.x * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = playerf.z * cos(glm::radians(pitch));
			cameraFront = glm::normalize(front);
			playerFront = glm::normalize(playerf);
			//std::cout << glm::to_string(cameraFront) << std::endl;
		}

		void updateCamera(float dx, float dy, float dz) {
			float previousY = cameraPos.y;
			//movenent in the z plane
			cameraPos += dz * playerFront;
			//movement in the x plane. cross creates a perpendicular vector to up and front
			cameraPos += glm::normalize(glm::cross(playerFront, cameraUp)) * dx;
			//movement in the y plane. Simply add the delta y to the y component.
			cameraPos.y = previousY + dy;

			playerPos = { cameraPos.x, cameraPos.y - 1, cameraPos.z };
		}

		glm::vec3 getCorrection(std::optional<Block> block, glm::vec3 pos, glm::vec3 dv) {
			float d = 1;
			if (abs(dv.z) > abs(dv.x)) {//moving toward z face
				d = dv.z < 0  ? (block->z - pos.z) / dv.z : (block->z - pos.z - Block::CUBE_SIZE) / dv.z;
			}
			else { //moving woard x face
				d = dv.x < 0 ? (block->x - pos.x + Block::CUBE_SIZE) / dv.x : (block->x - pos.x) / dv.x;
			}
			glm::vec3 newPos = d * dv + pos;
			glm::vec3 posDelta = newPos - pos;
			return posDelta;
		}

		void updateCamera() {
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			startTime = std::chrono::high_resolution_clock::now();

			float dx = Speed * timeDelta * direction.x;
			float dy = Speed * timeDelta * direction.y;
			float dz = Speed * timeDelta * direction.z;

			//Gravity calculations
			if (!flightMode && (isInAir || isJumping)) {
				flightTime += timeDelta;
				float dg = World::GRAVITY * flightTime * flightTime * 0.5f;
				if (dg > World::TERMINAL_VELOCITY) {
					dg = World::TERMINAL_VELOCITY;
				}
				float dp = isJumping ? jumpVelocity * timeDelta : 0.0f;
				dy = (dp - dg);
				std::cout << "In Air " << dg << ", " << dp << ", " << dy << std::endl;
			}
			//std::cout << glm::to_string(playerFront) << std::endl;
			updateCamera(dx, dy, dz);

			//Collision Detecton
			std::optional<Block> block = world.getBlockAt(playerPos);
			if (block.has_value()) {
				//determine if the player is standing on terrain. If the block below the current colliding block has a top y = player y and it is active, player is standing on the block.
				std::optional<Block> blockBelow = world.getBlockAt(playerPos.x, playerPos.y - Block::CUBE_SIZE, playerPos.z);
				bool onTerrain = blockBelow->isActive() && !block->isActive() ? floats::almostEqual(blockBelow->y + Block::CUBE_SIZE, playerPos.y) : false;
				if (block->isActive()) {

					//Downward Y collision correction
					float playerDeltaY = playerPos.y - (block->y + Block::CUBE_SIZE);
					if ((isInAir || isJumping || (direction.y < 0.0f && flightMode)) && playerDeltaY < 0.0f) {
						//std::cout << "Player y Pos: " << playerPos.y << " < " << block->y + Block::CUBE_SIZE << " delta : " << playerDeltaY << std::endl;
						direction.y = 0.0f;
						playerPos.y = block->y + Block::CUBE_SIZE;
						cameraPos.y = playerPos.y + Block::CUBE_SIZE;
						if (isJumping) {
							isJumping = false;
						}
						isInAir = false;
					}

					/*
					The playerFront is a unit vector representing the direction the player is facing. If we imagine taking the dot product
					of the player facing and each cube face normal, we will have a measure of how similar the two directions are. Since the normals are
					unit vectors with a single "1" value for an axis, the dot product is the same as looking at just the individual components of the player normal 
					(the dot product will just zero out the other components). So to figure out if the player is facing mostly in the x or z position,
					check the magnitude of the x and z components. The larger magnitued is the axis the player is aligned with.
					*/
					if (direction.x != 0 || direction.y != 0 || direction.z != 0) {
						//std::cout << "Player Pos: " << glm::to_string(playerPos) << " block: " << glm::to_string(glm::vec3(block->x, block->y, block->z)) << " direction: " << glm::to_string(direction) << " facing: " << glm::to_string(playerFront) << std::endl;

						//Line-Plane intersection equation https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
						//This will give a point on the block face to move the player to.
						glm::vec3  right = normalize(glm::cross(playerFront, cameraUp));
						glm::vec3 dv = direction.z != 0 ? playerFront * direction.z : right * direction.x;
						glm::vec3 posDelta = getCorrection(block, playerPos, dv);
						glm::vec3 newPlayerPos = playerPos + posDelta;
						std::optional<Block> newBlock = world.getBlockAt(newPlayerPos);
						if (newBlock.has_value() && newBlock->isActive() && floats::almostEqual(newBlock->z + 1, block->z)) {
							playerPos.x = playerPos.x + posDelta.x;
							//playerPos.z = playerPos.z + posDelta.z;
							cameraPos.x = playerPos.x;
							//cameraPos.z = playerPos.z;
							//std::cout << "New Player Pos: " << glm::to_string(playerPos) << std::endl;
						}
						else {
							if (abs(posDelta.x) < Block::CUBE_SIZE && abs(posDelta.y) < Block::CUBE_SIZE && abs(posDelta.z) < Block::CUBE_SIZE) { //Prevent teleporation Kyle! (why does this happen?)
								playerPos.x = playerPos.x + posDelta.x;
								playerPos.z = playerPos.z + posDelta.z;
								cameraPos.x = playerPos.x;
								cameraPos.z = playerPos.z;
								//std::cout << "New Player Pos: " << glm::to_string(playerPos) << std::endl;
							}
						}
					}
					//isInAir = false;
				}
				else {
					if (!onTerrain && !isInAir && !flightMode) {
						isInAir = true;
						flightTime = 0;
						direction.y = -1.0f;
					}
				}
			}
			else {
				if (!isInAir) {
					isInAir = true;
					flightTime = 0;
					direction.y = -1.0f;
				}
			}
		}
	};

}; // !namespace blok
#endif // !blok_camera_h

