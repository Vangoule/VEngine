#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Component.h"
#include "VulkanModel.h"

namespace VEngine {



	class GraphicsComponent : public Component {
	public:
		//GraphicsComponent(std::vector < glm::vec3 > v) : m_vertices(v), m_indexed(false), m_visible(true) {}
		//GraphicsComponent(std::vector < glm::vec3 > v, std::vector<unsigned int> i) : m_vertices(v), m_indices(i), m_indexed(true), m_visible(true) {}
		//GraphicsComponent(std::vector < glm::vec3 > v, std::vector<unsigned int> i, Texture* texture) : m_vertices(v), m_indices(i), m_indexed(true), m_texture(texture), m_visible(true) {}
		//GraphicsComponent(std::vector < glm::vec3 > v, std::vector<unsigned int> i, std::vector < glm::vec2 > uvs, Texture* texture) : m_vertices(v), m_indices(i), m_indexed(true), m_texture(texture), m_uvs(uvs), m_visible(true) {}
		GraphicsComponent(std::string file) { filename = file; };
		GraphicsComponent(VulkanModelComponent* mdl) { model = mdl; };

		GraphicsComponent(Model* mdl) { model = new VulkanModelComponent(); model->model = mdl; };

		~GraphicsComponent() {};

		VulkanModelComponent* model;

		std::string filename;

	private:

	};

}