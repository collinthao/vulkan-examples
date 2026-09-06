#pragma once 

#include "../../core/renderer/vulkanApp/vulkanApp.h"

class ParallaxMapping : public IVulkanApp
{
	public:
	ParallaxMapping() = default;
	constexpr static int frames = 2;
	
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;	
	std::vector<VertexNormal> quadVerticesNormal{};
	static inline const std::vector<uint32_t> quadIndices = {
	    3, 1, 0,
	    1, 3, 2,
	};

	glm::vec3 tangent1, bitangent1;
	glm::vec3 tangent2, bitangent2;

	struct LightUniform
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct ObjectUniform
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec3 cameraPos;
		alignas(16) glm::vec3 lightPos;
	};

	struct 
	{
		VkRenderPass renderPass;	
	} renderPasses{};
	
	struct
	{
		VkPipelineLayout cube;		
		VkPipelineLayout light;		
	} pipelineLayouts;
	
	struct
	{
		VkPipeline cube;
		VkPipeline light;
	} pipelines;	

	struct
	{
		VkDescriptorSetLayout cube;
		VkDescriptorSetLayout light;
	} descriptorSetLayouts;

	struct DescriptorSets
	{
		VkDescriptorSet cube;
		VkDescriptorSet light;
	};

	std::array<DescriptorSets, frames> descriptorSets;

	struct UniformBuffers 
	{
		VkBuffer cube;
		VkBuffer light;
	};
	
	struct UniformBuffersMapped
	{
		void * cube;
		void * light;
	};
	
	struct UniformBuffersMemory
	{
		VkDeviceMemory cube;
		VkDeviceMemory light;
	};

	struct IndexBuffer
	{
		VkBuffer buffer;		
		VkDeviceMemory memory;
	};

	struct
	{
		IndexBuffer quadIndex;
		IndexBuffer quadNormal;
	} indexBuffers;

	std::array<UniformBuffers, frames> uniformBuffers;	
	std::array<UniformBuffersMapped, frames> uniformBuffersMapped;	
	std::array<UniformBuffersMemory, frames> uniformBuffersMemory;	

	VkSampler sampler;

	struct Texture
	{
		VkImageView imageView;
		VkImage     image;
		VkDeviceMemory imageMemory;
	};		

	struct
	{
		Texture brick;	
		Texture brickNormal;	
		Texture brickDisp;	
		Texture depth;
		Texture resolved;
	} texture;	

	void calculateTangentAndBitangents()
	{
		glm::vec3 pos1 = IVulkanApp::cubeVertices[3].pos;
		glm::vec3 pos2 = IVulkanApp::cubeVertices[0].pos;
		glm::vec3 pos3 = IVulkanApp::cubeVertices[1].pos;
		glm::vec3 pos4 = IVulkanApp::cubeVertices[2].pos;		
		glm::vec3 normal = glm::vec3{0., 0., 1.};
		
		glm::vec2 uv1(0.0, 1.0);
		glm::vec2 uv2(0.0, 0.0);
		glm::vec2 uv3(1.0, 0.0);
		glm::vec2 uv4(1.0, 1.0);

		// First Triangle
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1; //(0.0, -1.0)
		glm::vec2 deltaUV2 = uv3 - uv1; //(1.0, -1.0)
				
		float f = 1.0f/(deltaUV1.x*deltaUV2.y-deltaUV2.x*deltaUV1.y);
		
		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		
		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		// Second Triangle
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;
				
		f = 1.0f/(deltaUV1.x*deltaUV2.y-deltaUV2.x*deltaUV1.y);
		
		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		
		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		tangent1 = glm::normalize(tangent1);	
		bitangent1 =glm::normalize(bitangent1);
		tangent2 = glm::normalize(tangent2);	
		bitangent2 =glm::normalize(bitangent2);

		quadVerticesNormal = {
			{pos1, normal, tangent1, bitangent1, uv1},	
			{pos2, normal, tangent1, bitangent1, uv2},	
			{pos3, normal, tangent1, bitangent1, uv3},	
			{pos1, normal, tangent2, bitangent2, uv1},	
			{pos3, normal, tangent2, bitangent2, uv3},	
			{pos4, normal, tangent2, bitangent2, uv4}};
	}	

	void setUpRenderPass()
	{
		VkAttachmentDescription colorAttachment{
			.format = swapChainImageFormat,
				.samples = VulkanConfig::msaaSamples,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};	

		VkAttachmentDescription colorAttachmentResolve{
			.format = swapChainImageFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentDescription depthAttachment{
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
				.samples = VulkanConfig::msaaSamples,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference colorAttachmentRef{
			.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkAttachmentReference depthAttachmentRef{
			.attachment = 1,
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference colorAttachmentResolveRef{
			.attachment = 2,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttachmentRef,
				.pResolveAttachments = &colorAttachmentResolveRef,
				.pDepthStencilAttachment = &depthAttachmentRef,
		};

		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		};	

		std::array<VkAttachmentDescription, 3> attachments{colorAttachment, depthAttachment, colorAttachmentResolve};

		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.subpassCount = 1,
				.pSubpasses = &subpass,
				.dependencyCount = 1,
				.pDependencies = &dependency
		};

		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		};
	};

	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments = { 
				texture.resolved.imageView,
				texture.depth.imageView,
				swapChainImageViews[i] 
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

	void setupUniformBuffers()
	{
		VkDeviceSize objectBufferSize = sizeof(ObjectUniform);	
		VkDeviceSize lightBufferSize = sizeof(LightUniform);	

		for (size_t i = 0; i < frames; i++)
		{
			Buffer::create(objectBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].cube, uniformBuffersMemory[i].cube, VulkanConfig::device, VulkanConfig::physicalDevice);
			
			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].cube, 0, objectBufferSize, 0, &uniformBuffersMapped[i].cube);
			
			//Light
			Buffer::create(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].light, uniformBuffersMemory[i].light, VulkanConfig::device, VulkanConfig::physicalDevice);
			
			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].light, 0, lightBufferSize, 0, &uniformBuffersMapped[i].light);

		};
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

	void setupResolved()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = swapChainImageFormat,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VulkanConfig::msaaSamples,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &texture.resolved.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, texture.resolved.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &texture.resolved.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for resolved image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, texture.resolved.image, texture.resolved.imageMemory, 0);
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = texture.resolved.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapChainImageFormat
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &texture.resolved.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create resolved image view!");
		};
	};	

	void setupDepth()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VulkanConfig::msaaSamples,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &texture.depth.image))
		{
			throw std::runtime_error("failed to create image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, texture.depth.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &texture.depth.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for depth image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, texture.depth.image, texture.depth.imageMemory, 0);
	
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture.depth.image	
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
			.image = texture.depth.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &texture.depth.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create depth image view!");
		};
	};

	void loadBrickDispTexture()
	{
		const std::string path = ROOT_DIR + "/resource/textures/brickWall2/brickWall2_disp.jpg";
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
		
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
		
		stbi_image_free(pixels);
		
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = static_cast<uint32_t>(mipLevels),
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &texture.brickDisp.image))
		{
			throw std::runtime_error("failed to create image!");
		};	
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, texture.brickDisp.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		
		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &texture.brickDisp.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");	
		};
		
		vkBindImageMemory(VulkanConfig::device, texture.brickDisp.image, texture.brickDisp.imageMemory, 0);
		
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture.brickDisp.image
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;		
		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		
		VkBufferImageCopy region{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageOffset = {0,0,0},
			.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}
		};	

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			texture.brickDisp.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);	
	
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;	
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;	
		vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	}

	void loadBrickNormalTexture()
	{
		const std::string path = ROOT_DIR + "/resource/textures/brickWall2/brickWall2_normal.jpg";
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
		
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
		
		stbi_image_free(pixels);
		
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = static_cast<uint32_t>(mipLevels),
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &texture.brickNormal.image))
		{
			throw std::runtime_error("failed to create image!");
		};	
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, texture.brickNormal.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		
		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &texture.brickNormal.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");	
		};
		
		vkBindImageMemory(VulkanConfig::device, texture.brickNormal.image, texture.brickNormal.imageMemory, 0);
		
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture.brickNormal.image
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;		
		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		
		VkBufferImageCopy region{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageOffset = {0,0,0},
			.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}
		};	

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			texture.brickNormal.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);	
	
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;	
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;	
		vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	}

	void loadBrickTexture()
	{
		const std::string path = ROOT_DIR + "/resource/textures/brickWall2/brickWall2.jpg";
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
		
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
		
		stbi_image_free(pixels);
		
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = static_cast<uint32_t>(mipLevels),
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &texture.brick.image))
		{
			throw std::runtime_error("failed to create image!");
		};	
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, texture.brick.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		
		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &texture.brick.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");	
		};
		
		vkBindImageMemory(VulkanConfig::device, texture.brick.image, texture.brick.imageMemory, 0);
		
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = texture.brick.image
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;		
		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		
		VkBufferImageCopy region{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageOffset = {0,0,0},
			.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}
		};	

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			texture.brick.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);	
	
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;	
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;	
		vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	}	

	void setupBrickDispImageView()
	{
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = texture.brickDisp.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB};		

			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &texture.brickDisp.imageView))
		{
			throw std::runtime_error("failed to create image view!");	
		};
	};

	void setupBrickNormalImageView()
	{
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = texture.brickNormal.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB};		

			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &texture.brickNormal.imageView))
		{
			throw std::runtime_error("failed to create image view!");	
		};
	};

	void setupBrickImageView()
	{
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = texture.brick.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB};		

			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &texture.brick.imageView))
		{
			throw std::runtime_error("failed to create image view!");	
		};
	};

	void setupSamplers()
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

	void setupLightDescriptorSets()
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
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.light))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
		layouts.fill(descriptorSetLayouts.light);	
		
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
		
		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].light) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};
			
			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[i].light,
				.offset = 0,
				.range = sizeof(LightUniform)
			};
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].light;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void setupCubeDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding fragmentLayoutBinding{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding normalLayoutBinding{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding dispLayoutBinding{
			.binding = 3,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 4> setLayoutBindings{vertexLayoutBinding, fragmentLayoutBinding, normalLayoutBinding, dispLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.cube))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 4> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 4;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[1].descriptorCount = static_cast<uint32_t>(frames) * 4;

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[2].descriptorCount = static_cast<uint32_t>(frames) * 4;

		poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[3].descriptorCount = static_cast<uint32_t>(frames) * 4;



		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 4,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		std::array<VkDescriptorSetLayout, 4> layouts{};
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
			
			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[i].cube,
				.offset = 0,
				.range = sizeof(ObjectUniform)
			};
			
			VkDescriptorImageInfo imageInfo
			{
				.sampler = sampler,
				.imageView = texture.brick.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			VkDescriptorImageInfo imageNormalInfo
			{
				.sampler = sampler,
				.imageView = texture.brickNormal.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			VkDescriptorImageInfo imageDispInfo
			{
				.sampler = sampler,
				.imageView = texture.brickDisp.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
	
			std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].cube;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i].cube;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = descriptorSets[i].cube;
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageNormalInfo;
			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = descriptorSets[i].cube;
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pImageInfo = &imageDispInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void setupQuadIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(quadIndices[0]) * quadIndices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, quadIndices.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffers.quadIndex.buffer, indexBuffers.quadIndex.memory, VulkanConfig::device, VulkanConfig::physicalDevice);
		copyBuffer(stagingBuffer, indexBuffers.quadIndex.buffer, bufferSize);
		
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

	void setupLightPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.light,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.light) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
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

	void setupLightPipeline()
	{
		setupLightPipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/parallaxMapping/pointLight/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/parallaxMapping/pointLight/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
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
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VulkanConfig::msaaSamples,
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
			.layout = pipelineLayouts.light,
			.renderPass = renderPasses.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.light) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupCubePipeline()
	{
		setupCubePipelineLayout();

		addShader(SHADER_DIRECTORY + "/parallaxMapping/object/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/parallaxMapping/object/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
		VkVertexInputBindingDescription bindingDescription{
			.binding = 0,
			.stride = sizeof(VertexNormal),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};		
		
		std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{bindingDescription};
		
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions;

	
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexNormal, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexNormal, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(VertexNormal, tangent);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(VertexNormal, bitangent);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(VertexNormal, texCoord);
	
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
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VulkanConfig::msaaSamples,
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
			.layout = pipelineLayouts.cube,
			.renderPass = renderPasses.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.cube) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};
	
	void updateUniformBuffer(uint32_t currentImage)
	{
		// Light	
		glm::vec3 lightPos = glm::vec3(.5, 1., .3);
		LightUniform lightUniform;
		lightUniform.model = glm::mat4(1.);		
		lightUniform.model = glm::translate(lightUniform.model, lightPos);
		lightUniform.model = glm::scale(lightUniform.model, glm::vec3(.1));
		lightUniform.view = camera.getViewMatrix();
		lightUniform.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
		lightUniform.proj[1][1] *= -1.;

		memcpy(uniformBuffersMapped[currentImage].light, &lightUniform, sizeof(lightUniform));

		ObjectUniform objectUniform;
		
		objectUniform.model = glm::translate(glm::mat4(1.), glm::vec3(1., 0., 1.));		
		objectUniform.view = camera.getViewMatrix();
		objectUniform.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
		objectUniform.cameraPos = camera.cameraPos;
		objectUniform.lightPos = lightPos; 

		objectUniform.proj[1][1] *= -1.;
		
		memcpy(uniformBuffersMapped[currentImage].cube, &objectUniform, sizeof(objectUniform));
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
		clearValues[0].color = {{.0f, .0f, .0f, 1.f}};
		clearValues[1].depthStencil = {1.f, 0};

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
		
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &indexBuffers.quadNormal.buffer, offsets);
		
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(quadVerticesNormal.size()), 1, 0, 0);	

