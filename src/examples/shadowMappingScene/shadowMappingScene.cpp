#include "./shadowMappingScene.h"
#include <iostream>
#include "../../bindings/particle.h"
#include "../../core/queueFamily/queueFamily.h"

void ShadowMappingScene::init(GLFWwindow* window)
{
	createInstance();
	setupDebugMessenger();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain(window);
	createImageViews();
	createShadowMapRenderPass();
	createRenderPass();
	createPostProcessingRenderPass();
	createDescriptorSetLayouts();	
	createModel();
	createPipelines();
	CommandBuffer::createCommandPool(physicalDevice, device, surface);
	createOffscreenResources();
	createColorResources();
	createDepthResources();
	createShadowMapResources();
	createFramebuffers();
	Image::Texture::create(TEXTURE_PATH, textureImage, textureImageMemory, device, physicalDevice, graphicsAndComputeQueue);
	createCubeTextureImage(CUBEMAP_PATH, cubemapImage, cubemapImageMemory);
	createCubeMapResources();
	Image::Texture::create(SPECULAR_PATH, specularImage, specularImageMemory, device, physicalDevice, graphicsAndComputeQueue);
	createTextureImageView(textureImage, textureImageView, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	createTextureImageView(specularImage, specularImageView, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	Image::Texture::Sampler::createTextureSampler(textureSampler, device, physicalDevice, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	Image::Texture::Sampler::createTextureSampler(specularSampler, device, physicalDevice, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	Image::Texture::Sampler::createTextureSampler(shadowSampler, device, physicalDevice, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	createTextureImages(modelImages, modelImageMemories);	
	createTextureImageViews(modelImages, modelImageViews);
	createTextureSamplers(modelSamplers);
	createShaderStorageBuffers();
	createVertexBuffers();
	createIndexBuffer();
	createQuadIndexBuffer();
	createModelIndexBuffers();
	createUniformBuffers();
	createDescriptorPools();
	createDescriptorSets();
	CommandBuffer::createCommandBuffers(device);
	createComputeCommandBuffers();
	createSyncObjects();
}

void ShadowMappingScene::createSwapChain(GLFWwindow * window)
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	QueueFamilyIndices indices = QueueFamily::findQueueFamilies(physicalDevice, surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsAndComputeFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("faild to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	shadowMapImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, shadowMapImages.data());

	swapChainImageFormat = surfaceFormat.format;
	VulkanConfig::swapChainExtent = extent;
}

void ShadowMappingScene::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());
	shadowMapImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImageViews[i] = Image::createView(swapChainImages[i], textureImageView, VK_IMAGE_VIEW_TYPE_2D, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, device, graphicsAndComputeQueue);
	}
}

void ShadowMappingScene::createShadowMapRenderPass()
{
	VkAttachmentDescription depthAttachment{};
	//depthAttachment.format = findDepthFormat();
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkSubpassDependency, 2> dependency{};

	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	dependency[1].srcSubpass = 0;
	dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependency[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency[1].dstStageMask =  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[1].dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;
	
	std::array<VkAttachmentDescription, 1> attachments = {depthAttachment};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependency.data();

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses.shadowMapRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void ShadowMappingScene::createRenderPass()
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
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	//depthattachment.format = finddepthformat();
	depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	depthAttachment.samples = VulkanConfig::msaaSamples; 
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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

	std::array<VkSubpassDependency, 2> dependency{};

	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	dependency[1].srcSubpass = 0;
	dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency[1].dstStageMask =  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[1].dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;
	
	std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependency.data();

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses.renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void ShadowMappingScene::createPostProcessingRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = nullptr;
	subpass.pResolveAttachments = nullptr;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		
	std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses.postProcessingRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void ShadowMappingScene::createDescriptorSetLayouts()
{
	createComputeDescriptorSetLayout();
}

void ShadowMappingScene::createComputeDescriptorSetLayout()
{
	std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].pImmutableSamplers = nullptr;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[1].pImmutableSamplers = nullptr;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	layoutBindings[2].binding = 2;
	layoutBindings[2].descriptorCount = 1;
	layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[2].pImmutableSamplers = nullptr;
	layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 3;
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create compute descriptor set layout!");
	}
}

void ShadowMappingScene::createPipelines()
{
	Pipelines::createPipelines(device, renderPasses);
	createComputePipeline();
}

VkShaderModule ShadowMappingScene::createShaderModule(const std::vector<char>& code)
{

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module");
	}

	return shaderModule;
}

void ShadowMappingScene::createComputePipeline()
{
	auto compShaderCode = readFile(ROOT_DIR + "/src/shaders/comp.spv");

	VkShaderModule compShaderModule = createShaderModule(compShaderCode);

	VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = compShaderModule;
	computeShaderStageInfo.pName = "main";

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create compute pipeline layout!");
	}

	VkComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.layout = computePipelineLayout;
	pipelineInfo.stage = computeShaderStageInfo;

	if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS)
	{
			throw std::runtime_error("failed to create compute pipeline!");
	}
}

void ShadowMappingScene::createShadowMapResources()
{
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

	shadowMapImages.resize(swapChainImages.size());
	shadowMapImageViews.resize(swapChainImages.size());
	shadowMapImageMemories.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		Image::create(VulkanConfig::swapChainExtent.width,VulkanConfig::swapChainExtent.height, 1, 1,0, VK_IMAGE_TYPE_2D,VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowMapImages[i], shadowMapImageMemories[i], VK_IMAGE_LAYOUT_UNDEFINED, device, physicalDevice);
		
		shadowMapImageViews[i] = Image::createView(shadowMapImages[i], shadowMapImageViews[i], VK_IMAGE_VIEW_TYPE_2D, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, device, graphicsAndComputeQueue);

	}
}

