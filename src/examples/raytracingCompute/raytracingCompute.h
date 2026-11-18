#pragma once

#include "../../core/renderer/vulkanApp/vulkanApp.h"

class RaytracingCompute : public IVulkanApp
{
	public:
	RaytracingCompute() = default;
	~RaytracingCompute(){};

	constexpr static int frames 		= 2;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	//compute
	VkCommandPool 				computeCommandPool;
	std::array<VkCommandBuffer, frames> 	computeCommandBuffer;
	std::vector<VkFence> 			computeInFlightFences;
	std::vector<VkSemaphore> 		computeFinishedSemaphores;
	VkDescriptorSetLayout 			computeDescriptorSetLayout;
	std::array<VkDescriptorSet, frames> 	computeDescriptorSets;

	struct
	{
		VkImage        image;
		VkImageView    imageView;
		VkSampler      sampler;
		VkDeviceMemory imageMemory;
	} texture;		

	struct
	{
		std::array<vk::Buffer, frames> shaderStorageBuffer;
	} buffers;

	struct ParameterUBO 
	{
		float deltaTime;
	};

	struct Sphere
	{
		alignas(16) glm::vec4 center;
		alignas(16) double    radius;
	};

	struct WorldInfo
	{
		alignas(64) Sphere spheres[4];
	};

	struct UniformData
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
	
	struct 
	{
		VkRenderPass renderPass;	
	} renderPasses{};

	struct
	{
		VkPipelineLayout cube;
		VkPipelineLayout compute;
	} pipelineLayouts{};	

	struct 
	{
		VkPipeline cube{VK_NULL_HANDLE};	
		VkPipeline compute{VK_NULL_HANDLE};	
	} pipelines;

	struct
	{
		VkDescriptorSetLayout cube;
		VkDescriptorSetLayout compute;
	} descriptorSetLayouts;

	struct DescriptorSets
	{
		VkDescriptorSet cube;		
	}; 

	std::array<DescriptorSets, frames> descriptorSets;

	struct UniformBuffers
	{
		VkBuffer compute;
		VkBuffer world;
	};

	struct UniformBuffersMapped
	{
		void * compute;
		void * world;
	};

	struct UniformBuffersMemory
	{
		VkDeviceMemory compute;
		VkDeviceMemory world;
	};

	std::array<UniformBuffers, frames> uniformBuffers;
	std::array<UniformBuffersMapped, frames> uniformBuffersMapped;
	std::array<UniformBuffersMemory, frames> uniformBuffersMemory;
	
	void setupUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(WorldInfo);
		VkDeviceSize computeBufferSize = sizeof(ParameterUBO);
		
