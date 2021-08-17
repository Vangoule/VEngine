#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "WindowManager.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <array>

#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanModel.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace VEngine {
#ifdef DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif
	extern VertexLayout vertexLayout;

	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			WLOG("Failed to open file!");
			throw std::runtime_error("Failed to open file!");
			exit(0);
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, color);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, texCoord);


			return attributeDescriptions;
		}
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	//static std::vector<Vertex> vertices = {

	//FRONT
	//{{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//BACK
	//{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//TOP
	//{{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{1.0f, 1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//BOTTOM
	//{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//RIGHT
	//{{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{1.0f, 1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//LEFT
	//{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{-1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-1.0f,  1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
	//};

	//const std::vector<uint16_t> indices = {
	//0,   1,  2,  2,  3,  0, //front
	//4,   5,  6,  6,  7,  4, //back

	//8,   9, 10, 10, 11,  8, //top
	//12, 13, 14, 14, 15, 12, //bottom

	//16, 17, 18, 18, 19,  16, //right
	//20, 21, 22, 22, 23, 20 //left
	//};

	//static std::vector<Vertex> vertices = {
	//	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	//};

	//const std::vector<uint16_t> indices = {
	//	0, 1, 2, 2, 3, 0,
	//	4, 5, 6, 6, 7, 4
	//};

	class VulkanRenderer
	{
	public:
		VulkanRenderer() { framebufferResized = false; sceneChanged = false; };

		void createInstance();
		void createSurface();

		std::vector<const char*> getRequiredExtensions();

		void initVulkan(int width, int height);
		void createGraphicsPipeline();
		void createRenderPass();
		void createCommandBuffers();
		void createSyncObjects();
		void createBuffers();
		void createUniformBuffers();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();
		void refresh();

		void updateCommandBuffers();

		void createColorResources();
		void createDepthResources();

		void updateFrame(uint32_t currentImage);
		void drawFrame();

		VulkanDevice* getDevice() { return m_device; };

		VkShaderModule createShaderModule(const std::vector<char>& code);

		void cleanupSwapChain();
		void cleanup();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
			LOG(pCallbackData->pMessage);

			return VK_FALSE;
		}
		
		bool framebufferResized;
		std::map<int, VulkanModelComponent*> m_models;

		void pushBackModel(int id, VulkanModelComponent* model) { sceneChanged = true;  model->pipeline = &m_graphicsPipeline; m_models.insert(std::pair<int, VulkanModelComponent*>(id, model)); };
		void removeModel(int id) { sceneChanged = true; m_models.erase(id); };

		bool sceneChanged;
	private:
		VulkanDevice* m_device;
		VulkanSwapChain* m_swapChain;
		VkSurfaceKHR m_surface;
		VkInstance m_instance;

		VkRenderPass m_renderPass;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_graphicsPipeline;

		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;


		std::vector<VkCommandBuffer> m_graphicsCommandBuffers;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;

	//	VulkanBuffer m_vertexBuffer;
	//	VulkanBuffer m_indexBuffer;
		VulkanBuffer m_stagingBuffer;
		std::vector<VulkanBuffer> m_uniformBuffers;

		Texture2D m_tex;
		VkFormat m_depthFormat;

		Texture2D m_colorTexture;
		Texture2D m_depthTexture;

		Model mdl;

	};
}