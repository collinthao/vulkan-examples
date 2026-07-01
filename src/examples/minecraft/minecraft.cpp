#include "./minecraft.h"
#include <thread>
#include <cmath>
#include <algorithm>
#include "./debugUtils/debugUtils.h"

void Minecraft::init(GLFWwindow* window)
{
	createInstance();
	setupDebugMessenger();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain(window);
	createImageViews();
	createRenderPass();
	createShadowMapRenderPass();
	createPostProcessingRenderPass();
	createPipelines();
	CommandBuffer::createCommandPool(physicalDevice, device, surface);
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createCubeTextureImage(GRASS_BLOCK_PATH, cubemapImage, cubemapImageMemory);
	createCubeMapResources();
	Image::Texture::Sampler::createTextureSampler(textureSampler, device, physicalDevice, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	createInstanceBuffers(glm::vec3{camera.cameraPos.x, 0., camera.cameraPos.z});
	createVertexBuffers();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorSets();
	CommandBuffer::createCommandBuffers(device);
	createSyncObjects();
}

void Minecraft::createVertexBuffers()
{
	createVertexBuffer(cubeVertices, vertexCubeBuffer, vertexCubeBufferMemory);
	createVertexBuffer(cubemapVertices, vertexCubemapBuffer, vertexCubemapBufferMemory);
}

void Minecraft::createDescriptorSets()
{
	createGraphicsDescriptorSets();
	createCubemapDescriptorSets();
};

void Minecraft::cleanup(GLFWwindow * window)
{
	cleanupSwapChain();

	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
	
	vkDestroyBuffer(device, vertexCubeBuffer, nullptr);
	vkFreeMemory(device, vertexCubeBufferMemory, nullptr);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, CommandBuffer::commandPool, nullptr);

	vkDestroyRenderPass(device, renderPasses.renderPass, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
};


void Minecraft::createUniformBuffers()
{
	createGraphicsUniformBuffers();
	createCubemapUniformBuffers();
}
void Minecraft::processInput(GLFWwindow * window)
{
	camera.cameraSpeed = 10.f * lastFrameTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.move(FORWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.move(BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.move(LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.move(RIGHT);
};

void Minecraft::createCubeMapResources()
{
	cubemapImageView = Image::createView(cubemapImage, cubemapImageView, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, 6, device, graphicsAndComputeQueue);
}

void Minecraft::createCubeTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory)
{
	std::vector<std::string> faces
	{
	    "right.jpg",
	    "left.jpg",
	    "top.jpg",
	    "bottom.jpg",
	    "front.jpg",
	    "back.jpg"
	};

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels[6];

	for (size_t i = 0; i < faces.size(); i++)
	{
		Image::Texture::loadTexture(imagePath + faces[i], texWidth, texHeight, texChannels, 6, pixels[i]);
	}
	
	VkDeviceSize imageSize = texWidth * texHeight * 4 * 6;
	VkDeviceSize layerSize =  imageSize / 6;

	Image::mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);

	for (size_t i = 0; i < 6; i++)
	{
		memcpy(static_cast<char*>(data) + (layerSize * i), pixels[i], static_cast<size_t>(layerSize));
		stbi_image_free(pixels[i]);
	}

	vkUnmapMemory(device, stagingBufferMemory);

	Image::create(texWidth, 
			texHeight, 
			Image::mipLevels,
			6,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, 
			VK_IMAGE_TYPE_2D,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			image, 
			imageMemory,
			VK_IMAGE_LAYOUT_UNDEFINED,
			device,
			physicalDevice);

	Image::transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Image::mipLevels, 6, device, graphicsAndComputeQueue, VK_IMAGE_ASPECT_COLOR_BIT);
	Image::copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 6, device, graphicsAndComputeQueue);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	Image::generateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, Image::mipLevels, 6, device, physicalDevice, graphicsAndComputeQueue);
}

void Minecraft::createCubemapUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObjectModel);

	cubemapUniformBuffers.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	cubemapUniformBuffersMemory.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	cubemapUniformBuffersMapped.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			cubemapUniformBuffers[i], cubemapUniformBuffersMemory[i], device, physicalDevice);

		vkMapMemory(device, cubemapUniformBuffersMemory[i], 0, bufferSize, 0, &cubemapUniformBuffersMapped[i]);

	}
}

