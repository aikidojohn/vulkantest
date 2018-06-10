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

namespace blok {

	class Camera {
	public:
		float Zoom = 45.0f;
		float Speed = 3.5f;
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

	private:
		Player& player;
		World& world;
		glm::vec3 cameraPosSkybox = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 2.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 playerFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 playerPos;
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		float yaw = -90;
		float pitch = 0;
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);

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

		void updateCamera() {
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			startTime = std::chrono::high_resolution_clock::now();

			float dx = Speed * timeDelta * direction.x;
			float dy = Speed * timeDelta * direction.y;
			float dz = Speed * timeDelta * direction.z;
			//std::cout << glm::to_string(playerFront) << std::endl;
			updateCamera(dx, dy, dz);

			//Collision Detecton
			std::optional<Block> block = world.getBlockAt(playerPos);
			if (block.has_value()) {
				if (block->isActive()) {
					float playerDeltaY = playerPos.y - (block->y + CUBE_SIZE);
					float playerDeltaX = playerPos.x - (block->x + CUBE_SIZE);
					float playerDeltaZ = playerPos.z - block->z;
					glm::vec3  right = normalize(glm::cross(playerFront, cameraUp));
					glm::vec3  left = normalize(glm::cross(playerFront, glm::vec3(0.0f, -1.0f, 0.0f)));
					if (direction.y < 0 && playerDeltaY < 0) {
						std::cout << "Player y Pos: " << playerPos.y << " < " << block->y + CUBE_SIZE << " delta : " << playerDeltaY << std::endl;
						direction.y = 0;
						playerPos.y = block->y + CUBE_SIZE;
						cameraPos.y = playerPos.y + 1;
					}
					/*if (direction.x < 0 && playerDeltaX < 0) {
						std::cout << "Player x Pos: " << playerPos.x << " < " << block->x + CUBE_SIZE << " delta : " << playerDeltaX << std::endl;
						direction.x = 0;
						playerPos.x = block->x + CUBE_SIZE;
						cameraPos.x = playerPos.x;
					//	glm::vec3  norm = normalize(glm::cross(playerFront, cameraUp));
					//	float newX = (block->x + CUBE_SIZE - cameraPos.x) / norm.x;
					//	updateCamera(newX, 0, 0);
						std::cout << "New Player x Pos: " << playerPos.x << std::endl;
					}*/
					/*
					The playerFront is a unit vector representing the direction the player is facing. If we imagine taking the dot product
					of the player facing and each cube face normal, we will have a measure of how similar the two directions are. Since the normals are
					unit vectors with a single "1" value for an axis, the dot product is the same as looking at just the individual components of the player normal 
					(the dot product will just zero out the other components). So to figure out if the player is facing mostly in the x or z position,
					check the magnitude of the x and z components. The larger magnitued is the axis the player is aligned with.
					*/
					if (direction.x != 0 ||  direction.y != 0 || direction.z != 0) 
						std::cout << "Player Pos: " << glm::to_string(playerPos) << " block: " << glm::to_string(glm::vec3(block->x, block->y, block->z)) << " direction: " << glm::to_string(direction) << " facing: " << glm::to_string(playerFront) << std::endl;

					//Line-Plane intersection equation https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
					//This will give a point on the block face to move the player to.
					//get block face normal based on direction player is facing
					//glm::vec3 n = playerFront.x > 0 ? glm::vec3(-1, 0, 0) : glm::vec3(1, 0, 0);
					//float d = glm::dot(glm::vec3(block->x, block->y, block->z) - playerPos, n) / glm::dot(playerFront, n);


					if (fabs(playerFront.z) >= 0.5f) { //Player facing Z direction
						if ((direction.z > 0 && playerFront.z < 0) || (direction.z < 0 && playerFront.z > 0)) {
							direction.z = 0;
							//playerPos.z = block->z;
							//cameraPos.z = playerPos.z;
							//float newZ = (block->z - cameraPos.z) / playerFront.z;
							//updateCamera(0, 0, newZ);
							float d = (block->z - playerPos.z) / playerFront.z;
							playerPos = d * playerFront + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "1 New Player z Pos: " << playerPos.z << std::endl;
						}

						if ((direction.z > 0 && playerFront.z > 0) || (direction.z < 0 && playerFront.z < 0)) {
							direction.z = 0;
							//playerPos.z = block->z - CUBE_SIZE;
							//cameraPos.z = playerPos.z;
							//float newZ = (block->z - cameraPos.z - CUBE_SIZE) / playerFront.z;
							//updateCamera(0, 0, newZ);
							float d = (block->z - playerPos.z - CUBE_SIZE) / playerFront.z;
							playerPos = d * playerFront + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "2 New Player z Pos: " << playerPos.z << std::endl;
						}

						if ((direction.x > 0 && playerFront.z > 0) || (direction.x < 0 && playerFront.z < 0)) {
							direction.x = 0;
							//playerPos.x = block->x + CUBE_SIZE;
							//cameraPos.x = playerPos.x;
							//float newX = (block->x + CUBE_SIZE - cameraPos.x) / right.x;
							//updateCamera(newX, 0, 0);
							float d = (block->x + CUBE_SIZE - playerPos.x) / right.x;
							playerPos = d * right + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "3 New Player x Pos: " << playerPos.x << std::endl;
						}

						if ((direction.x > 0 && playerFront.z < 0) || (direction.x < 0 && playerFront.z > 0)) {
							direction.x = 0;
							//playerPos.x = block->x;
							//cameraPos.x = playerPos.x;
							//float newX = (block->x - cameraPos.x) / right.x;
							//updateCamera(newX, 0, 0);
							float d = (block->x - playerPos.x) / right.x;
							playerPos = d * right + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "4 New Player x Pos: " << playerPos.x << std::endl;
						}
						std::cout << "New Player Pos: " << glm::to_string(playerPos) << std::endl;
					}
					else { //Player facing X direction
						if ((direction.z > 0 && playerFront.x < 0) || (direction.z < 0 && playerFront.x > 0)) {
							direction.z = 0;
							//playerPos.x = block->x + CUBE_SIZE;
							//cameraPos.x = playerPos.x;
							//float newX = (block->x + CUBE_SIZE - cameraPos.x) / right.x;
							//updateCamera(newX, 0, 0);
							float d = (block->x + CUBE_SIZE - playerPos.x) / playerFront.x;
							playerPos = d * playerFront + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "5 New Player x Pos: " << playerPos.x << std::endl;
						}

						if ((direction.z > 0 && playerFront.x > 0) || (direction.z < 0 && playerFront.x < 0)) {
							direction.z = 0;
							//playerPos.x = block->x;
							//cameraPos.x = playerPos.x;
							//float newX = (block->x - cameraPos.x) / right.x;
							//updateCamera(newX, 0, 0);
							float d = (block->x - playerPos.x) / playerFront.x;
							playerPos = d * playerFront + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "6 New Player x Pos: " << playerPos.x << std::endl;
						}

						if ((direction.x > 0 && playerFront.x < 0) || (direction.x < 0 && playerFront.x > 0)) {
							direction.x = 0;
							//playerPos.z = block->z;
							//cameraPos.z = playerPos.z;
							//float newZ = (block->z - cameraPos.z) / playerFront.z;
							//updateCamera(0, 0, newZ);
							float d = (block->z - playerPos.z) / right.z;
							playerPos = d * right + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "7 New Player z Pos: " << playerPos.z << std::endl;
						}

						if ((direction.x > 0 && playerFront.x > 0) || (direction.x < 0 && playerFront.x < 0)) {
							direction.x = 0;
							//playerPos.z = block->z - CUBE_SIZE;
							//cameraPos.z = playerPos.z;
							//float newZ = (block->z - cameraPos.z - CUBE_SIZE) / playerFront.z;
							//updateCamera(0, 0, newZ);
							float d = (block->z - playerPos.z - CUBE_SIZE) / right.z;
							playerPos = d * right + playerPos;
							cameraPos.x = playerPos.x;
							cameraPos.z = playerPos.z;
							std::cout << "8 New Player z Pos: " << playerPos.z << std::endl;
						}
						std::cout << "New Player Pos: " << glm::to_string(playerPos) << std::endl;
					}

					
				}
			}
		}
	};

}; // !namespace blok
#endif // !blok_camera_h

