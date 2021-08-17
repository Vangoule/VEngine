#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "Logger.h"
#define DEBUG true
namespace VEngine {

	namespace Debug {

		extern const std::vector<const char*> validationLayers;
		bool checkValidationLayerSupport();

		VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void destroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		void setupDebugMessenger(VkInstance instance);
		void cleanDebugging(VkInstance instance);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

 #ifdef DEBUG
			LOG(pCallbackData->pMessage);
 #endif
			return VK_FALSE;
		}



	}

}