#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <map>
#include <iostream>
#include <vector>
#include <memory>
#include <assert.h>

#include "VulkanDebug.h"
#include "VulkanBuffer.h"

#define LOGGING_LEVEL_1
#include "Logger.h"
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace VEngine{


	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> transferFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	class VulkanDevice
	{
	public:

		VkPhysicalDeviceFeatures deviceFeatures = {};

		void createDevice(VkInstance instance, VkSurfaceKHR* surface);

		void pickPhysicalDevice(VkInstance instance);
		bool isDeviceSuitable(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

		void createLogicalDevice();
	
		VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false, VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, VkCommandPool cmdPool = NULL);
		void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true, VkCommandPool cmdPool = NULL);

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		void destroy();

		VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VulkanBuffer* buffer, VkDeviceSize size, void* data = nullptr);
		VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
		void copyBuffer(VulkanBuffer* src, VulkanBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);

		VkCommandBuffer beginSingleTimeCommands(VkCommandPool cmdPool = NULL);
		void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmdPool = NULL, VkQueue queue = NULL);

		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		VkFormat findDepthFormat() {
			return findSupportedFormat(
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
		}

		bool hasStencilComponent(VkFormat format) {
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

		uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);

		VkSampleCountFlagBits getMaxUsableSampleCount();
		

		VkDevice getDevice() { return m_logicalDevice; };
		VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; };

		VkQueue getGraphicsQueue() { return m_graphicsQueue; };
		VkQueue getPresentQueue() { return m_presentQueue; };
		VkQueue getTransferQueue() { return m_transferQueue; };

		VkCommandPool getGraphicsCommandPool() { return m_graphicsCommandPool; };
		VkCommandPool getTransferCommandPool() { return m_transferCommandPool; };

		VkSampleCountFlagBits getMsaaSamples() {
			return m_msaaSamples;
		}



	private:
		VkPhysicalDevice m_physicalDevice;

		VkDevice m_logicalDevice;
		VkCommandPool m_graphicsCommandPool;
		VkCommandPool m_transferCommandPool;

		VkSurfaceKHR* m_surface;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkQueue m_transferQueue;

		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceFeatures m_features;
		VkPhysicalDeviceMemoryProperties m_memoryProperties;

		VkSampleCountFlagBits m_msaaSamples;

	};

}

