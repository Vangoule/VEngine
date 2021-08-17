#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include <set>
namespace VEngine {

	void VulkanDevice::createDevice(VkInstance instance, VkSurfaceKHR* surface)
	{
		m_surface = surface;
		pickPhysicalDevice(instance);
		createLogicalDevice();
	}

	void VulkanDevice::pickPhysicalDevice(VkInstance instance) {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			ELOG("Failed to find GPUs with Vulkan support!");
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				m_physicalDevice = device;
				m_msaaSamples = getMaxUsableSampleCount();
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE) {
			ELOG("Failed to find a suitable GPU!");
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;

		if (vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS) {
			ELOG("Failed to create command pool!");
			throw std::runtime_error("Failed to create command pool!");
		}

		return cmdPool;
	}

	VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin, VkCommandBufferUsageFlags flags, VkCommandPool cmdPool)
	{

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		if (cmdPool) {
			allocInfo.commandPool = cmdPool;
		}
		else
		{
			allocInfo.commandPool = m_graphicsCommandPool;
		}

		allocInfo.level = level;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;

		if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &cmdBuffer) != VK_SUCCESS) {
			ELOG("Failed to allocate command buffers!");
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBeginInfo.flags = flags;		

			vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);
		}

		return cmdBuffer;

	}

	void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free, VkCommandPool cmdPool)
	{
		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			ELOG("Failed to end command buffer!");
			throw std::runtime_error("Failed to end command buffer!");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;
		VkFence fence;

		if (vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
			ELOG("Failed to create fence!");
			throw std::runtime_error("Failed to create fence!");
		}

		vkQueueSubmit(queue, 1, &submitInfo, fence);

		vkWaitForFences(m_logicalDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

		vkDestroyFence(m_logicalDevice, fence, nullptr);

		if (free)
		{
			if (cmdPool) {
				vkFreeCommandBuffers(m_logicalDevice, cmdPool, 1, &commandBuffer);
			}
			else {
				vkFreeCommandBuffers(m_logicalDevice, m_graphicsCommandPool, 1, &commandBuffer);
			}
		}
	}

	bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
		
		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *m_surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, *m_surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, *m_surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, *m_surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, *m_surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void VulkanDevice::createLogicalDevice() {

		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

		deviceFeatures.samplerAnisotropy = VK_TRUE;
		//deviceFeatures.sampleRateShading = VK_TRUE;

		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

		float queuePriority = 0.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(Debug::validationLayers.size());
			createInfo.ppEnabledLayerNames = Debug::validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
			ELOG("Failed to create logical device!");
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
		vkGetDeviceQueue(m_logicalDevice, indices.transferFamily.value(), 0, &m_transferQueue);

		m_graphicsCommandPool = createCommandPool(indices.graphicsFamily.value());
		m_transferCommandPool = createCommandPool(indices.transferFamily.value());
	}

	VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size = size;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, buffer) != VK_SUCCESS) {
			ELOG("Failed to create buffer!");
			throw std::runtime_error("Failed to create buffer!");
		}

		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetBufferMemoryRequirements(m_logicalDevice, *buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		if (vkAllocateMemory(m_logicalDevice, &memAlloc, nullptr, memory) != VK_SUCCESS) {
			ELOG("Failed to allocate memory for buffer!");
			throw std::runtime_error("Failed to allocate memory for buffer!");
		}

		if (data != nullptr)
		{
			void* mapped;
			if (vkMapMemory(m_logicalDevice, *memory, 0, size, 0, &mapped) != VK_SUCCESS) {
				ELOG("Failed to map buffer!");
				throw std::runtime_error("Failed to map buffer!");
			}
			memcpy(mapped, data, (size_t)size);

			// If host coherency hasn't been requested, do a manual flush to make writes visible
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = {};
				mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(m_logicalDevice, 1, &mappedRange);
			}
			vkUnmapMemory(m_logicalDevice, *memory);
		}

		// Bind the memory to the buffer object
		if (vkBindBufferMemory(m_logicalDevice, *buffer, *memory, 0) != VK_SUCCESS) {
			ELOG("Failed to bind buffer!");
			throw std::runtime_error("Failed to bind buffer!");
		}

		return VK_SUCCESS;
	}

	VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VulkanBuffer* buffer, VkDeviceSize size, void* data)
	{
		buffer->setDevice(m_logicalDevice);

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size = size;

		if (vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, &buffer->getBuffer()) != VK_SUCCESS) {
			ELOG("Failed to create buffer!");
			throw std::runtime_error("Failed to create buffer!");
		}

		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = {};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetBufferMemoryRequirements(m_logicalDevice, buffer->getBuffer(), &memReqs);
		memAlloc.allocationSize = memReqs.size;

		memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		if (vkAllocateMemory(m_logicalDevice, &memAlloc, nullptr, &buffer->getMemory()) != VK_SUCCESS) {
			ELOG("Failed to allocate memory for buffer!");
			throw std::runtime_error("Failed to allocate memory for buffer!");
		}

		buffer->getAlignment() = memReqs.alignment;
		buffer->getSize() = memAlloc.allocationSize;
		buffer->getUsageFlags() = usageFlags;
		buffer->getMemoryPropertyFlags() = memoryPropertyFlags;

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			if (buffer->map() != VK_SUCCESS) {
				ELOG("Failed to map buffer!");
				throw std::runtime_error("Failed to map buffer!");
			}
			memcpy(buffer->getMapped(), data, (size_t)size);
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
				buffer->flush();

			buffer->unmap();
		}

		buffer->setupDescriptor();

		return buffer->bind();
	}

	void VulkanDevice::copyBuffer(VulkanBuffer* src, VulkanBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion)
	{
		assert(dst->getSize() <= src->getSize());
		assert(src->getBuffer());
		VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, m_transferCommandPool);
		VkBufferCopy bufferCopy{};
		if (copyRegion == nullptr)
		{
			bufferCopy.size = src->getSize();
		}
		else
		{
			bufferCopy = *copyRegion;
		}

		vkCmdCopyBuffer(copyCmd, src->getBuffer(), dst->getBuffer(), 1, &bufferCopy);

		flushCommandBuffer(copyCmd, queue, true, m_transferCommandPool);
	}

	uint32_t VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
	{
		for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
			typeBits >>= 1;
		}

		if (memTypeFound)
		{
			*memTypeFound = false;
			return 0;
		}
		else
		{
			ELOG("Could not find a matching memory type");
			throw std::runtime_error("Could not find a matching memory type");
		}
	}

	QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				if (i != indices.graphicsFamily)
				{
					indices.transferFamily = i;
				}
			}			
			
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *m_surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		//We might not get a dedicated transfer queue, if thats the case use the graphics queue
		if (!indices.transferFamily.has_value())
		{
			indices.transferFamily = indices.graphicsFamily;
		}

		return indices;
	}

	VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		ELOG("Failed to find supported format!");
		throw std::runtime_error("Failed to find supported format!");

		
	}

	VkCommandBuffer VulkanDevice::beginSingleTimeCommands(VkCommandPool cmdPool)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if (!cmdPool)
		{
			allocInfo.commandPool = m_graphicsCommandPool;
		}
		else {
			allocInfo.commandPool = cmdPool;
		}
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}
	void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer,VkCommandPool cmdPool, VkQueue queue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if (!queue) {
			vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_graphicsQueue);
		}
		else
		{
			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(queue);
		}

		if(!cmdPool)
			vkFreeCommandBuffers(m_logicalDevice, m_graphicsCommandPool, 1, &commandBuffer);
		else
			vkFreeCommandBuffers(m_logicalDevice, cmdPool, 1, &commandBuffer);
	}

	VkSampleCountFlagBits VulkanDevice::getMaxUsableSampleCount() {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = std::min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	void VulkanDevice::destroy()
	{
		vkDestroyCommandPool(m_logicalDevice, m_graphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_logicalDevice, m_transferCommandPool, nullptr);
		m_physicalDevice = nullptr;
		vkDestroyDevice(m_logicalDevice, nullptr);
		delete(this);
	}

}