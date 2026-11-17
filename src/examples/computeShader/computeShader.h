#pragma once

#include "../../core/renderer/vulkanApp/vulkanApp.h"
#include "../../core/vkAbstraction/vkAbstraction.h"

class ComputeShader : public IVulkanApp
{
	public:
	ComputeShader() = default;
	~ComputeShader(){};

	constexpr static int frames 		= 2;
	constexpr static int WIDTH 		= 800;
	constexpr static int HEIGHT 		= 600;
	constexpr static int SPHERE_COUNT 	= 1;
	constexpr static int PARTICLE_COUNT 	= 8192;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	std::vector<Vertex> 	sphereVertices{};
	std::vector<uint32_t> 	sphereIndices{};

	VkBuffer 		sphereBuffer;
	VkDeviceMemory 		sphereBufferMemory;
	VkBuffer 		sphereIndexBuffer;
	VkDeviceMemory 		sphereIndexBufferMemory;

	//compute
	VkCommandPool 				computeCommandPool;
	std::array<VkCommandBuffer, frames> 	computeCommandBuffer;
	std::vector<VkFence> 			computeInFlightFences;
	std::vector<VkSemaphore> 		computeFinishedSemaphores;
	VkDescriptorSetLayout 			computeDescriptorSetLayout;
	std::array<VkDescriptorSet, frames> 	computeDescriptorSets;

	struct Particle
	{
		glm::vec3 position;	
		glm::vec3 velocity;	
		glm::vec4 color;

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Particle, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Particle, color);