void ShadowMappingScene::createOffscreenResources()
{
	VkFormat colorFormat = swapChainImageFormat;

	offScreenImages.resize(swapChainImages.size());
	offScreenImageViews.resize(swapChainImages.size());
	offScreenImageMemories.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		Image::create(VulkanConfig::swapChainExtent.width,VulkanConfig::swapChainExtent.height, 1, 1,0, VK_IMAGE_TYPE_2D,VK_SAMPLE_COUNT_1_BIT, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, offScreenImages[i], offScreenImageMemories[i], VK_IMAGE_LAYOUT_UNDEFINED, device, physicalDevice);
		offScreenImageViews[i] = Image::createView(offScreenImages[i], textureImageView, VK_IMAGE_VIEW_TYPE_2D, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, device, graphicsAndComputeQueue);
	}
}

void ShadowMappingScene::createColorResources()
{
	VkFormat colorFormat = swapChainImageFormat;
	
	Image::create(VulkanConfig::swapChainExtent.width,VulkanConfig::swapChainExtent.height, 1, 1,0, VK_IMAGE_TYPE_2D, VulkanConfig::msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, VK_IMAGE_LAYOUT_UNDEFINED, device, physicalDevice);
	colorImageView = Image::createView(colorImage, textureImageView, VK_IMAGE_VIEW_TYPE_2D, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, device, graphicsAndComputeQueue);
}

void ShadowMappingScene::createDepthResources()
{
	//VkFormat depthFormat = findDepthFormat();
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
	Image::create(VulkanConfig::swapChainExtent.width, VulkanConfig::swapChainExtent.height, 1, 1,0, VK_IMAGE_TYPE_2D,
VulkanConfig::msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_IMAGE_LAYOUT_UNDEFINED, device, physicalDevice);

	depthImageView = Image::createView(depthImage, textureImageView, VK_IMAGE_VIEW_TYPE_2D, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, device, graphicsAndComputeQueue);
}

void ShadowMappingScene::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());
	offScreenFramebuffers.resize(swapChainImageViews.size());
	shadowMapFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 1> attachments = { 
			shadowMapImageViews[i],
		};
		
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPasses.shadowMapRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = VulkanConfig::swapChainExtent.width;
		framebufferInfo.height = VulkanConfig::swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &shadowMapFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		};

	}

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments = { 
			colorImageView, 
			depthImageView,
			offScreenImageViews[i], 
		};
		
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPasses.renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = VulkanConfig::swapChainExtent.width;
		framebufferInfo.height = VulkanConfig::swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &offScreenFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		};
	}

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 1> attachments = { 
			swapChainImageViews[i], 
		};
		
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPasses.postProcessingRenderPass;
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

void ShadowMappingScene::createModel()
{
	model = new Model(MODEL_PATH);
	vertexBuffers.resize(model->meshes.size());	
	vertexBufferMemories.resize(model->meshes.size());	
	VulkanConfig::MESH_COUNT = model->meshes.size();
}

void ShadowMappingScene::createTextureImageView(VkImage& image, VkImageView& imageView, VkFormat format, VkImageAspectFlagBits flags)
{
	imageView = Image::createView(image, imageView, VK_IMAGE_VIEW_TYPE_2D, format, flags, Image::mipLevels, 1, device, graphicsAndComputeQueue);
}

void ShadowMappingScene::createCubeTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory)
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

void ShadowMappingScene::createCubeMapResources()
{
	cubemapImageView = Image::createView(cubemapImage, cubemapImageView, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, 6, device, graphicsAndComputeQueue);
}

void ShadowMappingScene::createTextureSampler(VkSampler& sampler)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0.f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void ShadowMappingScene::createTextureImages(std::vector<VkImage>& images, std::vector<VkDeviceMemory>& imageMemories)
{
	// TODO: move to getTextureCount later, maybe
	images.resize(VulkanConfig::MESH_COUNT);
	imageMemories.resize(VulkanConfig::MESH_COUNT);
	modelSamplers.resize(VulkanConfig::MESH_COUNT);
	modelImageViews.resize(VulkanConfig::MESH_COUNT);

	for (size_t i = 0; i < VulkanConfig::MESH_COUNT; i++)
	{
		if (model->meshes[i].textures.empty())
		{
			Image::Texture::create(MODEL_TEXTURE_DIRECTORY + model->meshes[0].textures[0].path, images[i], imageMemories[i], device, physicalDevice, graphicsAndComputeQueue);
		}
		else
		{
			Image::Texture::create(MODEL_TEXTURE_DIRECTORY + model->meshes[i].textures[0].path, images[i], imageMemories[i], device, physicalDevice, graphicsAndComputeQueue);
		}
	}
}

void ShadowMappingScene::createTextureImageViews(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews)
{
	for (size_t i = 0; i < VulkanConfig::MESH_COUNT; i++)
	{
		imageViews[i] = Image::createView(images[i], imageViews[i], VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, Image::mipLevels, 1, device, graphicsAndComputeQueue);
	}
}

