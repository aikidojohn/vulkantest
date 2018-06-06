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

		void updateCamera() {
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			startTime = std::chrono::high_resolution_clock::now();

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

			float previousY = cameraPos.y;
			//movenent in the z plane
			cameraPos += (Speed * timeDelta * direction.z) * playerFront;
			//movement in the x plane. cross creates a perpendicular vector to up and front
			cameraPos += glm::normalize(glm::cross(playerFront, cameraUp)) * (Speed * timeDelta * direction.x);
			//movement in the y plane. Simply add the delta y to the y component.
			cameraPos.y = previousY + (Speed * timeDelta * direction.y);

			//Collision Detecton
			playerPos = { cameraPos.x, cameraPos.y - 1, cameraPos.z };
			std::optional<Block> block = world.getBlockAt(playerPos);
			if (block.has_value()) {
				if (block->isActive()) {
					if (playerPos.y < block->y + CUBE_SIZE) {
						std::cout << "Player y Pos: " << playerPos.y << " < " << block->y + CUBE_SIZE << std::endl;
						direction.y = 0;
						playerPos.y = block->y + CUBE_SIZE;
						cameraPos.y = playerPos.y + 1;
					}
				}
			}
		}
	};

}; // !namespace blok
#endif // !blok_camera_h

