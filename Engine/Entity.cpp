#include "Entity.h"

#include <iostream>
namespace VEngine {

	Entity::~Entity() {
		for (auto& c : m_components) {
			delete c.second;
		}
	}

}
