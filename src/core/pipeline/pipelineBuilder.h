#ifndef PIPELINEBUILDER_H
#define PIPELINEBUILDER_H
#include "./builder.h"
#include <stdexcept>
#include "../../config/vulkanConfig.h"
#include "../../fileContext/fileContext.h"
#include <unordered_set>

namespace std 
{
	template<> struct hash<VkVertexInputAttributeDescription>
	{
		size_t operator()(VkVertexInputAttributeDescription const& attribute) const
		{
			return ((hash<uint32_t>()(attribute.location) ^
				(hash<uint32_t>()(attribute.binding) << 1)) >> 1) ^
					(hash<uint32_t>()(static_cast<uint32_t>(attribute.format)) << 1) >> 1 ^
(hash<uint32_t>()(attribute.offset)) ;
		}
	};
};


namespace std 
{
	template<> struct hash<VkVertexInputBindingDescription>
	{
		size_t operator()(VkVertexInputBindingDescription const& binding) const
		{
			return ((hash<uint32_t>()(binding.binding) ^
				(hash<uint32_t>()(binding.stride) << 1)) >> 1) ^
					(hash<uint32_t>()(static_cast<uint32_t>(binding.inputRate)) << 1);
		}
	};
};

inline bool operator ==(const VkVertexInputAttributeDescription& a, const VkVertexInputAttributeDescription& b)
{
	return a.location == b.location &&
		a.binding == b.binding &&
		a.format == b.format &&
		a.offset == b.offset;		
};

inline bool operator ==(const VkVertexInputBindingDescription& a, const VkVertexInputBindingDescription& b)
{
	return a.binding == b.binding &&
		a.stride == b.stride &&
		a.inputRate == b.inputRate;		
};

class PipelineBuilder : private Builder
{
	
	private: 
	static inline int attributeDescriptionCounter{0};
	Descriptor descriptor;
	std::vector<ShaderContext> shaders;
	std::unordered_set<VkVertexInputBindingDescription> bindingDescriptions;
	std::unordered_set<VkVertexInputAttributeDescription> attributeDescriptions;
	VkPrimitiveTopology primitiveTopology;
	VkSampleCountFlagBits msaaSamples;
	VkDescriptorSetLayout setLayouts;
	VkBool32 stencilTestEnable;
	VkBool32 depthTestEnable;
	VkBool32 depthWriteEnable;
	VkStencilOp failOp; 
	VkStencilOp passOp; 
	uint32_t stencilWriteMask; 
	VkStencilOp depthFailOp; 
	VkCompareOp compareOp;
	VkRenderPass renderPass;
	VkFrontFace cullFace;
	VkCullModeFlags cullMode;
	VkCompareOp depthCompareOp;
	int pipelineCounter = 0;

	VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice& device)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			std::cout << "failed to create shader module!\n";
			throw std::runtime_error("failed to create shader module");
		}

		return shaderModule;
	}

	VkPipelineLayout createPipelineLayout(VkDevice& device, VkPipelineLayout pipelineLayout)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptor.layout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
		return pipelineLayout;
	}

	public:
	PipelineBuilder(){};

	PipelineBuilder& setDescriptor(std::vector<VkDescriptorSetLayoutBinding> bindings, std::vector<VkDescriptorType> types, uint32_t count,VkDevice& device)
	{
		for (int i = 0; i < bindings.size(); i++)
		{
		}

		DescriptorBuilder builder{};
		builder.setBindings(bindings);
		builder.setTypes(types);
		builder.setCount(count);
		descriptor = builder.build(device);
		
	
		return *this;
	}

	PipelineBuilder& setDepthCompareOp(VkCompareOp compareOp)
	{
		depthCompareOp = compareOp;
		return *this;
	};

	PipelineBuilder& setCullFace(VkFrontFace face)
	{
		cullFace = face;
		return *this;
	}


	PipelineBuilder& setCullMode(VkCullModeFlags mode)
	{
		cullMode = mode;
		return *this;
	}

	PipelineBuilder& setShaderPaths(std::vector<ShaderContext> shaderInfo)
	{
		for (size_t i = 0; i < shaderInfo.size(); i++)
		{
		}
		shaders = shaderInfo;
		return *this;
	};

	PipelineBuilder& setBindingDescription(uint32_t binding, size_t stride, VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription bindingDescription{};
    
		bindingDescription.binding = binding;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = inputRate;
 
		bindingDescriptions.insert(std::move(bindingDescription));
		return *this;
	};

	PipelineBuilder& setAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
	{
		VkVertexInputAttributeDescription attributeDescription;
		attributeDescription.binding = binding;
		attributeDescription.location = location;
		attributeDescription.format = format;
		attributeDescription.offset = offset;
		attributeDescriptions.insert(std::move(attributeDescription));
		return *this;
	};

	PipelineBuilder& clearBindingDescription(uint32_t binding, size_t stride, VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription description{};
    
		description.binding = binding;
		description.stride = stride;
		description.inputRate = inputRate;
 
		bindingDescriptions.erase(description);
		return *this;
	}

	PipelineBuilder& clearAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
	{
		VkVertexInputAttributeDescription description;
		description.binding = binding;
		description.location = location;
		description.format = format;
		description.offset = offset;

		attributeDescriptions.erase(description);
		return *this;
	}

	PipelineBuilder& setTopology(VkPrimitiveTopology topology)
	{
		primitiveTopology = topology;
		return *this;
	};

	PipelineBuilder& setMSAASamples(VkSampleCountFlagBits samples)
	{
		msaaSamples = samples;
		return *this;
	};
	PipelineBuilder& setDescriptorSetLayout(VkDescriptorSetLayout layouts)
	{
		setLayouts = layouts;
		return *this;
	};
	PipelineBuilder& setStencilTest(VkBool32 stencilTest)
	{
		stencilTestEnable = stencilTest;
		return *this;
	};

	PipelineBuilder& setStencilState(VkStencilOp fail, VkStencilOp pass, VkStencilOp depthFail, VkCompareOp compare)
	{
		failOp = fail;
		passOp = pass;
		depthFailOp = depthFail;
		compareOp = compare;
		return *this;
	};

	PipelineBuilder& setStencilWriteMask(uint32_t mask)
	{
		stencilWriteMask = mask;
		return *this;
	};

	PipelineBuilder& setDepthTest(VkBool32 depthTest)
	{
		depthTestEnable = depthTest;
		return *this;
	};
	PipelineBuilder& setDepthWrite(VkBool32 depthWrite)
	{
		depthWriteEnable = depthWrite;
		return *this;
	};
	PipelineBuilder& setRenderPass(VkRenderPass pass)
	{
		renderPass = pass;
		return *this;
	};

	Pipeline build(VkDevice& device)
	{	
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout{}; 
		pipelineLayout = createPipelineLayout(device, pipelineLayout);

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(shaders.size());

		for (const auto& shader : shaders)
		{
			auto shaderCode = FileContext::readFile(shader.path);

			VkShaderModule shaderModule = createShaderModule(shaderCode, device);
			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = shader.stage;
			shaderStageInfo.module = shaderModule;
			shaderStageInfo.pName = "main";
		
			assert(shader.stage != 0);

			shaderStages.push_back(shaderStageInfo);
		}

		std::vector<VkVertexInputBindingDescription> bindings{bindingDescriptions.begin(), bindingDescriptions.end()};
		std::vector<VkVertexInputAttributeDescription> attributes{attributeDescriptions.begin(), attributeDescriptions.end()};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
		vertexInputInfo.pVertexBindingDescriptions = bindings.data();
		vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = primitiveTopology;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)VulkanConfig::swapChainExtent.width;
		viewport.height = (float)VulkanConfig::swapChainExtent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// stencil info
		VkStencilOpState stencilOpState{};
		stencilOpState.failOp = failOp;
		stencilOpState.passOp = passOp;
		stencilOpState.depthFailOp = depthFailOp;
		stencilOpState.compareOp = compareOp;
		stencilOpState.compareMask = 0xFF;
		stencilOpState.writeMask = stencilWriteMask;
		stencilOpState.reference = 1;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = depthTestEnable;
		depthStencil.depthWriteEnable = depthWriteEnable;
		depthStencil.depthCompareOp = depthCompareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.f;
		depthStencil.stencilTestEnable = stencilTestEnable;
		depthStencil.front = stencilOpState;
		depthStencil.back = stencilOpState;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = cullMode;
		rasterizer.frontFace = cullFace;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.f;
		rasterizer.depthBiasClamp = 0.f;
		rasterizer.depthBiasSlopeFactor = 0.f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading = 0.2f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.f;
		colorBlending.blendConstants[1] = 0.f;
		colorBlending.blendConstants[2] = 0.f;
		colorBlending.blendConstants[3] = 0.f;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size(); // Two shader stages; vertex and fragment
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
		std::cout << "Pipeline built\n";

		//END

		return Pipeline(device, pipeline, pipelineLayout, descriptor);
	}
};

#endif