void ShadowMappingScene::createTextureSamplers(std::vector<VkSampler>& samplers)
{
	for (size_t i = 0; i < VulkanConfig::MESH_COUNT; i++)
	{
		Image::Texture::Sampler::createTextureSampler(samplers[i], device, physicalDevice, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	}
}

void ShadowMappingScene::createShaderStorageBuffers()
{
	uint32_t WIDTH = GLFWWindowContext::getWindowWidth();
	uint32_t HEIGHT = GLFWWindowContext::getWindowHeight();

	VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;
	shaderStorageBuffers.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	shaderStorageBuffersMemory.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	std::default_random_engine rndEngine((unsigned)time(nullptr));
	std::uniform_real_distribution<float> rndDist(0.0f, 1.f);
	
	std::vector<Particle> particles(PARTICLE_COUNT);

	for (auto & particle : particles)
	{
		float r = 0.25f * sqrt(rndDist(rndEngine));
		float theta = rndDist(rndEngine) * 2 * 3.141592;
		float x = 2. * r * cos(theta) * HEIGHT/WIDTH;
		float y = 2. * r * sin(theta);
		particle.position = glm::vec2(x, y);
		particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;
		particle.color = glm::vec4(rndDist(rndEngine),rndDist(rndEngine),rndDist(rndEngine), 1.f);
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

	void * data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, particles.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		Buffer::create(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffers[i], shaderStorageBuffersMemory[i], device, physicalDevice);	
	
		copyBuffer(stagingBuffer, shaderStorageBuffers[i], bufferSize);
	}

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void ShadowMappingScene::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(device);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	CommandBuffer::endSingleTimeCommands(commandBuffer, graphicsAndComputeQueue, device);
}

void ShadowMappingScene::createVertexBuffers()
{
	for (size_t i = 0; i < model->meshes.size(); i++)
	{
		const Mesh mesh = model->meshes[i];
		createVertexBuffer<Vertex>(mesh.vertices, vertexBuffers[i], vertexBufferMemories[i]);
	}

	createVertexBuffer<Vertex>(cubeVertices, vertexCubeBuffer, vertexCubeBufferMemory);
	createVertexBuffer<Vertex>(cubemapVertices, vertexCubemapBuffer, vertexCubemapBufferMemory);
}

void ShadowMappingScene::createQuadIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(ShadowMappingScene::quadIndices[0]) * ShadowMappingScene::quadIndices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, ShadowMappingScene::quadIndices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadIndexBuffer, quadIndexBufferMemory, device, physicalDevice);
	copyBuffer(stagingBuffer, quadIndexBuffer, bufferSize);
	
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void ShadowMappingScene::createModelIndexBuffers()
{
	indexModelBuffers.resize(VulkanConfig::MESH_COUNT);
	indexModelBufferMemories.resize(VulkanConfig::MESH_COUNT);

	for (size_t i = 0; i < VulkanConfig::MESH_COUNT; i++)
	{
		createModelIndexBuffer(model->meshes[i].indices, indexModelBuffers[i], indexModelBufferMemories[i]);
	}
}

void ShadowMappingScene::createModelIndexBuffer(std::vector<uint32_t> m_Indices, VkBuffer& modelBuffer,VkDeviceMemory& modelMemory)
{

	VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_Indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, modelBuffer, modelMemory, device, physicalDevice);
	copyBuffer(stagingBuffer, modelBuffer, bufferSize);
	
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

}

void ShadowMappingScene::createUniformBuffers()
{
	createGraphicsUniformBuffers();
	createPrimitiveUniformBuffers();
	createCubemapUniformBuffers();
	createStencilUniformBuffers();
	createMaterialUniformBuffers();
	createLightUniformBuffers();
	createModelLightUniformBuffers();
	createLightObjectUniformBuffers();
	createModelUniformBuffers();
}


void ShadowMappingScene::createMaterialUniformBuffers()
{
	materialUniformBuffers.resize(VulkanConfig::OBJECT_COUNT);
	materialUniformBuffersMemory.resize(VulkanConfig::OBJECT_COUNT);
	materialUniformBuffersMapped.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		VkDeviceSize bufferSize = sizeof(Material);

		materialUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		materialUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		materialUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			materialUniformBuffers[j][i], materialUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, materialUniformBuffersMemory[j][i], 0, bufferSize, 0, &materialUniformBuffersMapped[j][i]);

		}
	}
}

void ShadowMappingScene::createLightObjectUniformBuffers()
{
	lightObjectUniformBuffers.resize(VulkanConfig::MAX_POINT_LIGHTS);
	lightObjectUniformBuffersMemory.resize(VulkanConfig::MAX_POINT_LIGHTS);
	lightObjectUniformBuffersMapped.resize(VulkanConfig::MAX_POINT_LIGHTS);

	for (size_t j = 0; j < VulkanConfig::MAX_POINT_LIGHTS; j++)
	{
		VkDeviceSize bufferSize = sizeof(PointLight);

		lightObjectUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		lightObjectUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		lightObjectUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				lightObjectUniformBuffers[j][i], lightObjectUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, lightObjectUniformBuffersMemory[j][i], 0, bufferSize, 0, &lightObjectUniformBuffersMapped[j][i]);

		}
	}
}

void ShadowMappingScene::createModelLightUniformBuffers()
{
	modelLightUniformBuffers.resize(VulkanConfig::MESH_COUNT);
	modelLightUniformBuffersMemory.resize(VulkanConfig::MESH_COUNT);
	modelLightUniformBuffersMapped.resize(VulkanConfig::MESH_COUNT);

	for (size_t j = 0; j < VulkanConfig::MESH_COUNT; j++)
	{
		VkDeviceSize bufferSize = sizeof(Lights);

		modelLightUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		modelLightUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		modelLightUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				modelLightUniformBuffers[j][i], modelLightUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, modelLightUniformBuffersMemory[j][i], 0, bufferSize, 0, &modelLightUniformBuffersMapped[j][i]);

		}
	}
}