void Minecraft::createCubemapDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::cubemapDepthPipeline.descriptor.layout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pipelines::cubemapDepthPipeline.descriptor.pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	cubemapDescriptorSets.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, cubemapDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets!");
	}

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = cubemapImageView;
		imageInfo.sampler = textureSampler;
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = cubemapUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObjectModel);


		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = cubemapDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = cubemapDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfo;
	
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Minecraft::createGraphicsDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::basePipeline.descriptor.layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pipelines::basePipeline.descriptor.pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets!");
	}

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObjectModel);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = cubemapImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Minecraft::updateUniformBuffer(uint32_t currentImage)
{
	for (size_t j = 0; j < 1; j++)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		UniformBufferObjectModel ubom{};

		glm::mat4 lightSpaceProjection = glm::ortho(-20.f, 20.f, -20.f, 20.f, 0.1f, 15.f);
		glm::mat4 lightView = glm::lookAt(
			glm::vec3(2.0f, 15.0f,1.0f), 
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 1.0f, 0.f)
		);

		glm::vec3 cubePosition = glm::vec3(1.);
		glm::vec3 transformedPosition = glm::vec3(cubePosition.x, cubePosition.y, cubePosition.z);
		ubom.model = glm::mat4(1.);

		ubom.model = glm::translate(ubom.model, transformedPosition);
		float angle = 20.f * j;

		ubom.model = glm::rotate(ubom.model, glm::radians(angle), glm::vec3(1.f, 0.3f, 0.5f));

		ubom.model = glm::scale(ubom.model, glm::vec3(1.));

		ubom.view = camera.getViewMatrix();
		ubom.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);

		lightSpaceProjection[1][1] *= -1;
		ubom.lightSpaceMatrix = lightSpaceProjection * lightView;
		ubom.fragColor = glm::vec3(0., 1., 1.);
	
		ubom.proj[1][1] *= -1;
		ubom.deltaTime = glfwGetTime();

		memcpy(uniformBuffersMapped[currentImage], &ubom, sizeof(ubom));
	}

	UniformBufferObjectModel cubemapUbo;

	cubemapUbo.model = glm::mat4(1.);
	
	cubemapUbo.view = camera.getViewMatrix();

	cubemapUbo.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, 400.f);
	cubemapUbo.proj[1][1] *= -1;

	memcpy(cubemapUniformBuffersMapped[currentImage], &cubemapUbo, sizeof(cubemapUbo));
}

void Minecraft::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPasses.renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = VulkanConfig::swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{1.f, 1.f, 1.f, 1.f}};
	clearValues[1].depthStencil = {1.0f, 0};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkDeviceSize offsets[] = { 0 };

	VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };
	VkBuffer vertexCubemapBuffers[] = { vertexCubemapBuffer };
	
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(VulkanConfig::swapChainExtent.width);
	viewport.height = static_cast<float>(VulkanConfig::swapChainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = VulkanConfig::swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	clearValues[0].color = {{.1f, .1f, .1f, 1.f}};
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	Pipelines::basePipeline.bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::basePipeline.getLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(IVulkanApp::cubeIndices.size()), 1, 0, 0, 0);

	// Cubemap
	Pipelines::cubemapDepthPipeline.bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::cubemapDepthPipeline.getLayout(), 0, 1, &cubemapDescriptorSets[currentFrame], 0, nullptr);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubemapBuffers, offsets);

	for (size_t i = 0; i < instanceCount; i++)
	{
		VkBuffer instanceBuffers[] = { instanceBuffer[i] };
		vkCmdBindVertexBuffers(commandBuffer, 1, 1, instanceBuffers, offsets);
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(Minecraft::cubemapVertices.size()), chunks[i], 0, 0);
	}
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

void Minecraft::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VulkanConfig::msaaSamples; 
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT; 
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	//depthAttachment.format = findDepthFormat();
	depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	depthAttachment.samples = VulkanConfig::msaaSamples; 
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	std::array<VkSubpassDependency, 1> dependency{};

	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = dependency.data();

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses.renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void Minecraft::createDepthResources()
{
	//VkFormat depthFormat = findDepthFormat();
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
	Image::create(VulkanConfig::swapChainExtent.width, VulkanConfig::swapChainExtent.height, 1, 1,0, VK_IMAGE_TYPE_2D,
VulkanConfig::msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_IMAGE_LAYOUT_UNDEFINED, device, physicalDevice);

	depthImageView = Image::createView(depthImage, textureImageView, VK_IMAGE_VIEW_TYPE_2D, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, device, graphicsAndComputeQueue);
}

