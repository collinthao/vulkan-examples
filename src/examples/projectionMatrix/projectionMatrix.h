#pragma once

#include "../../core/renderer/vulkanApp/vulkanApp.h"
class ProjectionMatrix : public IVulkanApp
{
	public:
	ProjectionMatrix() = default;
	~ProjectionMatrix(){};

	constexpr static int frames = 2;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<Vertex> projectionVertices{};
	std::vector<VkFramebuffer> offscreenFramebuffers;
	float projectionX = 0.0;

	Camera mainCamera{glm::vec3(0.,0.,3.), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f)};
	Camera renderCamera{glm::vec3(6.,7.,14.), glm::vec3(-1.f, -1.f, -1.f), glm::vec3(0.f, 1.f, 0.f)};

	const std::vector<uint32_t> projectionIndices=
	{
	    0, 1, 2,
	    2, 0, 3,

	    5, 4, 6,
	    7, 4, 6,

	    9,10, 8,
	   11, 8,10,

	   12,14,13,
	   14,12,15,

	   17,18,16,
	   19,16,18,

	   20,22,21,
	   22,20,23,

	   9,10,14,
	   14,13, 9 
	};

	struct UniformData
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
	
	struct 
	{
		VkRenderPass offscreenPass;	
		VkRenderPass renderPass;	
	} renderPasses{};

	VkPipelineLayout pipelineLayout{}; 

	struct
	{
		VkPipelineLayout quad;
	} pipelineLayouts;

	struct 
	{
		VkPipeline cubePipeline{VK_NULL_HANDLE};
		VkPipeline projection{VK_NULL_HANDLE};
		VkPipeline projectionCube{VK_NULL_HANDLE};
		VkPipeline quad{VK_NULL_HANDLE};
	} pipelines;

	VkDescriptorSetLayout descriptorSetLayout;
	struct
	{
		VkDescriptorSetLayout quad;
	} descriptorSetLayouts;

	struct DescriptorSets
	{
		VkDescriptorSet cube;		
		VkDescriptorSet quad;		
		VkDescriptorSet projection;		
		VkDescriptorSet projectionCube;		
	}; 

	std::array<DescriptorSets, frames> descriptorSets;

	struct UniformBuffers
	{
		VkBuffer scene;
		VkBuffer quad;
		VkBuffer projection;
		VkBuffer projectionCube;
	};

	struct UniformBuffersMapped
	{
		void * scene;
		void * quad;
		void * projection;
		void * projectionCube;
	};

	struct UniformBuffersMemory
	{
		VkDeviceMemory scene;
		VkDeviceMemory quad;
		VkDeviceMemory projection;
		VkDeviceMemory projectionCube;
	};

	struct
	{
		VkDeviceMemory memory;	
		VkDeviceMemory indexMemory;	
		VkBuffer buffer;
		VkBuffer indexBuffer;
	} projection;

	VkSampler sampler;
	struct Texture
	{
		VkImageView imageView;
		VkImage     image;
		VkDeviceMemory imageMemory;
	};	

	struct
	{
		Texture depth;
		Texture color;
		Texture projectionDepth;
	} textures;

	std::array<UniformBuffers, frames> uniformBuffers;
	std::array<UniformBuffersMapped, frames> uniformBuffersMapped;
	std::array<UniformBuffersMemory, frames> uniformBuffersMemory;

	void createProjectionIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(projectionIndices[0]) * projectionIndices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, projectionIndices.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, projection.indexBuffer, projection.indexMemory, VulkanConfig::device, VulkanConfig::physicalDevice);
		copyBuffer(stagingBuffer, projection.indexBuffer, bufferSize);
		
		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	}

	void setupProjectionVertices()
	{
		glm::mat4 perspective = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, 5.f);
		
		for (size_t i = 0; i < cubeVertices.size(); i++)
		{
			glm::vec4 transformedPos = glm::vec4(cubeVertices[i].pos.x, cubeVertices[i].pos.y, cubeVertices[i].pos.z + 5.f, 1.0) * perspective;
			if (cubeVertices[i].pos.z > 0.f)
			{
				projectionVertices.push_back(Vertex{
					cubeVertices[i].pos,
					cubeVertices[i].color,
					cubeVertices[i].normal,
					cubeVertices[i].texCoord
				});
			}
			else
			{
				projectionVertices.push_back(Vertex{
					glm::vec3(transformedPos.x, transformedPos.y, transformedPos.z),
					cubeVertices[i].color,
					cubeVertices[i].normal,
					cubeVertices[i].texCoord
				});
			}
		};
	};

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

	void setupColor()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.color.image))
		{
			throw std::runtime_error("failed to create albedo image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.color.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.color.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for albedo image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.color.image, textures.color.imageMemory, 0);
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.color.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.color.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
		};
	};	

	void setupProjectionDepth()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT, 
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.projectionDepth.image))
		{
			throw std::runtime_error("failed to create image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.projectionDepth.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.projectionDepth.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for depth image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.projectionDepth.image, textures.projectionDepth.imageMemory, 0);
	
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = textures.projectionDepth.image	
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;	
		barrier.subresourceRange.baseMipLevel = 0;	
		barrier.subresourceRange.levelCount = 1;	
		barrier.subresourceRange.baseArrayLayer  = 0;	
		barrier.subresourceRange.layerCount = 1;	

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;	
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;		

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	

		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
	
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.projectionDepth.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.projectionDepth.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create depth image view!");
		};
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.projectionDepth.image, "textures.projectionDepth.image");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.projectionDepth.imageView, "textures.projectionDepth.imageView");
	};

	void setupDepth()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT, 
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.depth.image))
		{
			throw std::runtime_error("failed to create image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.depth.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.depth.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for depth image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.depth.image, textures.depth.imageMemory, 0);
	
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = textures.depth.image	
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;	
		barrier.subresourceRange.baseMipLevel = 0;	
		barrier.subresourceRange.levelCount = 1;	
		barrier.subresourceRange.baseArrayLayer  = 0;	
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;	
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;		

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	

		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
	
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.depth.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.depth.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create depth image view!");
		};
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.depth.image, "textures.depth.image");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.depth.imageView, "textures.depth.imageView");
	};

	void createOffscreenFramebuffers()
	{
		offscreenFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = { 
				textures.depth.imageView,
				textures.color.imageView,
			};
			
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPasses.offscreenPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = VulkanConfig::swapChainExtent.width;
			framebufferInfo.height = VulkanConfig::swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(VulkanConfig::device, &framebufferInfo, nullptr, &offscreenFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			};
		}
	}

	void setupOffscreenPass()
	{
		VkAttachmentDescription highResAttachment{
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkAttachmentDescription depthAttachment{
			.format = VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference depthAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference colorAttachmentRef{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		std::array<VkAttachmentReference, 1> colorAttachments{colorAttachmentRef};	

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = colorAttachments.data(),
			.pDepthStencilAttachment = &depthAttachmentRef,
};
		
		std::array<VkSubpassDependency, 2> dependency{};
		
		dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency[0].dstSubpass = 0;
		dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		dependency[1].srcSubpass = 0;
		dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency[1].dstStageMask =  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency[1].dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;

		std::array<VkAttachmentDescription, 2> attachments{depthAttachment, highResAttachment};
		
		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 2,
			.pDependencies = dependency.data()
		};
		
		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.offscreenPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		};
		setupDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, renderPasses.offscreenPass, "renderPasses.offscreenPass");
	};

	void setupRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 1;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{
			.format = VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference depthAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = {depthAttachment, colorAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
		setupDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, renderPasses.renderPass, "renderPasses.renderPass");
	}

	void setupUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformData);
		
		for (size_t i = 0; i < frames; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].scene, uniformBuffersMemory[i].scene, VulkanConfig::device, VulkanConfig::physicalDevice);
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].projection, uniformBuffersMemory[i].projection, VulkanConfig::device, VulkanConfig::physicalDevice);
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].projectionCube, uniformBuffersMemory[i].projectionCube, VulkanConfig::device, VulkanConfig::physicalDevice);

			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].scene, 0, bufferSize, 0, &uniformBuffersMapped[i].scene);
			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].projection, 0, bufferSize, 0, &uniformBuffersMapped[i].projection);
			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].projectionCube, 0, bufferSize, 0, &uniformBuffersMapped[i].projectionCube);
		}	
	}

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
		
		if (vkCreateSampler(VulkanConfig::device, &samplerInfo, nullptr, &sampler))
		{
			throw std::runtime_error("failed to create sampler!");	
		};
	};

	void setupQuadDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding samplerLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr};

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{samplerLayoutBinding};		

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		layoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.quad) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
		
		setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayouts.quad, "descriptorSetLayouts.quad");

		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()};
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

		//Debug
		setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, descriptorPool, "quadDescriptorPool");

		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(descriptorSetLayouts.quad);	
		VkDescriptorSetAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
			
		for (size_t i = 0; i < frames; i++)
		{

			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].quad) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create descriptor sets!");
			};

			setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSets[i].quad, std::string{"descriptorSets.quad" + i}.c_str());

			VkDescriptorImageInfo imageInfo{
				.sampler = sampler,
				.imageView = textures.color.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};	

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].quad;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}	
	};

	void setupProjectionDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr};

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{vertexLayoutBinding};		

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		layoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
		
		setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayout, "descriptorSetLayout");

		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()};
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

		std::array<VkDescriptorSetLayout, 1> layouts{};	
		layouts.fill(descriptorSetLayout);
		
		VkDescriptorSetAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
			
		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].projection) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create descriptor sets!");
			};

			setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSets[i].projection, std::string{"descriptorSets.projection" + i}.c_str());

			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[i].projection,
				.offset = 0,
				.range = sizeof(UniformData)
			};	

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].projection;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}	
	};

	void setupProjectionCubeDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr};

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{vertexLayoutBinding};		

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		layoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
		
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()};
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

		std::array<VkDescriptorSetLayout, 1> layouts{};	
		layouts.fill(descriptorSetLayout);
		
		VkDescriptorSetAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
			
		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].projectionCube) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create descriptor sets!");
			};

			setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSets[i].projectionCube, std::string{"descriptorSets.projectionCube" + i}.c_str());

			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[i].projectionCube,
				.offset = 0,
				.range = sizeof(UniformData)
			};	

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].projectionCube;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}	
	};

	void setupDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr};

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{vertexLayoutBinding};		

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		layoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
		
		setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayout, "descriptorSetLayout");

		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()};
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

		std::array<VkDescriptorSetLayout, 1> layouts{};	
		layouts.fill(descriptorSetLayout);
		
		VkDescriptorSetAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
			
		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].cube) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create descriptor sets!");
			};

			setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSets[i].cube, std::string{"descriptorSets.cube" + i}.c_str());

			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[i].scene,
				.offset = 0,
				.range = sizeof(UniformData)
			};	

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].cube;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}	
	};

	void setupQuadLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts.quad;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.quad) != VK_SUCCESS)
		{
			std::cout << "Failed to create pipeline layout!\n";
			throw std::runtime_error("failed to create pipeline layout!");
		}
		setupDebugObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, pipelineLayouts.quad, "pipelineLayouts.quad");
	}	

	void setupPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			std::cout << "Failed to create pipeline layout!\n";
			throw std::runtime_error("failed to create pipeline layout!");
		}
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

	void setupQuadPipeline()
	{
		shaderStages.clear();
		setupQuadLayout();
		addShader(SHADER_DIRECTORY + "/projectionMatrix/quad/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/projectionMatrix/quad/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	

		VkVertexInputBindingDescription bindingDescription{};
    
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{bindingDescription};
		
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.resize(4);	

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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
			.pVertexAttributeDescriptions = attributeDescriptions.data()};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE};

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
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_FALSE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = stencilOpState,
			.back = stencilOpState,
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
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling {
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

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = {0.f, 0.f, 0.f, 0.f}
		};

		VkGraphicsPipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
			.layout = pipelineLayouts.quad,
			.renderPass = renderPasses.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.quad) != VK_SUCCESS)
		{
			std::cout << "failed to create pipeline!\n";
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
		setupDebugObjectName(VK_OBJECT_TYPE_PIPELINE, pipelines.quad, "pipelines.quad");
	};

	void setupProjectionCubePipeline()
	{
		setupPipelineLayout();
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/projectionMatrix/cube/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/projectionMatrix/cube/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	

		VkVertexInputBindingDescription bindingDescription{};
    
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{bindingDescription};
		
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.resize(4);		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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
			.pVertexAttributeDescriptions = attributeDescriptions.data()};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE};

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
			.front = stencilOpState,
			.back = stencilOpState,
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
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling {
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

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = {0.f, 0.f, 0.f, 0.f}
		};

		VkGraphicsPipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
			.layout = pipelineLayout,
			.renderPass = renderPasses.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.projectionCube) != VK_SUCCESS)
		{
			std::cout << "failed to create pipeline!\n";
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
		setupDebugObjectName(VK_OBJECT_TYPE_PIPELINE, pipelines.projectionCube, "pipelines.projectionCube");
	};

	void setupCubePipeline()
	{
		setupPipelineLayout();
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/projectionMatrix/cube/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/projectionMatrix/cube/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	

		VkVertexInputBindingDescription bindingDescription{};
    
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{bindingDescription};
		
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.resize(4);		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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
			.pVertexAttributeDescriptions = attributeDescriptions.data()};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE};

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
			.front = stencilOpState,
			.back = stencilOpState,
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
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling {
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

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = {0.f, 0.f, 0.f, 0.f}
		};

		VkGraphicsPipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
			.layout = pipelineLayout,
			.renderPass = renderPasses.offscreenPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.cubePipeline) != VK_SUCCESS)
		{
			std::cout << "failed to create pipeline!\n";
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
		setupDebugObjectName(VK_OBJECT_TYPE_PIPELINE, pipelines.cubePipeline, "pipelines.cubePipeline");
	};

	void setupProjectionPipeline()
	{
		setupPipelineLayout();
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/projectionMatrix/projection/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/projectionMatrix/projection/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	

		VkVertexInputBindingDescription bindingDescription{};
    
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{bindingDescription};
		
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.resize(4);		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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
			.pVertexAttributeDescriptions = attributeDescriptions.data()};

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
			.primitiveRestartEnable = VK_FALSE};

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
			.front = stencilOpState,
			.back = stencilOpState,
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
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling {
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

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = {0.f, 0.f, 0.f, 0.f}
		};

		VkGraphicsPipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
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
			.layout = pipelineLayout,
			.renderPass = renderPasses.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.projection) != VK_SUCCESS)
		{
			std::cout << "failed to create pipeline!\n";
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
		setupDebugObjectName(VK_OBJECT_TYPE_PIPELINE, pipelines.projection, "pipelines.projection");
	};

	void createSwapChainFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = { 
				textures.projectionDepth.imageView,
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

			if (vkCreateFramebuffer(VulkanConfig::device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			};
		}
	}

	void init(GLFWwindow* window)
	{
		IVulkanApp::init(window);	
		setupOffscreenPass();
		setupRenderPass();
		setupProjectionVertices();
		createVertexBuffer(projectionVertices, projection.buffer, projection.memory);
		createProjectionIndexBuffer();
		setupSampler();
		setupColor();
		setupDepth();
		setupProjectionDepth();
		setupUniformBuffers();
		setupQuadDescriptorSets();
		setupProjectionDescriptorSets();
		setupProjectionCubeDescriptorSets();
		setupDescriptorSets();
		setupCubePipeline();
		setupProjectionPipeline();
		setupProjectionCubePipeline();
		setupQuadPipeline();
		createOffscreenFramebuffers();
		createSwapChainFramebuffers();
	};

	void updateUniformBuffer(uint32_t currentImage)
	{
		glm::vec3 cubePosition{0., 0., 5.};
		UniformData uniformData;
		
		uniformData.model = glm::translate(glm::mat4(1.), cubePosition);		
		uniformData.view = mainCamera.getViewMatrix();
		uniformData.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, 5.f);
		uniformData.proj[1][1] *= -1.;
		
		memcpy(uniformBuffersMapped[currentImage].scene, &uniformData, sizeof(uniformData));
	
		// Begin RenderPass Camera
		uniformData.view = renderCamera.getViewMatrix();
		uniformData.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
		uniformData.proj[1][1] *= -1.;
		memcpy(uniformBuffersMapped[currentImage].projectionCube, &uniformData, sizeof(uniformData));

		uniformData.model = mainCamera.getViewMatrix();		
		uniformData.model = glm::rotate(uniformData.model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));		
		memcpy(uniformBuffersMapped[currentImage].projection, &uniformData, sizeof(uniformData));
	};

	void processInput(GLFWwindow * window)
	{
		renderCamera.cameraSpeed = 10.f * lastFrameTime;
		mainCamera.cameraSpeed = 10.f * lastFrameTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			mainCamera.move(FORWARD);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			mainCamera.move(BACKWARD);
		};
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			mainCamera.move(LEFT);
		};
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			mainCamera.move(RIGHT);
		};
	};

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
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
		renderPassInfo.renderPass = renderPasses.offscreenPass;
		renderPassInfo.framebuffer = offscreenFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = VulkanConfig::swapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].depthStencil = {1.0f, 0};
		clearValues[1].color = {{0.f, 0.f, 0.f, 1.f}};

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

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
		VkBuffer vertexProjectionBuffers[] = { projection.buffer };
		
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.cubePipeline);	
		
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame].cube, 0, nullptr);
		
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		renderPassInfo.renderPass = renderPasses.renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

		viewport.width = static_cast<float>(VulkanConfig::swapChainExtent.width);
		viewport.height = static_cast<float>(VulkanConfig::swapChainExtent.height);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.projectionCube);	
		
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame].projectionCube, 0, nullptr);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.projection);	
		
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame].projection, 0, nullptr);
		
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexProjectionBuffers, offsets);
	 	vkCmdBindIndexBuffer(commandBuffer, projection.indexBuffer, 0, VK_INDEX_TYPE_UINT32);		
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(projectionIndices.size()), 1, 0, 0, 0);


		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.quad);	

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.quad, 0, 1, &descriptorSets[currentFrame].quad, 0, nullptr);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(DeferredRendering::cubeIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

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

		vkDestroyDescriptorSetLayout(VulkanConfig::device, descriptorSetLayout, nullptr);

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
		setupColor();
		setupDepth();
		setupProjectionDepth();
		setupQuadDescriptorSets();

		createOffscreenFramebuffers();
		createSwapChainFramebuffers();
	}

	void drawFrame(GLFWwindow * window)
	{
		mainCamera.update();
		renderCamera.update();
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		vkWaitForFences(VulkanConfig::device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

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

		vkResetFences(VulkanConfig::device, 1, &inFlightFences[currentFrame]);

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

	void moveCamera(double xpos, double ypos)
	{
		camera.move(xpos, ypos);
		mainCamera.move(xpos, ypos);
	}
};

