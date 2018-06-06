#ifndef blok_entity_h
#define blok_entity_h

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

namespace blok {
	class Entity {
	public:
		virtual glm::vec3 getPosition() = 0;
	};
}
#endif //!blok_entity_h