void Minecraft::createColorResources()
{
	VkFormat colorFormat = swapChainImageFormat;
	
	Image::create(VulkanConfig::swapChainExtent.width,VulkanConfig::swapChainExtent.height, 1, 1,0, VK_IMAGE_TYPE_2D, VulkanConfig::msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, VK_IMAGE_LAYOUT_UNDEFINED, device, physicalDevice);
	colorImageView = Image::createView(colorImage, textureImageView, VK_IMAGE_VIEW_TYPE_2D, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, device, graphicsAndComputeQueue);
}

void Minecraft::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments = { 
			colorImageView, 
			depthImageView,
			swapChainImageViews[i], 
		};
		
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPasses.renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = VulkanConfig::swapChainExtent.width;
		framebufferInfo.height = VulkanConfig::swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		};

	}
}

void Minecraft::recreateSwapChain(GLFWwindow * window)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain(window);
	createImageViews();
	createColorResources();
	createDepthResources();
	createFramebuffers();
}

int Minecraft::getRandomTile(int x, int xOffset, int z, int zOffset)
{
	double integral;
	return abs(std::modf( (sin(x + xOffset * 100. + z + zOffset * 6704.) * 5467.), &integral) * 10.f);	

}

int Minecraft::interpolate(float x, float y, float a)
{
	//std::cout << "x: " << x << '|' << "y: " << y << '|' << "a: " << a << '\n';
	//std::cout << (x * (1 - a) + y * a) << '\n';
//	return (x * (1 - a) + y * a);
	return x + smoothstep(x, y, a) * (y - x);
}

float Minecraft::createGradient(glm::vec2 randomGradient, glm::vec2 node)
{
	glm::vec2 offset = randomGradient - node;

	float gradient = glm::dot(randomGradient, offset);	
	
	std::cout << "Dot product: " << gradient << '\n';
	
	return gradient;
}

int Minecraft::createNoise(glm::vec3 offset)
{
	double integral;
	glm::vec2 id = 
	glm::vec2(std::modf(offset.x * 234.3 , &integral)
	, std::modf(offset.z * 1083.287 , &integral));

	glm::vec2 offsetFractionStage = 
	glm::vec2(std::floor(offset.x * 234.2232)
	, std::floor(offset.z * 2389.98)); 
	
	glm::vec2 offsetFraction = glm::vec2(
				smoothstep(0., CHUNK_SIZE, offsetFractionStage.x),
				smoothstep(0., CHUNK_SIZE, offsetFractionStage.y));	

	const float bottomLeft = getRandomTile(id.x, offset.x, id.y, offset.z);
	
	const float bottomRight = getRandomTile(id.x + CHUNK_SIZE, offset.x, id.y, offset.z);
			
	float bottom = interpolate(bottomLeft, bottomRight, offsetFraction.x);

	const float topLeft = getRandomTile(id.x, offset.x, id.y + CHUNK_SIZE, offset.z);

	const float topRight = getRandomTile(id.x + CHUNK_SIZE, offset.x, id.y + CHUNK_SIZE, offset.z);
	
	float top = interpolate(topLeft, topRight, offsetFraction.x);
	
	int result = interpolate(bottom, top, offsetFraction.y);
	
	return result;
}

float Minecraft::smoothstep(float edge0, float edge1, float x)
{
	float t = std::clamp((x - edge0)/(edge1 - edge0), 0.0f, 1.0f);
	return t * t * (3.0 - 2.0 * t); 
}