void ShadowMappingScene::createLightUniformBuffers()
{
	lightUniformBuffers.resize(VulkanConfig::OBJECT_COUNT);
	lightUniformBuffersMemory.resize(VulkanConfig::OBJECT_COUNT);
	lightUniformBuffersMapped.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		VkDeviceSize bufferSize = sizeof(Lights);

		lightUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		lightUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		lightUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				lightUniformBuffers[j][i], lightUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, lightUniformBuffersMemory[j][i], 0, bufferSize, 0, &lightUniformBuffersMapped[j][i]);

		}
	}
}

void ShadowMappingScene::createStencilUniformBuffers()
{
	stencilUniformBuffers.resize(VulkanConfig::OBJECT_COUNT);
	stencilUniformBuffersMemory.resize(VulkanConfig::OBJECT_COUNT);
	stencilUniformBuffersMapped.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObjectModel);

		stencilUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		stencilUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		stencilUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stencilUniformBuffers[j][i], stencilUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, stencilUniformBuffersMemory[j][i], 0, bufferSize, 0, &stencilUniformBuffersMapped[j][i]);

		}
	}
}

void ShadowMappingScene::createShadowMapUniformBuffers()
{
	shadowMapUniformBuffers.resize(VulkanConfig::OBJECT_COUNT);
	shadowMapUniformBuffersMemory.resize(VulkanConfig::OBJECT_COUNT);
	shadowMapUniformBuffersMapped.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObjectModel);

		shadowMapUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		shadowMapUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		shadowMapUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			shadowMapUniformBuffers[j][i], shadowMapUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, shadowMapUniformBuffersMemory[j][i], 0, bufferSize, 0, &shadowMapUniformBuffersMapped[j][i]);

		}
	}
}

void ShadowMappingScene::createPrimitiveUniformBuffers()
{
	primitiveUniformBuffers.resize(VulkanConfig::OBJECT_COUNT);
	primitiveUniformBuffersMemory.resize(VulkanConfig::OBJECT_COUNT);
	primitiveUniformBuffersMapped.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObjectModel);

		primitiveUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		primitiveUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		primitiveUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			primitiveUniformBuffers[j][i], primitiveUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, primitiveUniformBuffersMemory[j][i], 0, bufferSize, 0, &primitiveUniformBuffersMapped[j][i]);

		}
	}
}

void ShadowMappingScene::createModelUniformBuffers()
{		
	modelUniformBuffers.resize(VulkanConfig::MESH_COUNT);
	modelUniformBuffersMemory.resize(VulkanConfig::MESH_COUNT);
	modelUniformBuffersMapped.resize(VulkanConfig::MESH_COUNT);

	for (size_t j = 0; j < VulkanConfig::MESH_COUNT; j++)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObjectModel);

		modelUniformBuffers[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		modelUniformBuffersMemory[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		modelUniformBuffersMapped[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				modelUniformBuffers[j][i], modelUniformBuffersMemory[j][i], device, physicalDevice);

			vkMapMemory(device, modelUniformBuffersMemory[j][i], 0, bufferSize, 0, &modelUniformBuffersMapped[j][i]);

		}
	}
}


void ShadowMappingScene::createCubemapUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

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

void ShadowMappingScene::createGraphicsUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	baseUniformBuffers.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	baseUniformBuffersMemory.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	baseUniformBuffersMapped.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			baseUniformBuffers[i], baseUniformBuffersMemory[i], device, physicalDevice);

		vkMapMemory(device, baseUniformBuffersMemory[i], 0, bufferSize, 0, &baseUniformBuffersMapped[i]);

	}
}
	
void ShadowMappingScene::createDescriptorPools()
{
	createComputeDescriptorPool();
}

void ShadowMappingScene::createComputeDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT) * 2;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &computeDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}


void ShadowMappingScene::createDescriptorSets()
{
	createGraphicsDescriptorSets();
	createPrimitiveDescriptorSets();
	createShadowMapDescriptorSets();
	createStencilDescriptorSets();
	createModelDescriptorSets();
	createPostProcessingDescriptorSets();
	createShadowMapScreenSpaceQuadDescriptorSets();
	createCubemapDescriptorSets();
	createLightDescriptorSets();
	createComputeDescriptorSets();
}

