#pragma once 

#include "../../core/renderer/vulkanApp/vulkanApp.h"

class IBL : public IVulkanApp
{
	public:
	IBL() = default;
	constexpr static int frames = 2;
	constexpr static int SPHERE_COUNT = 49;

	constexpr static int nrRows = 7;
	constexpr static int nrColumns= 7;
	constexpr static float spacing = 2.5;

	Model * model;

	std::vector<Vertex> sphereVertices{};
	std::vector<uint32_t> sphereIndices{};
	VkBuffer sphereBuffer;
	VkDeviceMemory sphereBufferMemory;
	VkBuffer sphereIndexBuffer, cubeIndexBuffer;
	VkDeviceMemory sphereIndexBufferMemory, cubeIndexBufferMemory;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;	
		
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	glm::mat4 captureViews[6] = 
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	};

	static inline const std::vector<uint32_t> cubeIndices = {
	    0, 2, 1,
	    2, 0, 3,

	    7, 4, 6,
	    5, 6, 4,

	   12,14,13,
	   11, 8,10,

	    9,10, 8,
	   14,12,15,

	   17,18,16,
	   19,16,18,

	   22,20,23,
	   20,22,21,
	};

	struct SphereUniform
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec4 cameraPos;
		alignas(16) float roughness;
		alignas(16) float metallic;
	};

	struct ObjectUniform
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct 
	{
		VkFramebuffer offscreen;	
	} framebuffers{};

	struct 
	{
		VkRenderPass present;	
		VkRenderPass offscreen;	
	} renderPasses{};
	
	struct
	{
		VkPipelineLayout quad;		
		VkPipelineLayout sphere;		
		VkPipelineLayout cube;		
		VkPipelineLayout equirectangularCubemap;		
		VkPipelineLayout model;		
		VkPipelineLayout irradiance;		
	} pipelineLayouts;
	
	struct
	{
		VkPipeline quad;
		VkPipeline sphere;
		VkPipeline cube;
		VkPipeline equirectangularCubemap;
		VkPipeline model;
		VkPipeline irradiance;
	} pipelines;	

	struct
	{
		VkDescriptorSetLayout quad;
		VkDescriptorSetLayout cube;
		VkDescriptorSetLayout equirectangularCubemap;
		VkDescriptorSetLayout sphere;
		VkDescriptorSetLayout irradiance;
	} descriptorSetLayouts;

	struct DescriptorSets
	{
		std::array<VkDescriptorSet, 6> cube;
		std::array<VkDescriptorSet, 6> irradiance;
		std::vector<VkDescriptorSet> sphere;
		VkDescriptorSet quad;
		VkDescriptorSet equirectangularCubemap;
	};

	std::array<DescriptorSets, frames> descriptorSets;

	struct UniformBuffers 
	{
		std::array<VkBuffer, 6> cube;
		std::vector<VkBuffer> sphere;
		VkBuffer cubemap;
	};
	
	struct UniformBuffersMapped
	{
		std::array<void *, 6> cube;
		std::vector<void *> sphere;
		void * cubemap;
	};
	
	struct UniformBuffersMemory
	{
		std::array<VkDeviceMemory, 6> cube;
		std::vector<VkDeviceMemory> sphere;
		VkDeviceMemory cubemap;
	};

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
		Texture wood;
		Texture depth;
		Texture hdr;
		Texture offscreen;
		Texture offscreenDepth;
		Texture equirectangularCubemap;
		Texture irradianceCubemap;
	} textures;

	void createIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(cubeIndices[0]) * cubeIndices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, cubeIndices.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cubeIndexBuffer, cubeIndexBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);
		copyBuffer(stagingBuffer, cubeIndexBuffer, bufferSize);
		
		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	}

	void loadHDRImage()
	{
		int texWidth, texHeight, texChannels;
//		float * pixels;
		stbi_uc * pixels;

		const std::string path{ROOT_DIR + "/resource/textures/hdr/newport_loft.hdr"};		
		//pixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);	
		pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);	

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");	
		};

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		VkDeviceSize layerSize =  imageSize;

		int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

		void* data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, imageSize, 0, &data);

		memcpy(static_cast<char*>(data), pixels, static_cast<size_t>(layerSize));

		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);
		stbi_image_free(pixels);

		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			//.format = VK_FORMAT_R16G16B16A16_SFLOAT,
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

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.hdr.image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		};	
			
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.hdr.image, "textures.hdr.image");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.hdr.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		
		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.hdr.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");	
		};
		
		vkBindImageMemory(VulkanConfig::device, textures.hdr.image, textures.hdr.imageMemory, 0);
		
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = textures.hdr.image};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = static_cast<uint32_t>(mipLevels);
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
			textures.hdr.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);	
	
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

		// this is transitioning the layout back to shader read
		Image::generateMipmaps(textures.hdr.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 1, VulkanConfig::device, VulkanConfig::physicalDevice, VulkanConfig::graphicsAndComputeQueue);

		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);


		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.hdr.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			//.format = VK_FORMAT_R16G16B16A16_SFLOAT,
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.hdr.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture.depth image view!");
		};
	}	

	void setupModelDescriptorSets()
	{
		model->setupDescriptorSets<ObjectUniform>();	
	}	

	void loadModel()
	{
		model = new Model("/resource/models/skull/12140_Skull_v3_L2.obj", VulkanConfig::device, VulkanConfig::physicalDevice, VulkanConfig::graphicsAndComputeQueue);
		model->setupUniforms<ObjectUniform>();
	}

	void setupRenderPass()
	{
		VkAttachmentDescription colorAttachmentResolve{
		.format = swapChainImageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentDescription depthAttachment{
			.format = VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference depthAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference colorAttachmentRef{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pDepthStencilAttachment = &depthAttachmentRef,
		};	

		std::array<VkSubpassDependency, 2> dependencies{};
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = {depthAttachment, colorAttachmentResolve};

		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),	
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = dependencies.data()
		};
		
		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.present))
		{
			throw std::runtime_error("Failed to create render pass!");
		};
		setupDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, renderPasses.present, "renderPasses.present");
	};

	void setupOffscreenPass()
	{
		VkAttachmentDescription highResAttachment{
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkAttachmentDescription lowResAttachment{
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkAttachmentDescription depthAttachment
		{
			.format = VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference depthAttachmentRef
		{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference offscreenColorAttachmentRef
		{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		std::array<VkAttachmentReference, 1> colorAttachments
		{
			offscreenColorAttachmentRef
		};	

		VkSubpassDescription subpass
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = colorAttachments.data(),
			.pDepthStencilAttachment = &depthAttachmentRef
		};
		
		std::array<VkSubpassDependency, 1> dependency{};
		
		dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency[0].dstSubpass = 0;
		dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments
		{
			depthAttachment, 
			lowResAttachment
		};
		
		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = dependency.data()
		};
		
		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.offscreen) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		};
	};
		
	void createSwapChainFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = { 
				textures.depth.imageView,
				swapChainImageViews[i]
			};
			
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPasses.present;
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

	void createOffscreenFramebuffers()
	{
		std::array<VkImageView, 2> attachments = { 
			textures.offscreenDepth.imageView,
			textures.offscreen.imageView,
		};
		
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPasses.offscreen;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = 512;
		framebufferInfo.height = 512;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(VulkanConfig::device, &framebufferInfo, nullptr, &framebuffers.offscreen) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		};
	}

	void setupUniformBuffers()
	{
		VkDeviceSize sphereBufferSize = sizeof(SphereUniform);	
		VkDeviceSize objectBufferSize = sizeof(ObjectUniform);	

		//Sphere
		uniformBuffers[0].sphere.resize(SPHERE_COUNT);
		uniformBuffers[1].sphere.resize(SPHERE_COUNT);
		uniformBuffersMemory[0].sphere.resize(SPHERE_COUNT);
		uniformBuffersMemory[1].sphere.resize(SPHERE_COUNT);
		uniformBuffersMapped[0].sphere.resize(SPHERE_COUNT);
		uniformBuffersMapped[1].sphere.resize(SPHERE_COUNT);

		for (size_t i = 0; i < frames; i++)
		{
			Buffer::create(objectBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].cubemap, uniformBuffersMemory[i].cubemap, VulkanConfig::device, VulkanConfig::physicalDevice);
			
			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].cubemap, 0, objectBufferSize, 0, &uniformBuffersMapped[i].cubemap);
		};

		for (size_t i = 0; i < 6; i++)
		{
			Buffer::create(objectBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[0].cube[i], uniformBuffersMemory[0].cube[i], VulkanConfig::device, VulkanConfig::physicalDevice);
			
			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[0].cube[i], 0, objectBufferSize, 0, &uniformBuffersMapped[0].cube[i]);
		};

		for (size_t j = 0; j < SPHERE_COUNT; j++)
		{
			for (size_t i = 0; i < frames; i++)
			{
				Buffer::create(sphereBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].sphere[j], uniformBuffersMemory[i].sphere[j], VulkanConfig::device, VulkanConfig::physicalDevice);
				
				vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].sphere[j], 0, sphereBufferSize, 0, &uniformBuffersMapped[i].sphere[j]);
			};
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

	void setupIrradianceCubemap()
	{
		VkImageCreateInfo imageInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = 1,
			.arrayLayers = 6,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(512);
		imageInfo.extent.height = static_cast<uint32_t>(512);
		imageInfo.extent.depth = 1;

		if (vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.irradianceCubemap.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	
	
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.irradianceCubemap.image, "textures.irradianceCubemap.image");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.irradianceCubemap.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.irradianceCubemap.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for position image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.irradianceCubemap.image, textures.irradianceCubemap.imageMemory, 0);
		VkImageViewCreateInfo viewInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.irradianceCubemap.image,
			.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
			.format = VK_FORMAT_R8G8B8A8_SRGB
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.irradianceCubemap.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
		};
	};	

	void setupEquirectangularCubemap()
	{
		VkImageCreateInfo imageInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = 1,
			.arrayLayers = 6,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(512);
		imageInfo.extent.height = static_cast<uint32_t>(512);
		imageInfo.extent.depth = 1;

		if (vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.equirectangularCubemap.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	
	
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.equirectangularCubemap.image, "textures.equirectangularCubemap.image");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.equirectangularCubemap.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.equirectangularCubemap.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for position image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.equirectangularCubemap.image, textures.equirectangularCubemap.imageMemory, 0);
		VkImageViewCreateInfo viewInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.equirectangularCubemap.image,
			.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
			.format = VK_FORMAT_R8G8B8A8_SRGB
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.equirectangularCubemap.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
		};
	};	

	void setupOffscreenColorAttachment()
	{
		VkImageCreateInfo imageInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(512);
		imageInfo.extent.height = static_cast<uint32_t>(512);
		imageInfo.extent.depth = 1;

		if (vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.offscreen.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	

		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.offscreen.image, "textures.offscreen.image");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.offscreen.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.offscreen.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for position image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.offscreen.image, textures.offscreen.imageMemory, 0);
		VkImageViewCreateInfo viewInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.offscreen.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.offscreen.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
		};
	};	

	void setupOffscreenDepth()
	{
		VkImageCreateInfo imageInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		
		imageInfo.extent.width = 512; 
		imageInfo.extent.height = 512;
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.offscreenDepth.image))
		{
			throw std::runtime_error("failed to create image!");
		};	

		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.offscreenDepth.image, "textures.offscreenDepth.image");
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.offscreenDepth.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.offscreenDepth.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for texture.depth image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.offscreenDepth.image, textures.offscreenDepth.imageMemory, 0);
	
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = textures.offscreenDepth.image
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
	
		VkImageViewCreateInfo viewInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.offscreenDepth.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.offscreenDepth.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture.depth image view!");
		};
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

	void loadTexture()
	{
		const std::string path = ROOT_DIR + "/resource/textures/wood.png";
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
		
		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image! Path: " + path);	
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

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.wood.image))
		{
			throw std::runtime_error("failed to create image!");
		};	
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.wood.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		
		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.wood.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");	
		};
		
		vkBindImageMemory(VulkanConfig::device, textures.wood.image, textures.wood.imageMemory, 0);
		
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = textures.wood.image};

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
			textures.wood.image,
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

	void setupImageViews()
	{
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.wood.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB};		
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.wood.imageView))
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

	void setupIrradianceDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding equirectangularLayoutBinding
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings
		{
			vertexLayoutBinding, 
			equirectangularLayoutBinding
		};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.irradiance))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(6);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(6);

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(6),
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(descriptorSetLayouts.irradiance);	
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};

		for (size_t i = 0; i < 6; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[0].irradiance[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};
			
			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[0].cube[i],
				.offset = 0,
				.range = sizeof(ObjectUniform)
			};
		
			VkDescriptorImageInfo imageInfo
			{
				.sampler = sampler,
				.imageView = textures.equirectangularCubemap.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	
			};
			
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[0].irradiance[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[0].irradiance[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void setupEquirectangularCubemapDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding equirectangularLayoutBinding
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings
		{
			vertexLayoutBinding, 
			equirectangularLayoutBinding
		};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.equirectangularCubemap))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(frames);

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames),
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(descriptorSetLayouts.equirectangularCubemap);	
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};

		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].equirectangularCubemap) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};
			
			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[i].cubemap,
				.offset = 0,
				.range = sizeof(ObjectUniform)
			};
		
			VkDescriptorImageInfo imageInfo
			{
				.sampler = sampler,
				.imageView = textures.equirectangularCubemap.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	
			};
			
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].equirectangularCubemap;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i].equirectangularCubemap;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void setupEquirectangularCubeDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding hdrLayoutBinding
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings{vertexLayoutBinding, hdrLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.cube))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames * 6);

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(frames * 6);

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames * 6),
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

		for (size_t i = 0; i < 6; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[0].cube[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};
			
			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[0].cube[i],
				.offset = 0,
				.range = sizeof(ObjectUniform)
			};
		
			VkDescriptorImageInfo imageInfo
			{
				.sampler = sampler,
				.imageView = textures.hdr.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	
			};
			
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[0].cube[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[0].cube[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

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
					.range = sizeof(SphereUniform)
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

	void setupQuadPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.quad,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.quad) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupEquirectangularCubemapPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.equirectangularCubemap,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.equirectangularCubemap) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupEquirectangularCubePipelineLayout()
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

	void setupModelPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &model->layout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.model) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupIrradiancePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.irradiance,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.irradiance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupIrradiancePipeline()
	{
		setupIrradiancePipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/ibl/irradiance/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/ibl/irradiance/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
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
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
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
			.layout = pipelineLayouts.irradiance,
			.renderPass = renderPasses.offscreen,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.irradiance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupEquirectangularCubemapPipeline()
	{
		setupEquirectangularCubemapPipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/ibl/cubemap/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/ibl/cubemap/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
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
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
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
			.layout = pipelineLayouts.equirectangularCubemap,
			.renderPass = renderPasses.present,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.equirectangularCubemap) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupEquirectangularCubePipeline()
	{
		setupEquirectangularCubePipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/ibl/hdr/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/ibl/hdr/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
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
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
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
			.renderPass = renderPasses.offscreen,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.cube) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupSpherePipeline()
	{
		setupSpherePipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/ibl/sphere/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/ibl/sphere/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
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
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
			.layout = pipelineLayouts.sphere,
			.renderPass = renderPasses.present,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.sphere) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupModelPipeline()
	{
		setupModelPipelineLayout();

		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/deferred/object/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/deferred/object/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
		VkVertexInputBindingDescription bindingDescription
		{
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

		VkPipelineDepthStencilStateCreateInfo depthStencil 
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.minDepthBounds = 0.f,
			.maxDepthBounds = 1.f
		};

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamicState 
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineRasterizationStateCreateInfo rasterizer
		{
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

		std::array<VkPipelineColorBlendAttachmentState, 4> colorBlendAttachments{colorBlendAttachment, colorBlendAttachment, colorBlendAttachment, colorBlendAttachment};

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 4,
			.pAttachments = colorBlendAttachments.data(),
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
			.layout = pipelineLayouts.model,
			.renderPass = renderPasses.offscreen,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.model) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void updateUniformBuffer(uint32_t currentImage)
	{
		// Sphere	
		glm::mat4 model = glm::mat4(1.);
		SphereUniform sphereUniform;
		int sphereIndex = 0;

		for (int row = 0; row < nrRows; ++row)
		{
		    sphereUniform.metallic = (float)row/(float)nrRows;
		    for (int col = 0; col < nrColumns; ++col)
		    {
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(
			    (float)(col - (nrColumns / 2)) * spacing,
			    (float)(row - (nrRows / 2)) * spacing,
			    0.0f
			));
			sphereUniform.model = model;
//			sphereUniform.model = glm::scale(sphereUniform.model, glm::vec3(.2));
			sphereUniform.view = camera.getViewMatrix();
			sphereUniform.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
			sphereUniform.proj[1][1] *= -1.;
			
			sphereUniform.cameraPos = glm::vec4(camera.cameraPos.x, camera.cameraPos.y, camera.cameraPos.z,1.);
			sphereUniform.roughness = glm::clamp((float)col/(float)nrColumns, 0.05f, 1.0f);

			memcpy(uniformBuffersMapped[currentImage].sphere[sphereIndex], &sphereUniform, sizeof(sphereUniform));
			sphereIndex++;	
		    }
		}
		
		ObjectUniform objectUniform;
		objectUniform.model = glm::mat4(1.);
		objectUniform.view = glm::mat4(glm::mat3(camera.getViewMatrix()));	
		objectUniform.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
		objectUniform.proj[1][1] *= -1;
		
		memcpy(uniformBuffersMapped[currentImage].cubemap, &objectUniform, sizeof(objectUniform));
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

	void recordIrradiance()
	{
		for (int i = 0; i < 6; i++)
		{
			VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].depthStencil = {1.f, 0};
			clearValues[1].color = {{.0f, .0f, .0f, 1.f}};

			VkRect2D renderArea
			{
				.offset = { 0,0 },   		
				.extent = {32, 32}
			};
			
			VkRenderPassBeginInfo renderPassInfo
			{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = VK_NULL_HANDLE,
				.renderPass = renderPasses.offscreen,          
				.framebuffer = framebuffers.offscreen,
				.renderArea = renderArea,
				.clearValueCount = static_cast<uint32_t>(clearValues.size()),
				.pClearValues = clearValues.data()
			};

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkDeviceSize offsets[] = { 0 };

			VkViewport viewport{};
			viewport.x = 0.f;
			viewport.y = 0.f;
			viewport.width =  static_cast<float>(32);
			viewport.height = static_cast<float>(32);
			viewport.minDepth = 0.f;
			viewport.maxDepth = 1.f;

			VkRect2D scissor
			{
				.offset = {0, 0},
				.extent = {32, 32} 
			};
			
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.irradiance);	

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		
			vkCmdBindIndexBuffer(commandBuffer, cubeIndexBuffer, 0, VK_INDEX_TYPE_UINT32);		

			ObjectUniform uniformData;
			uniformData.model = glm::mat4(1.);
			uniformData.proj = captureProjection;;

			uniformData.view = captureViews[i];

			memcpy(uniformBuffersMapped[0].cube[i], &uniformData, sizeof(uniformData));

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.irradiance, 0, 1, &descriptorSets[0].irradiance[i], 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffer);
			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

			// Transition color attachment for transfer src 
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			VkImageMemoryBarrier barrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.offscreen.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = 0; 
			barrier.subresourceRange.layerCount = 1;
			
			VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

			// Transition Cubemap for transfer destination	
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			barrier = 
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.irradianceCubemap.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = i; 
			barrier.subresourceRange.layerCount = 1;
			
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

			// Copy equirectangular color attachment to cubemap
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = i;
			copyRegion.dstSubresource.mipLevel = 0;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(512);
			copyRegion.extent.height = static_cast<uint32_t>(512);
			copyRegion.extent.depth = 1;

			vkCmdCopyImage(
				commandBuffer,
				textures.offscreen.image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				textures.irradianceCubemap.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &copyRegion
			);	

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			barrier =
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.offscreen.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = 0; 
			barrier.subresourceRange.layerCount = 1;
			
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
			// Transition cubemap to shader read
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			barrier = 
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.irradianceCubemap.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = 0; 
			barrier.subresourceRange.layerCount = 6;
			
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
		};
	};

	void recordEquirectangularToCubemap()
	{
		for (int i = 0; i < 6; i++)
		{
			VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].depthStencil = {1.f, 0};
			clearValues[1].color = {{.0f, .0f, .0f, 1.f}};

			VkRect2D renderArea
			{
				.offset = { 0,0 },   		
				.extent = {512, 512}
			};
			
			VkRenderPassBeginInfo renderPassInfo
			{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext = VK_NULL_HANDLE,
				.renderPass = renderPasses.offscreen,          
				.framebuffer = framebuffers.offscreen,
				.renderArea = renderArea,
				.clearValueCount = static_cast<uint32_t>(clearValues.size()),
				.pClearValues = clearValues.data()
			};

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkDeviceSize offsets[] = { 0 };

			VkViewport viewport{};
			viewport.x = 0.f;
			viewport.y = 0.f;
			viewport.width = static_cast<float>(512);
			viewport.height = static_cast<float>(512);
			viewport.minDepth = 0.f;
			viewport.maxDepth = 1.f;

			VkRect2D scissor
			{
				.offset = {0, 0},
				.extent = {512, 512} 
			};
			
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.cube);	

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		
			vkCmdBindIndexBuffer(commandBuffer, cubeIndexBuffer, 0, VK_INDEX_TYPE_UINT32);		

			ObjectUniform uniformData;
			uniformData.model = glm::mat4(1.);
			uniformData.proj = captureProjection;;

			uniformData.view = captureViews[i];

			memcpy(uniformBuffersMapped[0].cube[i], &uniformData, sizeof(uniformData));

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.cube, 0, 1, &descriptorSets[0].cube[i], 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);
			

			vkCmdEndRenderPass(commandBuffer);
			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

			// Transition color attachment for transfer src 
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			VkImageMemoryBarrier barrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.offscreen.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = 0; 
			barrier.subresourceRange.layerCount = 1;
			
			VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

			// Transition Cubemap for transfer destination	
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			barrier = 
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.equirectangularCubemap.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = i; 
			barrier.subresourceRange.layerCount = 1;
			
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

			// Copy equirectangular color attachment to cubemap
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = i;
			copyRegion.dstSubresource.mipLevel = 0;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(512);
			copyRegion.extent.height = static_cast<uint32_t>(512);
			copyRegion.extent.depth = 1;

			vkCmdCopyImage(
				commandBuffer,
				textures.offscreen.image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				textures.equirectangularCubemap.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &copyRegion
			);	

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
			// Transition equirectangular back to color attachment
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			barrier =
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.offscreen.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = 0; 
			barrier.subresourceRange.layerCount = 1;
			
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
			// Transition cubemap to shader read
			commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		

			std::cout << "Fix this barrier \n";
			barrier = 
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = 0,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,	
				.image = textures.equirectangularCubemap.image
			};			
			
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1,
			barrier.subresourceRange.baseArrayLayer = 0; 
			barrier.subresourceRange.layerCount = 6;
			
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			
			vkCmdPipelineBarrier(
				commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
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
		renderPassInfo.renderPass = renderPasses.present;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = VulkanConfig::swapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].depthStencil = {1.f, 0};
		clearValues[1].color = {{.0f, .0f, .0f, 1.f}};

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

		VkRect2D scissor
		{
			.offset = {0, 0},
			.extent = VulkanConfig::swapChainExtent
		};
		
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };
		VkBuffer vertexSphereBuffers[] = { sphereBuffer };

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.equirectangularCubemap);	

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
	
		vkCmdBindIndexBuffer(commandBuffer, cubeIndexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.equirectangularCubemap, 0, 1, &descriptorSets[currentFrame].equirectangularCubemap, 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);

		vkCmdBindIndexBuffer(commandBuffer, sphereIndexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.sphere);	

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexSphereBuffers, offsets);
		
		vkCmdBindIndexBuffer(commandBuffer, sphereIndexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		for (size_t i = 0; i < SPHERE_COUNT; i++)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.sphere, 0, 1, &descriptorSets[currentFrame].sphere[i], 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(sphereIndices.size()), 1, 0, 0, 0);
		};

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	};

	void setupSphereResources()
	{
		setupSphereDescriptorSets();	
		setupSpherePipeline();
	};

	void init(GLFWwindow* window)
	{
		IVulkanApp::init(window);	
		setupSphere();
		setupUniformBuffers();
		setupSamplers();
		setupRenderPass();
		IBL::createIndexBuffer();

		{
			loadHDRImage();	
			setupOffscreenDepth();
			setupOffscreenColorAttachment();
			setupEquirectangularCubemap();
			setupOffscreenPass();
			createOffscreenFramebuffers();
			setupEquirectangularCubeDescriptorSets();
			setupEquirectangularCubePipeline();
			recordEquirectangularToCubemap();
			setupEquirectangularCubemapDescriptorSets();
			setupEquirectangularCubemapPipeline();
		};

		{
//			setupIrradianceCubemap();	
//			setupIrradianceDescriptorSets();
//			setupIrradiancePipeline();
//			recordIrradiance();
//			setupIrradianceCubemapDescriptorSets();
//			setupIrradianceCubemapPipeline();
		};

		createVertexBuffer(sphereVertices, sphereBuffer, sphereBufferMemory);
		createSphereIndexBuffer();
		loadModel();
		loadTexture();
		setupDepth();
		setupImageViews();
		setupSphereResources();
//		setupModelDescriptorSets(); 
//		setupModelPipeline();
		createSwapChainFramebuffers();
		setupDebugObjectNames();
	};

	void setupDebugObjectNames()	
	{
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.wood.image, "textures.wood.image");

		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.wood.imageView, "textures.wood.imageView");

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			const std::string swapChainFramebufferName{"swapChainFramebufferName[" + std::to_string(i) + "]"};

			setupDebugObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, swapChainFramebuffers[i], swapChainFramebufferName.c_str());
		};
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
		}		

		vkDestroyBuffer(VulkanConfig::device, indexBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, indexBufferMemory, nullptr);
		
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);

		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);

		vkDestroyImage(VulkanConfig::device, textures.wood.image, nullptr);
		vkFreeMemory(VulkanConfig::device, textures.wood.imageMemory, nullptr);

		vkDestroyRenderPass(VulkanConfig::device, renderPasses.offscreen, nullptr);

		vkDestroyDevice(VulkanConfig::device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
		
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	};

	void blitImage(VkCommandBuffer& commandBuffer, VkImage& srcImage, VkImage& dstImage)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = srcImage;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		barrier.image = dstImage;
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {static_cast<int32_t>(VulkanConfig::swapChainExtent.width),static_cast<int32_t>(VulkanConfig::swapChainExtent.height), 1};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {static_cast<int32_t>(VulkanConfig::swapChainExtent.width),static_cast<int32_t>(VulkanConfig::swapChainExtent.height), 1};
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			commandBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_NEAREST);

		barrier.image = dstImage;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.image = srcImage;
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
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
		setupDepth();
		
		//createOffscreenFramebuffers();
		createSwapChainFramebuffers();
	}

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
};
