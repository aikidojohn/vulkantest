#ifndef blok_player_h
#define blok_player_h

#include "Entity.h"
#include "World.h"
namespace blok {
	class Player : public Entity {
	public:
		Player(World& aWorld, glm::vec3 aPostion, glm::vec3 aFront) : world(aWorld), position(aPostion), front(aFront) {}
		
		Player(const Player& other) : world(other.world), position(other.position), front(other.front) {}

		Player& operator=(const Player& other) {
			if (this == &other) {
				return *this;
			}

			world = other.world;
			position = other.position;
			front = other.front;
			return *this;
		}

		glm::vec3 getPosition() {
			return position;
		}

		BoundingBox getBoundingBox() {
			return {
				{-width, 0.0f, -width},
				{width, 0.0f, -width },
				{-width, 0.0f, width },
				{ width, 0.0f, width },
				{-width, height, -width },
				{ width, height, -width },
				{-width, height, width },
				{width, height, width },

			};
		}
		
		glm::vec3 getFront() {
			return front;
		}

		void setPosition(glm::vec3 position) {
			this->position = position;
		}

		void setFront(glm::vec3 front) {
			this->front = front;
		}

	private:
		glm::vec3 position;
		glm::vec3 front;
		World& world;
		float width = 0.25f;
		float height = 1.9f;
	};
}
#endif // !blok_player_h