void ShadowMappingScene::createLightDescriptorSets()
{
	lightDescriptorSets.resize(VulkanConfig::MAX_POINT_LIGHTS);

	for (size_t j = 0; j < VulkanConfig::MAX_POINT_LIGHTS; j++)
	{
		std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::lightPipeline.descriptor.layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = Pipelines::lightPipeline.descriptor.pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		lightDescriptorSets[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, lightDescriptorSets[j].data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor sets!");
		}

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = lightObjectUniformBuffers[j][i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(PointLight);

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = lightDescriptorSets[j][i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}


void ShadowMappingScene::createStencilDescriptorSets()
{
	stencilDescriptorSets.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::stencilPipeline.descriptor.layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = Pipelines::stencilPipeline.descriptor.pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		stencilDescriptorSets[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, stencilDescriptorSets[j].data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor sets!");
		}

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = stencilUniformBuffers[j][i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObjectModel);

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = stencilDescriptorSets[j][i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}

void ShadowMappingScene::createShadowMapDescriptorSets()
{
	shadowMapDescriptorSets.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::shadowMapPipeline.descriptor.layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = Pipelines::shadowMapPipeline.descriptor.pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		shadowMapDescriptorSets[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, shadowMapDescriptorSets[j].data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor sets!");
		}

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = primitiveUniformBuffers[j][i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObjectModel);

			VkDescriptorBufferInfo materialsBufferInfo{};
			materialsBufferInfo.buffer = materialUniformBuffers[j][i];
			materialsBufferInfo.offset = 0;
			materialsBufferInfo.range = sizeof(Material);
	
			VkDescriptorBufferInfo lightBufferInfo{};
			lightBufferInfo.buffer = lightUniformBuffers[j][i];
			lightBufferInfo.offset = 0;
			lightBufferInfo.range = sizeof(Lights);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			VkDescriptorImageInfo specularImageInfo{};
			specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			specularImageInfo.imageView = specularImageView;
			specularImageInfo.sampler = specularSampler;

			VkDescriptorImageInfo primitiveImageInfo{};
			primitiveImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			primitiveImageInfo.imageView = shadowMapImageViews[i];
			primitiveImageInfo.sampler = textureSampler;

			std::array<VkWriteDescriptorSet, 5> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMapDescriptorSets[j][i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
	
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = shadowMapDescriptorSets[j][i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &materialsBufferInfo;
		
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = shadowMapDescriptorSets[j][i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageInfo;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = shadowMapDescriptorSets[j][i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pBufferInfo = &lightBufferInfo;
			
			descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[4].dstSet = shadowMapDescriptorSets[j][i];
			descriptorWrites[4].dstBinding = 4;
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[4].descriptorCount = 1;
			descriptorWrites[4].pImageInfo = &specularImageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}

void ShadowMappingScene::createPrimitiveDescriptorSets()
{
	primitiveDescriptorSets.resize(VulkanConfig::OBJECT_COUNT);

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::primitivePipeline.descriptor.layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = Pipelines::primitivePipeline.descriptor.pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		primitiveDescriptorSets[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, primitiveDescriptorSets[j].data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor sets!");
		}

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = primitiveUniformBuffers[j][i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObjectModel);

			VkDescriptorBufferInfo materialsBufferInfo{};
			materialsBufferInfo.buffer = materialUniformBuffers[j][i];
			materialsBufferInfo.offset = 0;
			materialsBufferInfo.range = sizeof(Material);
	
			VkDescriptorBufferInfo lightBufferInfo{};
			lightBufferInfo.buffer = lightUniformBuffers[j][i];
			lightBufferInfo.offset = 0;
			lightBufferInfo.range = sizeof(Lights);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			VkDescriptorImageInfo specularImageInfo{};
			specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			specularImageInfo.imageView = specularImageView;
			specularImageInfo.sampler = specularSampler;

			VkDescriptorImageInfo shadowMapImageInfo{};
			shadowMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			shadowMapImageInfo.imageView = shadowMapImageViews[i];
			shadowMapImageInfo.sampler = shadowSampler;

			std::array<VkWriteDescriptorSet, 6> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = primitiveDescriptorSets[j][i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
	
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = primitiveDescriptorSets[j][i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &materialsBufferInfo;
		
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = primitiveDescriptorSets[j][i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageInfo;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = primitiveDescriptorSets[j][i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pBufferInfo = &lightBufferInfo;
			
			descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[4].dstSet = primitiveDescriptorSets[j][i];
			descriptorWrites[4].dstBinding = 4;
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[4].descriptorCount = 1;
			descriptorWrites[4].pImageInfo = &specularImageInfo;

			descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[5].dstSet = primitiveDescriptorSets[j][i];
			descriptorWrites[5].dstBinding = 5;
			descriptorWrites[5].dstArrayElement = 0;
			descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[5].descriptorCount = 1;
			descriptorWrites[5].pImageInfo = &shadowMapImageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}

void ShadowMappingScene::createModelDescriptorSets()
{
	modelDescriptorSets.resize(VulkanConfig::MESH_COUNT);
	for (size_t j = 0; j < VulkanConfig::MESH_COUNT; j++)
	{
		std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::meshPipeline.descriptor.layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = Pipelines::meshPipeline.descriptor.pool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		modelDescriptorSets[j].resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, modelDescriptorSets[j].data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor sets!");
		}

		for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = modelUniformBuffers[j][i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObjectModel);
			
			VkDescriptorBufferInfo lightBufferInfo{};
			lightBufferInfo.buffer = modelLightUniformBuffers[j][i];
			lightBufferInfo.offset = 0;
			lightBufferInfo.range = sizeof(Lights);

			VkDescriptorImageInfo shadowMapInfo{};
			shadowMapInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			shadowMapInfo.imageView = shadowMapImageViews[i];
			shadowMapInfo.sampler = shadowSampler;

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = modelImageViews[j];
			imageInfo.sampler = modelSamplers[j];

			std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = modelDescriptorSets[j][i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = modelDescriptorSets[j][i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;
			
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = modelDescriptorSets[j][i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = &lightBufferInfo;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = modelDescriptorSets[j][i];
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pImageInfo = &shadowMapInfo;
			
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
}

void ShadowMappingScene::createCubemapDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::cubemapPipeline.descriptor.layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pipelines::cubemapPipeline.descriptor.pool;
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

void ShadowMappingScene::createShadowMapScreenSpaceQuadDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::screenSpacePipeline.descriptor.layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pipelines::screenSpacePipeline.descriptor.pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	screenSpaceDescriptorSets.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, screenSpaceDescriptorSets.data()) != VK_SUCCESS)
	{
		std::cout << "Failed to create descriptor sets!\n";
		throw std::runtime_error("Failed to create descriptor sets!");
	}

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorImageInfo shadowMapImageInfo{};
		shadowMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		shadowMapImageInfo.imageView = shadowMapImageViews[i];
		shadowMapImageInfo.sampler = textureSampler;


		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = screenSpaceDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &shadowMapImageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void ShadowMappingScene::createPostProcessingDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::postProcessingPipeline.descriptor.layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pipelines::postProcessingPipeline.descriptor.pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	postProcessingDescriptorSets.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, postProcessingDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets!");
	}

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = offScreenImageViews[i];
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = postProcessingDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void ShadowMappingScene::createGraphicsDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::basePipeline.descriptor.layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pipelines::basePipeline.descriptor.pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	baseDescriptorSets.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, baseDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets!");
	}

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = baseUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = baseDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = baseDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}


void ShadowMappingScene::createComputeDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = computeDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	computeDescriptorSets.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, computeDescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}	

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo uniformBufferInfo{};
		uniformBufferInfo.buffer = baseUniformBuffers[i];
		uniformBufferInfo.offset = 0;
		uniformBufferInfo.range = sizeof(UniformBufferObject);

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

		VkDescriptorBufferInfo storageBufferInfoLastFrame{};
		storageBufferInfoLastFrame.buffer = shaderStorageBuffers[(i - 1) % VulkanConfig::MAX_FRAMES_IN_FLIGHT];
		storageBufferInfoLastFrame.offset = 0;
		storageBufferInfoLastFrame.range = sizeof(Particle) * PARTICLE_COUNT;

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = computeDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = computeDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

		VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
		storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i];
		storageBufferInfoCurrentFrame.offset = 0;
		storageBufferInfoCurrentFrame.range = sizeof(Particle) * PARTICLE_COUNT;
		
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = computeDescriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

		vkUpdateDescriptorSets(device, 3, descriptorWrites.data(), 0, nullptr);
	}
}

void ShadowMappingScene::createComputeCommandBuffers()
{
	computeCommandBuffers.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = CommandBuffer::commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)computeCommandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate compute command buffers!");
	}
}

void ShadowMappingScene::createSyncObjects()
{
	imageAvailableSemaphores.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	computeFinishedSemaphores.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	computeInFlightFences.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &computeInFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create compute synchronization objects for a frame!");
		}
	}
}

void ShadowMappingScene::drawFrame(GLFWwindow * window)
{
	camera.update();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	vkWaitForFences(device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	updateUniformBuffer(currentFrame);

	vkResetFences(device, 1, &computeInFlightFences[currentFrame]);

	vkResetCommandBuffer(computeCommandBuffers[currentFrame], 0);

	recordComputeCommandBuffer(computeCommandBuffers[currentFrame]);

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

	if (vkQueueSubmit(graphicsAndComputeQueue, 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit compute command buffer!");
	};

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

	VkSemaphore waitSemaphores[] = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 2;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffer::commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

	if (vkQueueSubmit(graphicsAndComputeQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

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

void ShadowMappingScene::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
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
	renderPassInfo.renderPass = renderPasses.shadowMapRenderPass;
	renderPassInfo.framebuffer = shadowMapFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = VulkanConfig::swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{1.f, 1.f, 1.f, 1.f}};
	clearValues[1].depthStencil = {.0f, 0};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// SHADOW MAP PASS
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	Pipelines::basePipeline.bind(commandBuffer);

	VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };
	VkBuffer vertexCubemapBuffers[] = { vertexCubemapBuffer };
	VkDeviceSize offsets[] = { 0 };
	
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(VulkanConfig::swapChainExtent.width);
	viewport.height = static_cast<float>(VulkanConfig::swapChainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {2048, 2048};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	Pipelines::shadowMapMeshPipeline.bind(commandBuffer);

	for (size_t i = 0; i < VulkanConfig::MESH_COUNT; i++)
	{
		VkBuffer modelVertexBuffers[] = {vertexBuffers[i]};

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, modelVertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexModelBuffers[i], 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::shadowMapMeshPipeline.getLayout(), 0, 1, &modelDescriptorSets[i][currentFrame], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->meshes[i].indices.size()), 1, 0, 0, 0);
	}

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		Pipelines::shadowMapPrimitivePipeline.bind(commandBuffer);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::shadowMapPrimitivePipeline.getLayout(), 0, 1, &shadowMapDescriptorSets[j][currentFrame], 0, nullptr);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ShadowMappingScene::cubeIndices.size()), 1, 0, 0, 0);

		// STENCIL
/*
		Pipelines::stencilPipeline.bind(commandBuffer);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::stencilPipeline.getLayout(), 0, 1, &stencilDescriptorSets[j][currentFrame], 0, nullptr);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);
*/
/*
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);
*/	
	}

	vkCmdEndRenderPass(commandBuffer);

	scissor.extent = VulkanConfig::swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	// NORMAL PASS
	clearValues[0].color = {{.1f, .1f, .1f, 1.f}};
	clearValues[1].depthStencil = {1.0f, 0};
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	renderPassInfo.renderPass = renderPasses.renderPass;
	renderPassInfo.framebuffer = offScreenFramebuffers[imageIndex];

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	Pipelines::basePipeline.bind(commandBuffer);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &shaderStorageBuffers[currentFrame], offsets);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::basePipeline.getLayout(), 0, 1, &baseDescriptorSets[currentFrame], 0, nullptr);

	vkCmdDraw(commandBuffer, PARTICLE_COUNT, 1, 0, 0);

	Pipelines::meshPipeline.bind(commandBuffer);

	for (size_t i = 0; i < VulkanConfig::MESH_COUNT; i++)
	{
		VkBuffer modelVertexBuffers[] = {vertexBuffers[i]};

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, modelVertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexModelBuffers[i], 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::meshPipeline.getLayout(), 0, 1, &modelDescriptorSets[i][currentFrame], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->meshes[i].indices.size()), 1, 0, 0, 0);
	}

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		Pipelines::primitivePipeline.bind(commandBuffer);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::primitivePipeline.getLayout(), 0, 1, &primitiveDescriptorSets[j][currentFrame], 0, nullptr);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ShadowMappingScene::cubeIndices.size()), 1, 0, 0, 0);

		// STENCIL
