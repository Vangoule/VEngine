#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Component.h"


namespace VEngine {

	class GraphicsComponent : public Component {
	public:
		GraphicsComponent() {};

		void setPosition(const glm::vec3 position) {
			this->m_pos = position;
		}

		glm::vec3 getPosition() const {
			return m_pos;
		}

	private:
		glm::vec3 m_pos;
	};

}