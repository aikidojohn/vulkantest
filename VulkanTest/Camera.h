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

namespace blok {

	class Camera {
	public:
		float Zoom = 45.0f;
		float Speed = 1.5f;
		float Sensitivity = 0.05f;
		
		
		glm::mat4 getModelView() {
			updateCamera();
			return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
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
		glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 2.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 playerFront = glm::vec3(0.0f, 0.0f, -1.0f);
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
			glm::vec3 player;
			player.x = cos(glm::radians(yaw));
			player.y = 1.0f;
			player.z = sin(glm::radians(yaw));

			//this is used for camera target
			glm::vec3 front;
			front.x = player.x * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = player.z * cos(glm::radians(pitch));
			cameraFront = glm::normalize(front);
			playerFront = glm::normalize(player);
			//std::cout << glm::to_string(cameraFront) << std::endl;

			float previousY = cameraPos.y;
			//movenent in the z plane
			cameraPos += (Speed * timeDelta * direction.z) * playerFront;
			//movement in the x plane. cross creates a perpendicular vector to up and front
			cameraPos += glm::normalize(glm::cross(playerFront, cameraUp)) * (Speed * timeDelta * direction.x);
			//movement in the y plane. Simply add the delta y to the y component.
			cameraPos.y = previousY + (Speed * timeDelta * direction.y);
		}
	};

}; // !namespace blok
#endif // !blok_camera_h