/*
		Pipelines::stencilPipeline.bind(commandBuffer);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::stencilPipeline.getLayout(), 0, 1, &stencilDescriptorSets[j][currentFrame], 0, nullptr);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cubeIndices.size()), 1, 0, 0, 0);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);
*/	
	}
	for (size_t j = 0; j < VulkanConfig::MAX_POINT_LIGHTS; j++)
	{
		Pipelines::lightPipeline.bind(commandBuffer);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::lightPipeline.getLayout(), 0, 1, &lightDescriptorSets[j][currentFrame], 0, nullptr);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ShadowMappingScene::cubeIndices.size()), 1, 0, 0, 0);
	}

	Pipelines::cubemapPipeline.bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::cubemapPipeline.getLayout(), 0, 1, &cubemapDescriptorSets[currentFrame], 0, nullptr);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubemapBuffers, offsets);

	vkCmdDraw(commandBuffer, static_cast<uint32_t>(ShadowMappingScene::cubemapVertices.size()), 1, 0, 0);

	//vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdEndRenderPass(commandBuffer);

	// POST PROCESSING PASS
	renderPassInfo.renderPass = renderPasses.postProcessingRenderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	Pipelines::postProcessingPipeline.bind(commandBuffer);
	
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::postProcessingPipeline.getLayout(), 0, 1, &postProcessingDescriptorSets[currentFrame], 0, nullptr);

	vkCmdBindIndexBuffer(commandBuffer, quadIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ShadowMappingScene::quadIndices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}