void Minecraft::createInstanceBuffers(glm::vec3 offset)
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<> dis(5, 10);	
	std::uniform_real_distribution<> rScale(0.1f, 0.3f);	
	std::uniform_int_distribution<> rRotation(0,180);	
	std::array<std::array<int, CHUNK_SIZE>, CHUNK_SIZE> randomTerrainPositions;

	int currentID = 0;
	chunks.push_back(0);
	for (size_t y = 0; y < randomTerrainPositions.size(); y++)
	{
		for (size_t x = 0; x < randomTerrainPositions[y].size(); x++)
		{
			glm::vec3 pos = glm::vec3(x + cos(glfwGetTime()) * 100., 0., y + sin(glfwGetTime()) * 1216.);

			randomTerrainPositions[y][x] = static_cast<int>(abs(createNoise(pos))); 
			std::cout << "Random pos first call: " << randomTerrainPositions[y][x];

		}
		std::cout << '\n';
	}

	
	// make function
	std::array<glm::vec2, 4> gradients{
		glm::normalize(glm::vec2{cos(glfwGetTime()) * 1867., sin(glfwGetTime()) * 1216.}),
		glm::normalize(glm::vec2{cos(glfwGetTime()) * 9723., sin(glfwGetTime()) * 9629.}),
		glm::normalize(glm::vec2{cos(glfwGetTime()) * 792., sin(glfwGetTime()) * 9787.}),
		glm::normalize(glm::vec2{cos(glfwGetTime()) * 100., sin(glfwGetTime()) * 1216.})
	};

	std::cout << "Gradient vecs: " << '\n' << gradients[0] << '\n' << gradients[1] << '\n' << gradients[2] << '\n' << gradients[3] << '\n';
	for (size_t y = 0; y < randomTerrainPositions.size(); y++)
	{
		for (size_t x = 0; x < randomTerrainPositions[y].size(); x++)
		{
			int randomHeight = 0;
			glm::vec2 currentCoord = glm::vec2{x + offset.x,y + offset.z};

			float gradientNode1 = createGradient(currentCoord + gradients[0], glm::vec2{x + offset.x, y + offset.z});
			float gradientNode2 = createGradient(currentCoord + gradients[1], glm::vec2{x + offset.x, y + offset.z});
			float gradientNode3 = createGradient(currentCoord + gradients[2], glm::vec2{x + offset.x, y + offset.z});
			float gradientNode4 = createGradient(currentCoord + gradients[3], glm::vec2{x + offset.x, y + offset.z});
			
			std::cout << "Interpolation start with gradients: " << gradientNode1 << ',' << gradientNode2 << ',' << gradientNode3 << ',' << gradientNode4 << '\n';
			randomHeight = interpolate(gradientNode1, gradientNode2, randomHeight);
			randomHeight = interpolate(gradientNode2, gradientNode3, randomHeight);
			randomHeight = interpolate(gradientNode3, gradientNode4, randomHeight);
			randomHeight = interpolate(gradientNode4, gradientNode1, randomHeight);
			
			randomTerrainPositions[y][x] = abs(randomHeight);
			chunks[instanceCount] += abs(randomHeight);
		}
	}

	std::vector<InstanceData> iData;
	iData.resize(chunks[instanceCount]);

	std::cout << "Grid\n";
	for (size_t y = 0; y < randomTerrainPositions.size(); y++)
	{
		for (size_t x = 0; x < randomTerrainPositions[y].size(); x++)
		{
		
			std::cout << randomTerrainPositions[y][x] << " , ";

			for (int j = 0 ; j < randomTerrainPositions[y][x]; j++)
			{
				iData[currentID].pos = glm::vec3(x + offset.x, j, y + offset.z);
				iData[currentID].scale = glm::vec3(1.);
				iData[currentID].rot = 0.;
				iData[currentID].id = currentID;
				currentID++;
			}	
		}
		std::cout << '\n';
	}

	VkDeviceMemory&& deviceMemory{};
	VkBuffer&& buffer{};
	instanceBufferMemory.push_back(deviceMemory);
	instanceBuffer.push_back(buffer);

	createVertexBuffer<InstanceData>(iData, instanceBuffer[instanceCount], instanceBufferMemory[instanceCount]);
	instanceCount++;
}

void Minecraft::generateTerrain()
{
	const int RENDER_DISTANCE_MIN = -2;
	const int RENDER_DISTANCE_MAX = 2;
	for (int x = RENDER_DISTANCE_MIN; x < RENDER_DISTANCE_MAX; x++)
	{
		for (int z = RENDER_DISTANCE_MIN; z < RENDER_DISTANCE_MAX; z++)
		{
			glm::vec3 chunkPosition = glm::vec3{CHUNK_SIZE * ((int)(camera.cameraPos.x/CHUNK_SIZE) + x), 0., CHUNK_SIZE * ((int)(camera.cameraPos.z/CHUNK_SIZE) + z)};
			if (!renderedChunks.contains(chunkPosition))
			{
				createInstanceBuffers(chunkPosition);
			
				renderedChunks.insert(std::move(chunkPosition));	
			}

		}
	}
}

void Minecraft::drawFrame(GLFWwindow * window)
{
	std::thread terrainThread(&Minecraft::generateTerrain, this);	
	terrainThread.join();

	camera.update();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain(window);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(CommandBuffer::commandBuffers[currentFrame], 0);

	recordCommandBuffer(CommandBuffer::commandBuffers[currentFrame], imageIndex);
	
	updateUniformBuffer(currentFrame);

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffer::commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsAndComputeQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || GLFWWindowContext::framebufferResized)
	{
		GLFWWindowContext::framebufferResized = false;
		recreateSwapChain(window);
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % VulkanConfig::MAX_FRAMES_IN_FLIGHT;

	double currentTime = glfwGetTime();
	lastFrameTime = (currentTime - lastTime);
	lastTime = currentTime;
}
