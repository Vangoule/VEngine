#include "VulkanBuffer.h"
namespace VEngine {
	VkResult VulkanBuffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(m_device, m_memory, offset, size, 0, &m_mapped);
	}

	void VulkanBuffer::unmap()
	{
		if (m_mapped)
		{
			vkUnmapMemory(m_device, m_memory);
			m_mapped = nullptr;
		}
	}

	VkResult VulkanBuffer::bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(m_device, m_buffer, m_memory, offset);
	}

	void VulkanBuffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		m_descriptor.offset = offset;
		m_descriptor.buffer = m_buffer;
		m_descriptor.range = size;
	}

	void VulkanBuffer::copyTo(void* data, VkDeviceSize size)
	{
		assert(m_mapped);
		memcpy(m_mapped, data, (size_t)size);
	}

	VkResult VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);
	}

	VkResult VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(m_device, 1, &mappedRange);
	}

	void VulkanBuffer::destroy()
	{
		if (m_buffer)
		{
			vkDestroyBuffer(m_device, m_buffer, nullptr);
		}
		if (m_memory)
		{
			vkFreeMemory(m_device, m_memory, nullptr);
		}
	}
}