		for (size_t i = 0; i < frames; i++)
		{
			Buffer::create(computeBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].compute, uniformBuffersMemory[i].compute, VulkanConfig::device, VulkanConfig::physicalDevice);

			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].compute, 0, computeBufferSize, 0, &uniformBuffersMapped[i].compute);
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].world, uniformBuffersMemory[i].world, VulkanConfig::device, VulkanConfig::physicalDevice);

			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].world, 0, bufferSize, 0, &uniformBuffersMapped[i].world);
		}	
	};

	void init(GLFWwindow* window)
	{
		IVulkanApp::init(window);	
		setupUniformBuffers();
		setupSampler();
		loadTexture();
		setupImageView();

		setupRaytracing();
		setupComputeCommandBuffer();
		setupComputeDescriptorSets();
		setupComputePipeline();
		setupCubeResources();
	};

	void createVertexBuffer(std::vector<Vertex> vertices, VkBuffer& buffer, VkDeviceMemory& memory)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory, VulkanConfig::device, VulkanConfig::physicalDevice);

		copyBuffer(stagingBuffer, buffer, bufferSize);

		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	};	

	void updateUniformBuffer(uint32_t currentImage)
	{
		ParameterUBO computeUBO;
		computeUBO.deltaTime = lastFrameTime * 2.f;	

		memcpy(uniformBuffersMapped[currentImage].compute, &computeUBO, sizeof(computeUBO));
	
		WorldInfo world;
		world.spheres[0].center = glm::vec4(0.,0.,-1., 1.);
		world.spheres[0].radius = 0.5;
		
		memcpy(uniformBuffersMapped[currentImage].world, &world, sizeof(world));
	};

	void processInput(GLFWwindow * window)
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

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkRenderingAttachmentInfoKHR colorAttachmentInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
			.imageView = swapChainImageViews[imageIndex],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {{1.f, 0.f, 0.f, 1.f}}
		};

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture.image};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	

		barrier = 
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapChainImages[imageIndex]
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;		

		vkCmdPipelineBarrier
		(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);	

		VkRenderingInfo renderingInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
		};

		renderingInfo.renderArea.offset = {0,0};
		renderingInfo.renderArea.extent = VulkanConfig::swapChainExtent;
	
		vkCmdBeginRenderingKHR(instance, commandBuffer, &renderingInfo);

		VkDeviceSize offsets[] = { 0 };

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

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };
		
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.cube);	
		
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.cube, 0, 1, &descriptorSets[currentFrame].cube, 0, nullptr);
		
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(RaytracingCompute::cubeIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderingKHR(instance, commandBuffer);

		barrier = 
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapChainImages[imageIndex]
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		

		vkCmdPipelineBarrier
		(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);	

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void cleanup(GLFWwindow * window)
	{
		cleanupSwapChain();

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		for (size_t i = 0; i < frames; i++)
		{
			vkDestroyBuffer(VulkanConfig::device, uniformBuffers[i].compute, nullptr);
			vkFreeMemory(VulkanConfig::device, uniformBuffersMemory[i].compute, nullptr);
		}		

		vkDestroyBuffer(VulkanConfig::device, indexBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, indexBufferMemory, nullptr);
		
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);

		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);

		vkDestroyRenderPass(VulkanConfig::device, renderPasses.renderPass, nullptr);

		vkDestroyDevice(VulkanConfig::device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
		
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	};

	void setupCubePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.cube,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.cube) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupCubePipeline()
	{
		setupCubePipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/raytracingCompute/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/raytracingCompute/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
		VkVertexInputBindingDescription bindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};		
		
		std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{bindingDescription};
		
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),	
			.pVertexBindingDescriptions = bindingDescriptions.data(),
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};
		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		VkViewport viewport{
			.x = 0.f,
			.y = 0.f,
			.width = (float)VulkanConfig::swapChainExtent.width,
			.height = (float)VulkanConfig::swapChainExtent.height,
			.minDepth = 0.f,
			.maxDepth = 1.f};

		VkPipelineViewportStateCreateInfo viewportState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1};

		// stencil info
		VkStencilOpState stencilOpState {
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_REPLACE,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.compareMask = 0xFF,
			.writeMask = 0xFF,
			.reference = 1 
		};

		VkPipelineDepthStencilStateCreateInfo depthStencil {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			//.front = stencilOpState,
			//.back = stencilOpState,
			.minDepthBounds = 0.f,
			.maxDepthBounds = 1.f
		};

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamicState {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineRasterizationStateCreateInfo rasterizer{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.minSampleShading = 0.2f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		VkPipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		std::array<VkPipelineColorBlendAttachmentState, 1> colorBlendAttachments{colorBlendAttachment};

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = colorBlendAttachments.data(),
			.blendConstants = {0.f, 0.f, 0.f, 0.f}
		};

		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &swapChainImageFormat,
			.depthAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		VkGraphicsPipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &pipelineRenderingCreateInfo,
			.stageCount = static_cast<uint32_t>(shaderStages.size()),
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = pipelineLayouts.cube,
			.renderPass = nullptr,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.cube) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
		
		setupDebugObjectName(VK_OBJECT_TYPE_PIPELINE, pipelines.cube, "pipelines.cube");
	};

	void setupComputePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &computeDescriptorSetLayout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.compute) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create compute pipeline layout!");
		};
	};

	void setupComputePipeline()
	{
		setupComputePipelineLayout();

		auto shaderCode = FileContext::readFile(SHADER_DIRECTORY + "/raytracingCompute/comp.spv");

		VkShaderModule shaderModule = createShaderModule(shaderCode, VulkanConfig::device);

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo
		{
			.sType 	= VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage	= shaderStageInfo,
			.layout = pipelineLayouts.compute,
		};
		
		if (vkCreateComputePipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.compute) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create compute pipeline!");
		};
	};

	void setupComputeDescriptorSets()
	{
		VkDescriptorPool descriptorPool;

		std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings{};
		
		layoutBindings[0].binding 		= 0;
		layoutBindings[0].stageFlags 		= VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[0].descriptorType 	= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		layoutBindings[0].descriptorCount 	= 1;
		layoutBindings[0].pImmutableSamplers 	= nullptr;

		layoutBindings[1].binding 		= 1;
		layoutBindings[1].stageFlags 		= VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[1].descriptorType 	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[1].descriptorCount 	= 1;
		layoutBindings[1].pImmutableSamplers 	= nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
			.pBindings = layoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &computeDescriptorSetLayout))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(computeDescriptorSetLayout);	
		
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};

		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &computeDescriptorSets[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};

			VkDescriptorBufferInfo bufferInfo
			{
				.buffer = uniformBuffers[i].world,
				.offset = 0,
				.range 	= sizeof(WorldInfo)
			};

			VkDescriptorImageInfo storageImageInfo
			{
				.sampler = texture.sampler,
				.imageView = texture.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL
			};
	
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = computeDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &storageImageInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = computeDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void setupCubeResources()
	{
		setupCubeDescriptorSets();	
		setupCubePipeline();
	};

	void addShader(const std::string&& path, VkShaderStageFlagBits stage)
	{
		auto shaderCode = FileContext::readFile(path);

		VkShaderModule shaderModule = createShaderModule(shaderCode, VulkanConfig::device);
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = stage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		
		assert(stage != 0);
		
		shaderStages.push_back(shaderStageInfo);	
	}

	void drawFrame(GLFWwindow * window)
	{
		camera.update();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		vkWaitForFences(VulkanConfig::device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		updateUniformBuffer(currentFrame);

		vkResetFences(VulkanConfig::device, 1, &computeInFlightFences[currentFrame]);

		vkResetCommandBuffer(computeCommandBuffer[currentFrame], 0);

		recordComputeCommandBuffer(computeCommandBuffer[currentFrame]);

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &computeCommandBuffer[currentFrame];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores= &computeFinishedSemaphores[currentFrame];

		if (vkQueueSubmit(VulkanConfig::graphicsAndComputeQueue, 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit compute command buffer!");
		}

		vkWaitForFences(VulkanConfig::device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		vkResetFences(VulkanConfig::device, 1, &inFlightFences[currentFrame]);

		uint32_t imageIndex;

		VkResult result = vkAcquireNextImageKHR(VulkanConfig::device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain(window);
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		recordCommandBuffer(CommandBuffer::commandBuffers[currentFrame], imageIndex);

		VkSemaphore waitSemaphores[] = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 2;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer::commandBuffers[currentFrame];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex]};
		submitInfo.signalSemaphoreCount = 1;

		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(VulkanConfig::graphicsAndComputeQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
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

	void createSyncObjects()
	{
		renderFinishedSemaphores.resize(swapChainImages.size());
		imageAvailableSemaphores.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		computeInFlightFences.resize(frames);
		computeFinishedSemaphores.resize(frames);

		inFlightFences.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (
				vkCreateSemaphore(VulkanConfig::device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(VulkanConfig::device, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(VulkanConfig::device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS ||
				vkCreateFence(VulkanConfig::device, &fenceInfo, nullptr, &computeInFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create semaphores!");
			}
			setupDebugObjectName(VK_OBJECT_TYPE_SEMAPHORE, imageAvailableSemaphores[i], "imageAvailableSemaphores[" + std::string{i} + "]");
			setupDebugObjectName(VK_OBJECT_TYPE_FENCE, inFlightFences[i], "inFlightFences[" + std::string{i} + "]");
		}

		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			if (vkCreateSemaphore(VulkanConfig::device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create semaphores!");
			}

			setupDebugObjectName(VK_OBJECT_TYPE_SEMAPHORE, renderFinishedSemaphores[i], "renderFinishedSemaphores[" + std::string{i} + "]");
		}
	}

	// Compute related
	void setupRaytracing()
	{		
		int texWidth 	= VulkanConfig::swapChainExtent.width;
		int texHeight	= VulkanConfig::swapChainExtent.height;

		std::vector<glm::vec4> colors{};
		
		colors.resize(texWidth * texHeight * 4);		
	
		for (size_t i = 0; i < texWidth * texHeight * 4; i++)
		{
			colors.push_back(glm::vec4(1., 1., 0., 0.));
		};
/*
		VkDeviceSize bufferSize = texWidth * texHeight * 4;

		VkBuffer 	stagingBuffer;
		VkDeviceMemory 	stagingBufferMemory;

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, colors.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

		for (size_t i = 0; i < frames; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers.shaderStorageBuffer[i].buffer, buffers.shaderStorageBuffer[i].memory, VulkanConfig::device, VulkanConfig::physicalDevice);

			copyBuffer(stagingBuffer, buffers.shaderStorageBuffer[i].buffer, bufferSize);
			
			setupDebugObjectName(VK_OBJECT_TYPE_BUFFER, buffers.shaderStorageBuffer[i].buffer, "buffers.shaderStorageBuffer[i].buffer");
		};
*/
	};
	
	void setupComputeCommandBuffer()
	{
		QueueFamilyIndices queueFamilyIndices = QueueFamily::findQueueFamilies(VulkanConfig::physicalDevice, surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

		if (vkCreateCommandPool(VulkanConfig::device, &poolInfo, nullptr, &computeCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = computeCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = frames;

		if (vkAllocateCommandBuffers(VulkanConfig::device, &allocInfo, computeCommandBuffer.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
		
		setupDebugObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, computeCommandBuffer[0], "computeCommandBuffer[0]");
		setupDebugObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, computeCommandBuffer[1], "computeCommandBuffer[1]");
	};

	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_GENERAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture.image};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	


		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.compute);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayouts.compute, 0, 1, &computeDescriptorSets[currentFrame], 0, 0);
	
		vkCmdDispatch(commandBuffer, 100, 100, 1);

		int texWidth 	= VulkanConfig::swapChainExtent.width;
		int texHeight	= VulkanConfig::swapChainExtent.height;

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanConfig::physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find a suitable memory type!");
	}

	// Raytracing
	void loadTexture()
	{
/*
		const std::string raytrace_png = ROOT_DIR + "/resource/textures/raytrace_example.png";
	
		// World
		hittable_list world;
		auto R = std::cos(pi/4);

		auto material_ground 	= 	make_shared<lambertian>(glm::vec3(0.5, 0.5,  0.5));
		auto material_center 	= 	make_shared<lambertian>(glm::vec3(0.1, 0.2,  0.5));
		auto material_left 	= 	make_shared<dielectric>(1.5);
		auto material_bubble    = 	make_shared<dielectric>(1./1.5);
		auto material_right 	= 	make_shared<metal>(glm::vec3(0.8, 0.6, 0.2), 1.0);

		for (int a = -11; a < 11; a++)
		{
			for (int b = -11; b < 11; b++)
			{
				auto choose_mat = random_double();
				glm::vec3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());
				
				if ((center - glm::vec3(4, 0.2, 0)).length() > 0.9)
				{
					shared_ptr<material> sphere_material;
					
					if (choose_mat < 0.8)
					{
						auto albedo = glm::vec3(random_double(0., 1.), random_double(0., 1.), random_double(0., 1.))
						* glm::vec3(random_double(0., 1.), random_double(0., 1.), random_double(0., 1.));
						
						sphere_material = make_shared<lambertian>(albedo);
						world.add(make_shared<sphere>(center, 0.2, sphere_material));
					}
					else if (choose_mat < 0.95)
					{
						auto albedo = glm::vec3(random_double(0.5, 1.), random_double(0.5, 1.), random_double(0.5, 1.));
						auto fuzz = random_double(0., 0.5);
						sphere_material = make_shared<metal>(albedo, fuzz);
						world.add(make_shared<sphere>(center, 0.2, sphere_material));
					}
					else
					{
						sphere_material = make_shared<dielectric>(1.5);
						world.add(make_shared<sphere>(center, 0.2, sphere_material));
					};
				};
			};
		};

		auto material1 = make_shared<dielectric>(1.5);
		world.add(make_shared<sphere>(glm::vec3(0., 1., 0.), 1., material1));

		auto material2 = make_shared<lambertian>(glm::vec3(0.4, 0.2, 0.1));
		world.add(make_shared<sphere>(glm::vec3(-4., 1., 0.), 1., material2));

		auto material3 = make_shared<metal>(glm::vec3(0.7, 0.6, 0.5), 0.0);
		world.add(make_shared<sphere>(glm::vec3(4., 1., 0.), 1., material3));

		rtCamera cam;
	
		cam.aspect_ratio = 16./9.;
		cam.image_width  = 1200;
		cam.samples_per_pixel = 3;
		cam.max_depth = 50;

		cam.vfov = 20;
		cam.lookfrom 	= glm::vec3(13., 2., 3.);
		cam.lookat 	= glm::vec3(0., 0., 0.);
		cam.vup 	= glm::vec3(0., 1., 0.);
		
		cam.defocus_angle = 0.6;
		cam.focus_dist = 10.0;

		cam.render(world);

		stbi_write_png(raytrace_png.c_str(), cam.image_width, cam.image_height, 3, cam.rayTracePixels.data(), cam.image_width * 3);

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(raytrace_png.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");	
		};
		
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;	
	
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		
		Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);		
		void * data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, imageSize, 0, &data);
		
		memcpy(data, pixels, static_cast<size_t>(imageSize));	
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);
		
		//stbi_image_free(pixels);
	*/	
		int texWidth 	= VulkanConfig::swapChainExtent.width;
		int texHeight	= VulkanConfig::swapChainExtent.height;

		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &texture.image))
		{
			throw std::runtime_error("failed to create image!");
		};	
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, texture.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		
		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &texture.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");	
		};
		
		vkBindImageMemory(VulkanConfig::device, texture.image, texture.imageMemory, 0);
}	

	void setupImageView()
	{
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = texture.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_UNORM};		

			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &texture.imageView))
		{
			throw std::runtime_error("failed to create image view!");	
		};
	};

	void setupSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanConfig::physicalDevice, &properties);
		
		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.f,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.f,
			.maxLod = VK_LOD_CLAMP_NONE,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};		
		
		if (vkCreateSampler(VulkanConfig::device, &samplerInfo, nullptr, &texture.sampler))
		{
			throw std::runtime_error("failed to create sampler!");	
		};
	};

	void setupCubeDescriptorSets()
	{
		VkDescriptorPool descriptorPool;

		std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings{};

		layoutBindings[0].binding 		= 0;
		layoutBindings[0].descriptorType 	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[0].descriptorCount 	= 1;
		layoutBindings[0].stageFlags 		= VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[0].pImmutableSamplers 	= nullptr;
	
		VkDescriptorSetLayoutCreateInfo layoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
			.pBindings = layoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.cube))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(descriptorSetLayouts.cube);	
		
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
		
		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].cube) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};
		
			VkDescriptorImageInfo imageInfo
			{
				.sampler = texture.sampler,
				.imageView = texture.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].cube;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void recreateSwapChain(GLFWwindow * window)
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);

		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(VulkanConfig::device);

		cleanupSwapChain();

		createSwapChain(window);
		createImageViews();
		createFramebuffers();
		setupRaytracing();
		loadTexture();
		setupImageView();
		setupComputeDescriptorSets();
	}
	
};
