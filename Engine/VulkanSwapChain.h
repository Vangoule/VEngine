#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include <array>

namespace VEngine {

	class VulkanSwapChain
	{
	public:
		void createSwapChain(VulkanDevice* device, VkSurfaceKHR surface, uint32_t width, uint32_t height);

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync = true);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		void createImageViews();
		void createFramebuffers(VkRenderPass renderPass, Texture2D depthTexture, Texture2D colorTexture);

		void destroy();
		void destroyFramebuffers();

		VkSwapchainKHR &getSwapChain() { return m_swapChain; };
		std::vector<VkImage> &getSwapImages() { return m_swapChainImages; };
		std::vector<VkImageView> &getSwapImageViews() { return m_swapChainImageViews; };
		VkExtent2D &getSwapChainExtent() { return m_swapChainExtent; };
		VkFormat &getSwapChainImageFormat() { return m_swapChainImageFormat; };
		std::vector<VkFramebuffer> &getSwapChainFramebuffers() { return m_swapChainFramebuffers; };

		void setWidth(int width) { m_width = width; };
		void setHeight(int height) { m_height = height; };

		int getWidth() { return m_width; };
		int getHeight() { return m_height; };
	private:
		VulkanDevice* m_device;
		VkSurfaceKHR m_surface;

		std::vector<VkImageView> m_swapChainImageViews;

		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;


		int m_width = 1920;
		int m_height = 1080;
	};


}