void ShadowMappingScene::updateUniformBuffer(uint32_t currentImage)
{
	for (size_t i = 0; i < VulkanConfig::MAX_POINT_LIGHTS; i++)
	{
		glm::vec3 lightPos = glm::vec3(sin(3. * glfwGetTime()) + i, cos(3. * i), 3. * i);

		PointLight light{};
			
		light.ambient = glm::vec3(0.1f, .1f, .1f);
		light.diffuse = glm::vec3(.9f * (3. - i), .9f * (2. - i), 0.f);
		light.specular = glm::vec3(1.f * (3. - i), 1. * (2. - i), 0.f);
		light.position = lightPos;	
		light.constant = 1.f;
		light.linear = 0.7f;
		light.quadratic = 1.8f;
		light.color = glm::vec3(1.f * (3. - i), 1. * (2. - i), 0.);
		light.model = glm::mat4(1.);
		light.model = glm::translate(light.model, lightPos);
		light.view = camera.getViewMatrix();
		light.projection = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, 400.f);
		light.projection[1][1] *= -1;

		pointLights[i] = light;

		memcpy(lightObjectUniformBuffersMapped[i][currentImage], &light, sizeof(light));
	}

	for (size_t i = 0; i < VulkanConfig::MESH_COUNT; i++)
	{
		UniformBufferObjectModel meshUBO{};
		DirectionalLight directionalLight;
		SpotLight spotLight;
		glm::mat4 lightSpaceProjection = glm::ortho(-20.f, 20.f, -20.f, 20.f, 0.1f, 15.f);
		glm::mat4 lightView = glm::lookAt(
			glm::vec3(-2.0f, 15.0f, -1.0f), 
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 1.0f, 0.f)
		);

		meshUBO.model = glm::mat4(1.);
		meshUBO.model = glm::scale(meshUBO.model, glm::vec3(0.1,0.1,0.1));
		meshUBO.view = camera.getViewMatrix();
		meshUBO.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
		lightSpaceProjection[1][1] *= -1;
		meshUBO.lightSpaceMatrix = lightSpaceProjection * lightView;
		meshUBO.fragColor = glm::vec3(0., 1., 1.);

		meshUBO.proj[1][1] *= -1;

		memcpy(modelUniformBuffersMapped[i][currentImage], &meshUBO, sizeof(meshUBO));

		for (size_t j = 0; j < VulkanConfig::MAX_POINT_LIGHTS; ++j)
		{
			lights.pointLights[j] = pointLights[j];
		}

		directionalLight.ambient = glm::vec3(0.1f, .1f, .1f);
		directionalLight.diffuse = glm::vec3(.9f, .9f, .9f);
		directionalLight.specular = glm::vec3(1.f);
		directionalLight.direction = glm::normalize(glm::vec3(-2.0f, -15.0f, -1.0f));

		lights.directionalLight = directionalLight;

		spotLight.ambient = glm::vec3(0.1f, .1f, .1f);
		spotLight.diffuse = glm::vec3(.9f, .9f, .9f);
		spotLight.specular = glm::vec3(1.f);
		spotLight.position = camera.cameraPos;
		spotLight.direction = camera.cameraFront;
		spotLight.cutOff = glm::cos(glm::radians(12.5));
		spotLight.outerCutOff = glm::cos(glm::radians(15.5));
		
		lights.spotLight = spotLight;

		memcpy(modelLightUniformBuffersMapped[i][currentImage], &lights, sizeof(lights));
	}

	for (size_t j = 0; j < VulkanConfig::OBJECT_COUNT; j++)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		UniformBufferObjectModel ubom{};
		Material material{};
		DirectionalLight directionalLight;
		SpotLight spotLight;

		glm::mat4 lightSpaceProjection = glm::ortho(-20.f, 20.f, -20.f, 20.f, 0.1f, 15.f);
		glm::mat4 lightView = glm::lookAt(
			glm::vec3(2.0f, 15.0f,1.0f), 
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 1.0f, 0.f)
		);

		glm::vec3 cubePosition = ShadowMappingScene::cubePositions[j];
		glm::vec3 transformedPosition = glm::vec3(cubePosition.x + 2.f, cubePosition.y - 1.f, cubePosition.z);
		//glm::vec3 transformedPosition = cubePosition;
		ubom.model = glm::mat4(1.);
		if (j == 0){
			ubom.model = glm::translate(ubom.model, camera.cameraPos);
		}
		else
		{
			ubom.model = glm::translate(ubom.model, transformedPosition);
		};
		float angle = 20.f * j;

		ubom.model = glm::rotate(ubom.model, glm::radians(angle), glm::vec3(1.f, 0.3f, 0.5f));

		ubom.view = camera.getViewMatrix();
		ubom.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);

		lightSpaceProjection[1][1] *= -1;
		ubom.lightSpaceMatrix = lightSpaceProjection * lightView;
		ubom.fragColor = glm::vec3(0., 1., 1.);
	
		ubom.proj[1][1] *= -1;

		memcpy(primitiveUniformBuffersMapped[j][currentImage], &ubom, sizeof(ubom));

		// STENCIL
		ubom.model = glm::scale(ubom.model, glm::vec3(1.1));

		memcpy(stencilUniformBuffersMapped[j][currentImage], &ubom, sizeof(ubom));

		ubo.deltaTime = lastFrameTime * 2.f;

		memcpy(baseUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

		material.specular = glm::vec3(.5); 	
		material.shininess = 64.f; 

		memcpy(materialUniformBuffersMapped[j][currentImage], &material, sizeof(material));

		for (size_t i = 0; i < VulkanConfig::MAX_POINT_LIGHTS; ++i)
		{
			lights.pointLights[i] = pointLights[i];
		}

		directionalLight.ambient = glm::vec3(0.1f, .1f, .1f);
		directionalLight.diffuse = glm::vec3(.9f, .9f, .9f);
		directionalLight.specular = glm::vec3(1.f);
		directionalLight.direction =  glm::normalize(glm::vec3(-2.0f, -15.0f, -1.0f));

		lights.directionalLight = directionalLight;

		spotLight.ambient = glm::vec3(0.1f, .1f, .1f);
		spotLight.diffuse = glm::vec3(.9f, .9f, .9f);
		spotLight.specular = glm::vec3(1.f);
		spotLight.position = camera.cameraPos;
		spotLight.direction = camera.cameraFront;
		spotLight.cutOff = glm::cos(glm::radians(12.5));
		spotLight.outerCutOff = glm::cos(glm::radians(15.5));
		
		lights.spotLight = spotLight;

		memcpy(lightUniformBuffersMapped[j][currentImage], &lights, sizeof(lights));
	}

	UniformBufferObjectModel cubemapUbo;

	cubemapUbo.model = glm::mat4(1.);
	cubemapUbo.model = glm::translate(cubemapUbo.model, glm::vec3(4., 10., 0.));
	
	cubemapUbo.view = glm::mat4(glm::mat3(camera.getViewMatrix()));
	cubemapUbo.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, 400.f);
	cubemapUbo.proj[1][1] *= -1;

	memcpy(cubemapUniformBuffersMapped[currentImage], &cubemapUbo, sizeof(cubemapUbo));
}