			return attributeDescriptions;
		}    

		static VkVertexInputBindingDescription getBindingDesciption()
		{
			VkVertexInputBindingDescription bindingDescription{};
		    
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Particle);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	    
			return bindingDescription;
		}
	};

	struct
	{
		std::array<vk::Buffer, frames> shaderStorageBuffer;
	} buffers;

	struct ParameterUBO 
	{
		float deltaTime;
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
		VkPipelineLayout sphere;
		VkPipelineLayout compute;
	} pipelineLayouts{};	

	struct 
	{
		VkPipeline sphere{VK_NULL_HANDLE};	
		VkPipeline compute{VK_NULL_HANDLE};	
	} pipelines;

	struct
	{
		VkDescriptorSetLayout cube;
		VkDescriptorSetLayout sphere;
		VkDescriptorSetLayout particle;
	} descriptorSetLayouts;

	struct DescriptorSets
	{
		VkDescriptorSet cube;		
		VkDescriptorSet particle;		
		std::vector<VkDescriptorSet> sphere;
	}; 

	std::array<DescriptorSets, frames> descriptorSets;

	struct UniformBuffers
	{
		std::vector<VkBuffer> sphere;
		VkBuffer scene;
		VkBuffer particle;
	};

	struct UniformBuffersMapped
	{
		std::vector<void *> sphere;
		void * scene;
		void * particle;
	};

	struct UniformBuffersMemory
	{
		std::vector<VkDeviceMemory> sphere;
		VkDeviceMemory scene;
		VkDeviceMemory particle;
	};

	std::array<UniformBuffers, frames> uniformBuffers;
	std::array<UniformBuffersMapped, frames> uniformBuffersMapped;
	std::array<UniformBuffersMemory, frames> uniformBuffersMemory;
	
	void setupUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformData);
		VkDeviceSize computeBufferSize = sizeof(ParameterUBO);
		
		for (size_t i = 0; i < frames; i++)
		{
			uniformBuffers[i].sphere.resize(SPHERE_COUNT);
			uniformBuffersMemory[i].sphere.resize(SPHERE_COUNT);
			uniformBuffersMapped[i].sphere.resize(SPHERE_COUNT);

			Buffer::create(computeBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].particle, uniformBuffersMemory[i].particle, VulkanConfig::device, VulkanConfig::physicalDevice);

			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].particle, 0, computeBufferSize, 0, &uniformBuffersMapped[i].particle);

			for (size_t j = 0; j < SPHERE_COUNT; j++)
			{
				Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				uniformBuffers[i].sphere[j], uniformBuffersMemory[i].sphere[j], VulkanConfig::device, VulkanConfig::physicalDevice);

				vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].sphere[j], 0, bufferSize, 0, &uniformBuffersMapped[i].sphere[j]);
			};
	
		}	
	};

	void init(GLFWwindow* window)
	{
		IVulkanApp::init(window);	
		setupUniformBuffers();
		{
			setupSphere();
			createVertexBuffer(sphereVertices, sphereBuffer, sphereBufferMemory);
			createSphereIndexBuffer();
			setupSphereResources();
		};
		setupParticles();
		setupComputeCommandBuffer();
		setupComputeDescriptorSets();
		setupComputePipeline();
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
		UniformData uniformData;
		
		uniformData.model = glm::mat4(1.);		
		uniformData.view = camera.getViewMatrix();
		uniformData.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
		uniformData.proj[1][1] *= -1.;
		
		memcpy(uniformBuffersMapped[currentImage].sphere[0], &uniformData, sizeof(uniformData));

		ParameterUBO computeUBO;
		computeUBO.deltaTime = lastFrameTime * 2.f;	

		memcpy(uniformBuffersMapped[currentImage].particle, &computeUBO, sizeof(computeUBO));
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
			.clearValue = {{0.f, 0.f, 0.f, 1.f}}
		};

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkImageMemoryBarrier barrier
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

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
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

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.sphere);	

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.shaderStorageBuffer[currentFrame].buffer, offsets);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.sphere, 0, 1, &descriptorSets[currentFrame].sphere[0], 0, nullptr);
		
		vkCmdDraw(commandBuffer, PARTICLE_COUNT, 1, 0, 0);
		
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
			vkDestroyBuffer(VulkanConfig::device, uniformBuffers[i].scene, nullptr);
			vkFreeMemory(VulkanConfig::device, uniformBuffersMemory[i].scene, nullptr);
		}		

		vkDestroyDescriptorSetLayout(VulkanConfig::device, descriptorSetLayouts.sphere, nullptr);

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

	void setupSpherePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.sphere,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.sphere) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupSpherePipeline()
	{
		setupSpherePipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/computeShader/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/computeShader/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
		VkVertexInputBindingDescription bindingDescription{
			.binding = 0,
			.stride = sizeof(Particle),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};		
		
		std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{bindingDescription};
		
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Particle, position);
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Particle, color);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),	
			.pVertexBindingDescriptions = bindingDescriptions.data(),
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};
		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
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
			.layout = pipelineLayouts.sphere,
			.renderPass = nullptr,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.sphere) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
		
		setupDebugObjectName(VK_OBJECT_TYPE_PIPELINE, pipelines.sphere, "pipelines.sphere");
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

		auto shaderCode = FileContext::readFile(SHADER_DIRECTORY + "/computeShader/comp.spv");

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

		std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
		
		layoutBindings[0].binding 		= 0;
		layoutBindings[0].stageFlags 		= VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[0].descriptorType 	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount 	= 1;
		layoutBindings[0].pImmutableSamplers 	= nullptr;

		layoutBindings[1].binding 		= 1;
		layoutBindings[1].stageFlags 		= VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[1].descriptorType 	= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[1].descriptorCount 	= 1;
		layoutBindings[1].pImmutableSamplers 	= nullptr;

		layoutBindings[2].binding 		= 2;
		layoutBindings[2].stageFlags 		= VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[2].descriptorType 	= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[2].descriptorCount 	= 1;
		layoutBindings[2].pImmutableSamplers 	= nullptr;

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
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
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
				.buffer = uniformBuffers[i].particle,
				.offset = 0,
				.range = sizeof(ParameterUBO)
			};

			VkDescriptorBufferInfo storageBufferInfoLastFrame
			{
				.buffer = buffers.shaderStorageBuffer[(i - 1) % frames].buffer,
				.offset = 0,
				.range = sizeof(Particle) * PARTICLE_COUNT
			};

			VkDescriptorBufferInfo storageBufferInfoCurrentFrame
			{
				.buffer = buffers.shaderStorageBuffer[i].buffer,
				.offset = 0,
				.range = sizeof(Particle) * PARTICLE_COUNT
			};
		
			std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = computeDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = computeDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = computeDescriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void setupSphereDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings{vertexLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.sphere))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames * SPHERE_COUNT) * 2;

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames * SPHERE_COUNT) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		descriptorSets[0].sphere.resize(SPHERE_COUNT);
		descriptorSets[1].sphere.resize(SPHERE_COUNT);

		for (size_t j = 0; j < SPHERE_COUNT; j++)
		{
			std::array<VkDescriptorSetLayout, 1> layouts{};
			layouts.fill(descriptorSetLayouts.sphere);	
			
			VkDescriptorSetAllocateInfo allocInfo
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool = descriptorPool,
				.descriptorSetCount = layouts.size(),
				.pSetLayouts = layouts.data()
			};

			for (size_t i = 0; i < frames; i++)
			{
				if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].sphere[j]) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to allocate descriptor sets!");
				};
				
				VkDescriptorBufferInfo bufferInfo{
					.buffer = uniformBuffers[i].sphere[j],
					.offset = 0,
					.range = sizeof(UniformData)
				};
				
				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
				
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSets[i].sphere[j];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
			};
		};
	}	

	void setupSphereResources()
	{
		setupSphereDescriptorSets();	
		setupSpherePipeline();
	};

	void setupSphere()
	{
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
		    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		    {
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			sphereVertices.emplace_back(
				glm::vec3{xPos, yPos, zPos}, 
				glm::vec3{1.0, 1.0, 1.0}, 
				glm::vec3{xPos, yPos, zPos}, 
				glm::vec2{xSegment, ySegment});
		    };
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
		    if (!oddRow) // even rows: y == 0, y == 2; and so on
		    {
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
			    sphereIndices.push_back(y * (X_SEGMENTS + 1) + x);
			    sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		    }
		    else
		    {
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
			    sphereIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			    sphereIndices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		    }
		    oddRow = !oddRow;
		}
	};

	void createSphereIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(sphereIndices[0]) * sphereIndices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, sphereIndices.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sphereIndexBuffer, sphereIndexBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);
		copyBuffer(stagingBuffer, sphereIndexBuffer, bufferSize);
		
		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	}

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
	void setupParticles()
	{
		std::default_random_engine rndEngine((unsigned)time(nullptr));
		std::uniform_real_distribution<float> rndDist(0.f, 1.f);
		
		std::vector<Particle> particles(PARTICLE_COUNT);
		
		for (auto& particle : particles)
		{
			float r 	= 0.25f * sqrt(rndDist(rndEngine));
			float theta 	= rndDist(rndEngine) * 2 * 3.14159265358979323846;
			float x 	= 2.f * r * cos(theta) * HEIGHT / WIDTH;
			float y 	= 2.f * r * sin(theta);
			float z 	= x;

			particle.position = glm::vec3(x, y, z);
			particle.velocity = glm::normalize(glm::vec3(x, y, z)) * 0.00025f;
			particle.color 	  = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.f);
		};

		VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

		VkBuffer 	stagingBuffer;
		VkDeviceMemory 	stagingBufferMemory;

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, particles.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

		for (size_t i = 0; i < frames; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers.shaderStorageBuffer[i].buffer, buffers.shaderStorageBuffer[i].memory, VulkanConfig::device, VulkanConfig::physicalDevice);

			copyBuffer(stagingBuffer, buffers.shaderStorageBuffer[i].buffer, bufferSize);

		};

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
	
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelines.compute);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayouts.compute, 0, 1, &computeDescriptorSets[currentFrame], 0, 0);
	
		vkCmdDispatch(commandBuffer, PARTICLE_COUNT/256, 1, 1);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

};
