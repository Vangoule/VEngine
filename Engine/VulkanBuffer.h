#pragma once
#include <vulkan/vulkan.h>
#include <assert.h>
#include <memory>
namespace VEngine {
	class VulkanBuffer
	{
	public:
		VulkanBuffer(VkDevice device) {
			m_device = device;
			m_buffer = VK_NULL_HANDLE;
			m_memory = VK_NULL_HANDLE;
			m_mapped = nullptr;
		};

		VulkanBuffer() {
			m_buffer = VK_NULL_HANDLE;
			m_memory = VK_NULL_HANDLE;
			m_mapped = nullptr;
		};

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();

		VkResult bind(VkDeviceSize offset = 0);

		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void copyTo(void* data, VkDeviceSize size);

		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void destroy();

		VkDevice getDevice() { return m_device; };
		void setDevice(VkDevice device) { m_device = device; }

		VkBuffer& getBuffer() { return m_buffer; };
		VkBuffer& getMemory() { return m_memory; };

		void* getMapped() { return m_mapped; };

		VkDeviceSize& getSize() { return m_size; };
		VkDeviceSize& getAlignment() { return m_alignment; };

		VkBufferUsageFlags& getUsageFlags() { return m_usageFlags; };
		VkMemoryPropertyFlags& getMemoryPropertyFlags() { return m_memoryPropertyFlags; };
	private:

		VkDevice m_device;
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_memory = VK_NULL_HANDLE;

		VkDescriptorBufferInfo m_descriptor;
		VkDeviceSize m_size = 0;
		VkDeviceSize m_alignment = 0;

		VkBufferUsageFlags m_usageFlags;
		VkMemoryPropertyFlags m_memoryPropertyFlags;

		void* m_mapped;

	};
}
