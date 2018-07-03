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
			//playerPos = { p.x, p.y, p.z };
			player.setFront(glm::vec3(0.0f, 0.0f, -1.0f));
			
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
			//playerPos = other.playerPos;
			//playerFront = other.playerFront;
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
		//glm::vec3 playerFront = glm::vec3(0.0f, 0.0f, -1.0f);
		//glm::vec3 playerPos;
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
			player.setFront(glm::normalize(playerf));
			//std::cout << glm::to_string(cameraFront) << std::endl;
		}

		void updateCamera(float dx, float dy, float dz) {
			float previousY = cameraPos.y;
			//movenent in the z plane
			cameraPos += dz * player.getFront();
			//movement in the x plane. cross creates a perpendicular vector to up and front
			cameraPos += glm::normalize(glm::cross(player.getFront(), cameraUp)) * dx;
			//movement in the y plane. Simply add the delta y to the y component.
			cameraPos.y = previousY + dy;

			player.setPosition({ cameraPos.x, cameraPos.y - 1, cameraPos.z });
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

			glm::vec3 lastPosition = player.getPosition();
			updateCamera(dx, dy, dz);

			//Collision Detecton
			//correctCollision();
			correctCollision3(lastPosition);
		}

		std::optional<glm::vec3> getPlaneIntersection(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 lastPosition, glm::vec3 playerPosition) {
			//Find the point where the vector going from lastPosition to playerPosition intersects the plane defined by the points (p0,p1,p2)
			glm::vec3 norm = glm::cross(p1 - p0, p2 - p0); // normal vector for the plane
			glm::vec3 lab = (playerPosition - lastPosition);
			float t = glm::dot(norm, lastPosition - p0) / glm::dot(-1.0f * lab, norm);
			glm::vec3 labt = lab * t;
			glm::vec3 intersect = lastPosition + labt;

			// https://www.lucidar.me/en/mathematics/check-if-a-point-belongs-on-a-line-segment/
			float kab = glm::dot(lab, lab);
			float kac = glm::dot(lab, labt);
			if (kac >= 0 && kac <= kab) {
				return std::make_optional(intersect);
			}
			return {};
		}

		void correctCollision3(glm::vec3 lastCenter) {
			BoundingBox boxOffsets = player.getBoundingBox();
			glm::vec3* offsets = &boxOffsets.p0;
			glm::vec3 playerCenter = player.getPosition();

			
			bool onTerrain = false;
			for (int i = 0; i < 7; i++, offsets++) {
				glm::vec3 playerPos = playerCenter + *offsets;
				glm::vec3 lastPosition = lastCenter + *offsets;

				std::optional<Block> block = world.getBlockAt(playerPos);
				if (!block.has_value() || !block->isActive()) {
					//There is no collision. Player is outside of the world.
					//is player in the air?
					std::optional<Block> blockBelow = world.getBlockAt(block->x, block->y - 1, block->z);
					onTerrain |= blockBelow.has_value() && blockBelow->isActive() ? floats::almostEqual(block->y, playerPos.y) : false;
					continue;
				}

				//Did we intersect a visible top y plain of a block?
				std::optional<Block> blockAbove = world.getBlockAt(block->x, block->y + 1, block->z);
				if (blockAbove.has_value() && !blockAbove->isActive()) {
					//no block above, so it's visible. Check for intersection.
					glm::vec3 p0 = { block->x, block->y + 1, block->z };
					glm::vec3 p1 = { block->x + 1, block->y + 1, block->z };
					glm::vec3 p2 = { block->x, block->y + 1, block->z - 1 };
					/*glm::vec3 cross = glm::cross(p1 - p0, p2 - p0);
					glm::vec3 lab = (playerPos - lastPosition);
					float t = glm::dot(cross, lastPosition - p0) / glm::dot(-1.0f * lab , cross);
					glm::vec3 intersect = lastPosition + lab * t;
					*/
					std::optional<glm::vec3> intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
					if (intersect.has_value()) {
						std::cout << "y intersect " << glm::to_string(intersect.value()) << std::endl;
						//We have intersected the top face of the block. Set player position.
						playerCenter += intersect.value() - playerPos;
						//cameraPos = { intersect->x, intersect->y + 1, intersect->z };
						isInAir = false;
						isJumping = false;
						direction.y = 0.0f;
						continue; //Don't need to check anything else? Collided with top face of visible block;
					}
				}

				if (floats::almostEqual(playerPos.x, block->x) || floats::almostEqual(playerPos.x, block->x + 1.0f) || floats::almostEqual(playerPos.z, block->z) || floats::almostEqual(playerPos.z, block->z - 1.0f)) {
					//sliding on wall, return;
					continue;
				}

				glm::vec3 p0 = { block->x, block->y, block->z };
				glm::vec3 p1 = { block->x, block->y + 1, block->z };
				glm::vec3 p2 = { block->x, block->y, block->z - 1 };
				std::optional<glm::vec3> intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
				if (intersect.has_value()) {
					std::cout << "z, x intersect " << glm::to_string(intersect.value()) << std::endl;
					playerCenter += intersect.value() - playerPos;
					//cameraPos = { intersect->x, intersect->y + 1, intersect->z };
					direction.x = 0.0f;
					continue; //Don't need to check anything else?
				}

				p0 = { block->x + 1, block->y, block->z };
				p1 = { block->x + 1, block->y + 1, block->z };
				p2 = { block->x + 1, block->y, block->z - 1 };
				intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
				if (intersect.has_value()) {
					std::cout << "z, x+1 intersect " << glm::to_string(intersect.value()) << std::endl;
					playerCenter += intersect.value() - playerPos;
					//cameraPos = { intersect->x, intersect->y + 1, intersect->z };
					direction.x = 0.0f;
					continue; //Don't need to check anything else?
				}

				p0 = { block->x, block->y, block->z };
				p1 = { block->x, block->y + 1, block->z };
				p2 = { block->x + 1, block->y, block->z };
				intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
				if (intersect.has_value()) {
					std::cout << "x, z intersect " << glm::to_string(intersect.value()) << std::endl;
					playerCenter += intersect.value() - playerPos;
					//cameraPos = { intersect->x, intersect->y + 1, intersect->z };
					direction.x = 0.0f;
					continue; //Don't need to check anything else?
				}

				p0 = { block->x, block->y, block->z - 1 };
				p1 = { block->x, block->y + 1, block->z - 1 };
				p2 = { block->x + 1, block->y, block->z - 1 };
				intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
				if (intersect.has_value()) {
					std::cout << "x, z-1 intersect " << glm::to_string(intersect.value()) << std::endl;
					playerCenter += intersect.value() - playerPos;
					//cameraPos = { intersect->x, intersect->y + 1, intersect->z };
					direction.x = 0.0f;
					continue; //Don't need to check anything else?
				}
			}

			if (!isInAir && !onTerrain) {
				isInAir = true;
				flightTime = 0;
				direction.y = -1.0f;
			}

			player.setPosition(playerCenter);
			cameraPos = { playerCenter.x, playerCenter.y + 1, playerCenter.z };
		}

		void correctCollision2(glm::vec3 lastPosition) {
			glm::vec3 playerPos = player.getPosition();
			std::optional<Block> block = world.getBlockAt(playerPos);
			if (!block.has_value() || !block->isActive()) {
				//There is no collision. Player is outside of the world.
				//is player in the air?
				std::optional<Block> blockBelow = world.getBlockAt(block->x, block->y - 1, block->z);
				bool onTerrain = blockBelow.has_value() && blockBelow->isActive() ? floats::almostEqual(block->y, playerPos.y) : false;
				if (!isInAir && !onTerrain) {
					isInAir = true;
					flightTime = 0;
					direction.y = -1.0f;
				}
				return;
			}

			//Did we intersect a visible top y plain of a block?
			std::optional<Block> blockAbove = world.getBlockAt(block->x, block->y + 1, block->z);
			if (blockAbove.has_value() && !blockAbove->isActive()) {
				//no block above, so it's visible. Check for intersection.
				glm::vec3 p0 = { block->x, block->y + 1, block->z };
				glm::vec3 p1 = { block->x + 1, block->y + 1, block->z };
				glm::vec3 p2 = { block->x, block->y + 1, block->z - 1 };
				/*glm::vec3 cross = glm::cross(p1 - p0, p2 - p0);
				glm::vec3 lab = (playerPos - lastPosition);
				float t = glm::dot(cross, lastPosition - p0) / glm::dot(-1.0f * lab , cross);
				glm::vec3 intersect = lastPosition + lab * t;
				*/
				std::optional<glm::vec3> intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
				if (intersect.has_value()) {
					//We have intersected the top face of the block. Set player position.
					player.setPosition(intersect.value());
					cameraPos = { intersect->x, intersect->y + 1, intersect->z };
					isInAir = false;
					isJumping = false;
					direction.y = 0.0f;
					return; //Don't need to check anything else? Collided with top face of visible block;
				}
			}

			if (floats::almostEqual(playerPos.x, block->x) || floats::almostEqual(playerPos.x, block->x + 1.0f) || floats::almostEqual(playerPos.z, block->z) || floats::almostEqual(playerPos.z, block->z - 1.0f)) {
				//sliding on wall, return;
				return;
			}

			glm::vec3 p0 = { block->x, block->y, block->z };
			glm::vec3 p1 = { block->x, block->y + 1, block->z };
			glm::vec3 p2 = { block->x, block->y, block->z - 1 };
			std::optional<glm::vec3> intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
			if (intersect.has_value()) {
				player.setPosition(intersect.value());
				cameraPos = { intersect->x, intersect->y + 1, intersect->z };
				direction.x = 0.0f;
				return; //Don't need to check anything else?
			}

			p0 = { block->x + 1, block->y, block->z };
			p1 = { block->x + 1, block->y + 1, block->z };
			p2 = { block->x + 1, block->y, block->z - 1 };
			intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
			if (intersect.has_value()) {
				player.setPosition(intersect.value());
				cameraPos = { intersect->x, intersect->y + 1, intersect->z };
				direction.x = 0.0f;
				return; //Don't need to check anything else?
			}

			p0 = { block->x, block->y, block->z };
			p1 = { block->x, block->y + 1, block->z };
			p2 = { block->x + 1, block->y, block->z };
			intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
			if (intersect.has_value()) {
				player.setPosition(intersect.value());
				cameraPos = { intersect->x, intersect->y + 1, intersect->z };
				direction.x = 0.0f;
				return; //Don't need to check anything else?
			}

			p0 = { block->x, block->y, block->z -1 };
			p1 = { block->x, block->y + 1, block->z -1 };
			p2 = { block->x + 1, block->y, block->z - 1 };
			intersect = getPlaneIntersection(p0, p1, p2, lastPosition, playerPos);
			if (intersect.has_value()) {
				player.setPosition(intersect.value());
				cameraPos = { intersect->x, intersect->y + 1, intersect->z };
				direction.x = 0.0f;
				return; //Don't need to check anything else?
			}
		}

		void correctCollision() {
			glm::vec3 playerPos = player.getPosition();
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
						glm::vec3  right = normalize(glm::cross(player.getFront(), cameraUp));
						glm::vec3 dv = direction.z != 0 ? player.getFront() * direction.z : right * direction.x;
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
					player.setPosition(playerPos);
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

