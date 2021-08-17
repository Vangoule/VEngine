#include "VulkanRenderer.h"

namespace VEngine {
	size_t currentFrame = 0;
	const int MAX_FRAMES_IN_FLIGHT = 2;

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	VertexLayout vertexLayout = VertexLayout({
		   VERTEX_COMPONENT_POSITION,
		   VERTEX_COMPONENT_COLOR,
		   VERTEX_COMPONENT_NORMAL,
		   VERTEX_COMPONENT_UV,
		});


	void VulkanRenderer::initVulkan(int width, int height) {
		LOG("Initializing Vulkan");
		createInstance();
		Debug::setupDebugMessenger(m_instance);
		
		createSurface();

		m_device = new VulkanDevice();
		m_device->createDevice(m_instance, &m_surface); 

		ModelCreateInfo modelCreateInfo(glm::vec3(4.0f), glm::vec3(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

		mdl.loadFromFile("resources/models/cube.obj", vertexLayout, &modelCreateInfo, m_device, m_device->getGraphicsQueue());

		m_swapChain = new VulkanSwapChain();
		m_swapChain->createSwapChain(m_device, m_surface, (uint32_t)width, (uint32_t)height );
		m_swapChain->createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createColorResources();
		createDepthResources();
		m_swapChain->createFramebuffers(m_renderPass, m_depthTexture, m_colorTexture);
		//createBuffers();
		createUniformBuffers();
			
		m_tex.loadFromFile("resources/textures/mosaic.png", VK_FORMAT_R8G8B8A8_UNORM, m_device, m_device->getTransferQueue(), VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();

		
		GLFWwindow* window = WindowManager::get().getHandle();
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(WindowManager::get().getHandle(), framebufferResizeCallback);
	}

	void VulkanRenderer::updateFrame(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo = {};
		//ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.view = glm::lookAt(glm::vec3(15.0f, 5.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), m_swapChain->getSwapChainExtent().width / (float)m_swapChain->getSwapChainExtent().height, 0.1f, 1000.0f);
		ubo.proj[1][1] *= -1;

		m_uniformBuffers[currentImage].map();
		m_uniformBuffers[currentImage].copyTo(&ubo, sizeof(ubo));
		m_uniformBuffers[currentImage].unmap();
	}


	void VulkanRenderer::updateCommandBuffers() {
		m_graphicsCommandBuffers.resize(m_swapChain->getSwapChainFramebuffers().size());

		for (size_t i = 0; i < m_graphicsCommandBuffers.size(); i++) {
			vkResetCommandBuffer(m_graphicsCommandBuffers[i], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBeginInfo.flags = NULL;

			vkBeginCommandBuffer(m_graphicsCommandBuffers[i], &cmdBufferBeginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_renderPass;
			renderPassInfo.framebuffer = m_swapChain->getSwapChainFramebuffers()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_graphicsCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			for (std::map<int, VulkanModelComponent*>::iterator it = m_models.begin(); it != m_models.end(); ++it)
			{
				vkCmdBindDescriptorSets(m_graphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);
				draw(it->second, m_graphicsCommandBuffers[i]);
			}

			vkCmdEndRenderPass(m_graphicsCommandBuffers[i]);

			if (vkEndCommandBuffer(m_graphicsCommandBuffers[i]) != VK_SUCCESS) {
				ELOG("Failed to record command buffer!");
				throw std::runtime_error("Failed to record command buffer!");
			}
		}
	}

	void VulkanRenderer::createCommandBuffers() {
		m_graphicsCommandBuffers.resize(m_swapChain->getSwapChainFramebuffers().size());

		for (size_t i = 0; i < m_graphicsCommandBuffers.size(); i++) {
			m_graphicsCommandBuffers[i] = m_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_renderPass;
			renderPassInfo.framebuffer = m_swapChain->getSwapChainFramebuffers()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(m_graphicsCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			for (std::map<int, VulkanModelComponent*>::iterator it = m_models.begin(); it != m_models.end(); ++it)
			{
				vkCmdBindDescriptorSets(m_graphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);
 				draw(it->second,m_graphicsCommandBuffers[i]);
			}

			vkCmdEndRenderPass(m_graphicsCommandBuffers[i]);

			if (vkEndCommandBuffer(m_graphicsCommandBuffers[i]) != VK_SUCCESS) {
				ELOG("Failed to record command buffer!");
				throw std::runtime_error("Failed to record command buffer!");
			}
		}
	}
	

	void VulkanRenderer::drawFrame()
	{
		vkWaitForFences(m_device->getDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

		/*Get the next image in the swap chain to render too*/
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_device->getDevice(), m_swapChain->getSwapChain(), std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		/*If image out of date, from resizing for example, then recreate the swapchain. */
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			refresh();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			ELOG("Failed to acquire swap chain image!");
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		updateFrame(imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_graphicsCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device->getDevice(), 1, &m_inFlightFences[currentFrame]);

		if (vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
			ELOG("Failed to sumbit draw command buffer!");
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_swapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(m_device->getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			refresh();
		}
		else if (result != VK_SUCCESS) {
			ELOG("Failed to present swap chain image!");
			throw std::runtime_error("Failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanRenderer::createBuffers() {

		//uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);
		//m_device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_stagingBuffer, vertexBufferSize, (void*)vertices.data());
		//m_device->createBuffer((VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_vertexBuffer, vertexBufferSize);
		//m_device->copyBuffer(&m_stagingBuffer, &m_vertexBuffer, m_device->getTransferQueue());

		//VulkanBuffer indexStagingBuffer;
		//uint32_t indicesBufferSize = static_cast<uint32_t>(indices.size()) * sizeof(uint16_t);
		//m_device->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexStagingBuffer, indicesBufferSize, (void*)indices.data());
		//m_device->createBuffer((VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_indexBuffer, indicesBufferSize);
		//m_device->copyBuffer(&indexStagingBuffer, &m_indexBuffer, m_device->getTransferQueue());
		//indexStagingBuffer.destroy();
	}

	void VulkanRenderer::createUniformBuffers()
	{
		VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);
		m_uniformBuffers.resize(m_swapChain->getSwapImages().size());

		for (size_t i = 0; i < m_swapChain->getSwapImages().size(); i++) {
			m_device->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_uniformBuffers[i], uniformBufferSize);
		}

	}

	void VulkanRenderer::createColorResources() {
		VkFormat colorFormat = m_swapChain->getSwapChainImageFormat();

		m_colorTexture.setDevice(m_device);
		m_colorTexture.createImage(m_swapChain->getSwapChainExtent().width, m_swapChain->getSwapChainExtent().height, 1, m_device->getMsaaSamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_colorTexture.getView() = m_colorTexture.createImageView(m_colorTexture.getImage(), colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		m_colorTexture.transitionImageLayout(m_colorTexture.getImage(), colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
	}

	void VulkanRenderer::createDepthResources()
	{
		m_depthFormat = m_device->findDepthFormat();
		m_depthTexture.setDevice(m_device);
		m_depthTexture.createImage(m_swapChain->getSwapChainExtent().width, m_swapChain->getSwapChainExtent().height, 1, m_device->getMsaaSamples(), m_depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_depthTexture.getView() = m_depthTexture.createImageView(m_depthTexture.getImage(), m_depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

		m_depthTexture.transitionImageLayout(m_depthTexture.getImage(), m_depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	}

	void VulkanRenderer::createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChain->getSwapImages().size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapChain->getSwapImages().size());

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(m_swapChain->getSwapImages().size());

		if (vkCreateDescriptorPool(m_device->getDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
			ELOG("Failed to create descriptor pool!");
			throw std::runtime_error("Failed to create descriptor pool!");
		}
	}

	void VulkanRenderer::createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		//Add all the descriptor sets together
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_device->getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
			ELOG("Failed to create descriptor set layout!");
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
	}

	void VulkanRenderer::createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(m_swapChain->getSwapImages().size(), m_descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChain->getSwapImages().size());
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(m_swapChain->getSwapImages().size());
		if (vkAllocateDescriptorSets(m_device->getDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
			ELOG("Failed to allocate descriptor sets!");
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < m_swapChain->getSwapImages().size(); i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = m_uniformBuffers[i].getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_tex.getView();
			imageInfo.sampler = m_tex.getSampler();

			std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = m_descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;


			vkUpdateDescriptorSets(m_device->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanRenderer::createSyncObjects() {
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_device->getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
				ELOG("Failed to create synchronization objects for a frame!");
				throw std::runtime_error("Failed to create synchronization objects for a frame!");
			}
		}
	}

	void VulkanRenderer::createRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_swapChain->getSwapChainImageFormat();
		colorAttachment.samples = m_device->getMsaaSamples();
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = m_device->findDepthFormat();
		depthAttachment.samples = m_device->getMsaaSamples();
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#
		VkAttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = m_swapChain->getSwapChainImageFormat();
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef = {};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device->getDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
			ELOG("Failed to create render pass!");
			throw std::runtime_error("Failed to create render pass!");
		}
	}

	void VulkanRenderer::createGraphicsPipeline()
	{
		auto vertShaderCode = readFile("resources/shaders/vert.spv");
		auto fragShaderCode = readFile("resources/shaders/frag.spv");

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.width = (float)m_swapChain->getSwapChainExtent().width;
		viewport.height = (float)m_swapChain->getSwapChainExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapChain->getSwapChainExtent();

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.minSampleShading = .0f;
		multisampling.rasterizationSamples = m_device->getMsaaSamples();

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;



		if (vkCreatePipelineLayout(m_device->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			ELOG("Failed to create pipeline layout!");
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &depthStencil;

		if (vkCreateGraphicsPipelines(m_device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
			ELOG("Failed to create graphics pipeline!");
			throw std::runtime_error("Failed to create graphics pipeline!");
		}


		vkDestroyShaderModule(m_device->getDevice(), fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device->getDevice(), vertShaderModule, nullptr);
	}

	VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			ELOG("Failed to create shader module!");
			throw std::runtime_error("Failed to create shader module!");
		}

		return shaderModule;
	}

	void VulkanRenderer::createSurface() {
		if (glfwCreateWindowSurface(m_instance, WindowManager::get().getHandle(), nullptr, &m_surface) != VK_SUCCESS) {
			ELOG("Failed to create window surface!");
			throw std::runtime_error("Failed to create window surface!");
		}
	}
	   	 
	std::vector<const char*> VulkanRenderer::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
	
	void VulkanRenderer::createInstance() {

		if (enableValidationLayers && !Debug::checkValidationLayerSupport()) {
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Game";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Vangoule's Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;


		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(Debug::validationLayers.size());
			createInfo.ppEnabledLayerNames = Debug::validationLayers.data();

			Debug::populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			ELOG("Failed to create a vulkan instance!");
			throw std::runtime_error("Failed to create vulkan instance!");
		}
	}

	void VulkanRenderer::refresh()
	{
		GLFWwindow* window = WindowManager::get().getHandle();
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_device->getDevice());

		cleanupSwapChain();

		m_swapChain->createSwapChain(m_device,m_surface, m_swapChain->getWidth(), m_swapChain->getHeight());
		m_swapChain->createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createColorResources();
		createDepthResources();
		m_swapChain->createFramebuffers(m_renderPass, m_depthTexture, m_colorTexture);
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
	}

	void VulkanRenderer::cleanupSwapChain()
	{
		m_colorTexture.destroy();
		m_depthTexture.destroy();	

		m_swapChain->destroy();

		vkFreeCommandBuffers(m_device->getDevice(), m_device->getGraphicsCommandPool(), static_cast<uint32_t>(m_graphicsCommandBuffers.size()), m_graphicsCommandBuffers.data());
		
		for (size_t i = 0; i < m_swapChain->getSwapImages().size(); i++) {
			m_uniformBuffers[i].destroy();
		}

		vkDestroyPipeline(m_device->getDevice(), m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_device->getDevice(), m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_device->getDevice(), m_renderPass, nullptr);

		vkDestroyDescriptorPool(m_device->getDevice(), m_descriptorPool, nullptr);
	}
	
	void VulkanRenderer::cleanup()
	{
		vkDeviceWaitIdle(m_device->getDevice());

		m_tex.destroy();
		mdl.destroy();
		cleanupSwapChain();	

		m_stagingBuffer.destroy();
	//	m_vertexBuffer.destroy();
		//m_indexBuffer.destroy();

		vkDestroyDescriptorSetLayout(m_device->getDevice(), m_descriptorSetLayout, nullptr);
		

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device->getDevice(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device->getDevice(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device->getDevice(), m_inFlightFences[i], nullptr);
		}				
				
		m_device->destroy();
		
		if (enableValidationLayers) {
			Debug::cleanDebugging(m_instance);
		}

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}



}