//		vkCmdBindIndexBuffer(commandBuffer, indexBuffers.quadIndex.buffer, 0, VK_INDEX_TYPE_UINT32);		

//		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ParallaxMapping::quadIndices.size()), 1, 0, 0, 0);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.light);	

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.light, 0, 1, &descriptorSets[currentFrame].light, 0, nullptr);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ParallaxMapping::cubeIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	};

	void setupCubeResources()
	{
		setupCubeDescriptorSets();	
		setupCubePipeline();
	};

	void setupLightResources()
	{
		setupLightDescriptorSets();	
		setupLightPipeline();
	};
	
	void init(GLFWwindow* window)
	{
		IVulkanApp::init(window);	
		calculateTangentAndBitangents();
		createVertexBuffer<VertexNormal>(quadVerticesNormal, indexBuffers.quadNormal.buffer, indexBuffers.quadNormal.memory);
		setUpRenderPass();
		setupSamplers();
		loadBrickTexture();
		loadBrickNormalTexture();
		loadBrickDispTexture();
		setupResolved();
		setupDepth();
		setupBrickImageView();
		setupBrickNormalImageView();
		setupBrickDispImageView();
		setupUniformBuffers();
		setupCubeResources();
		setupLightResources();
		setupQuadIndexBuffer();
		createFramebuffers();
	};

	void cleanup(GLFWwindow * window)
	{
		cleanupSwapChain();

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		for (size_t i = 0; i < frames; i++)
		{
			vkDestroyBuffer(VulkanConfig::device, uniformBuffers[i].cube, nullptr);
			vkFreeMemory(VulkanConfig::device, uniformBuffersMemory[i].cube, nullptr);
		}		

		vkDestroyDescriptorSetLayout(VulkanConfig::device, descriptorSetLayouts.cube, nullptr);

		vkDestroyBuffer(VulkanConfig::device, indexBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, indexBufferMemory, nullptr);
		
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);

		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);

		vkDestroyImage(VulkanConfig::device, texture.brick.image, nullptr);
		vkFreeMemory(VulkanConfig::device, texture.brick.imageMemory, nullptr);

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
		setupResolved();
		setupDepth();

		createFramebuffers();
	}

};
