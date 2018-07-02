#ifndef blok_entity_h
#define blok_entity_h

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

namespace blok {
	struct BoundingBox {
		glm::vec3 p0;
		glm::vec3 p1;
		glm::vec3 p2;
		glm::vec3 p3;
		glm::vec3 p4;
		glm::vec3 p5;
		glm::vec3 p6;
		glm::vec3 p7;
	};

	class Entity {
	public:
		virtual glm::vec3 getPosition() = 0;
		virtual BoundingBox getBoundingBox() = 0;
	};
}
#endif //!blok_entity_h