void ShadowMappingScene::recordComputeCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

	vkCmdBindDescriptorSets(
		commandBuffer, 
		VK_PIPELINE_BIND_POINT_COMPUTE, 
		computePipelineLayout,0,1,
		&computeDescriptorSets[currentFrame],0,nullptr);

	vkCmdDispatch(commandBuffer, PARTICLE_COUNT/256, 1, 1);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record compute command buffer!");
	}
}

void ShadowMappingScene::recreateSwapChain(GLFWwindow * window)
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
	createOffscreenResources();
	createShadowMapResources();

	//TODO: move to own function eventually
for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = offScreenImageViews[i];
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = postProcessingDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	createFramebuffers();
}


void ShadowMappingScene::cleanupSwapChain()
{
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

}

VkDevice* ShadowMappingScene::getDevice()
{
	return &device;
}

void ShadowMappingScene::processInput(GLFWwindow * window)
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
//	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
//		steps += 0.1;
//	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
//		steps -= 0.1;
//	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
//		verticalSteps += 0.1;
//	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
//		verticalSteps -= 0.1;
}

void ShadowMappingScene::cleanup(GLFWwindow * window)
{
	cleanupSwapChain();

	delete model;

	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);

	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(device, baseUniformBuffers[i], nullptr);
		vkFreeMemory(device, baseUniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
	
	vkDestroyBuffer(device, vertexCubeBuffer, nullptr);
	vkFreeMemory(device, vertexCubeBufferMemory, nullptr);
	
	for (size_t i = 0; i < vertexBuffers.size(); i++)
	{
		vkDestroyBuffer(device, vertexBuffers[i], nullptr);
		vkFreeMemory(device, vertexBufferMemories[i], nullptr);
	}

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
}

void ShadowMappingScene::deviceWaitIdle()
{
	vkDeviceWaitIdle(device);
}

void ShadowMappingScene::moveCamera(double xpos, double ypos)
{
	camera.move(xpos, ypos);
}
