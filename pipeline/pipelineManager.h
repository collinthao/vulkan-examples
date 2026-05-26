#pragma once
#include "./pipelineBuilder.h"
#include "./pipeline.h"

namespace Pipelines
{
	inline	PipelineBuilder pipelineBuilder{};
	inline	Pipeline primitivePipeline; 
	inline	Pipeline shadowMapPipeline; 
	inline	Pipeline shadowMapPrimitivePipeline; 
	inline	Pipeline shadowMapMeshPipeline; 
	inline	Pipeline basePipeline; 
	inline	Pipeline stencilPipeline;
	inline	Pipeline lightPipeline;
	inline	Pipeline meshPipeline;
	inline	Pipeline cubemapPipeline;
	inline	Pipeline postProcessingPipeline;
	inline	Pipeline screenSpacePipeline;

	struct
	{
		VkDescriptorSetLayoutBinding samplerUniformLayoutBinding{};
		VkDescriptorSetLayoutBinding specularUniformLayoutBinding{};
		VkDescriptorSetLayoutBinding vertexLayoutBinding{};
		VkDescriptorSetLayoutBinding fragmentLayoutBinding{};
		VkDescriptorSetLayoutBinding allStagesUniformLayoutBinding{};
	} layoutBindings{};

	inline void setDescriptorSetLayoutBindings()
	{
		layoutBindings.samplerUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings.samplerUniformLayoutBinding.descriptorCount = 1;
		layoutBindings.samplerUniformLayoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.samplerUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		layoutBindings.specularUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings.specularUniformLayoutBinding.descriptorCount = 1;
		layoutBindings.specularUniformLayoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.specularUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		layoutBindings.vertexLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings.vertexLayoutBinding.descriptorCount = 1;
		layoutBindings.vertexLayoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.vertexLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

		layoutBindings.fragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings.fragmentLayoutBinding.descriptorCount = 1;
		layoutBindings.fragmentLayoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.fragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		layoutBindings.allStagesUniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings.allStagesUniformLayoutBinding.descriptorCount = 1;
		layoutBindings.allStagesUniformLayoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.allStagesUniformLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
	}

	template <typename T>
	void createPipelines(VkDevice& device, T renderPasses)
	{

		setDescriptorSetLayoutBindings();

		std::vector<VkDescriptorSetLayoutBinding> shadowMapBindings = 
		{
			layoutBindings.vertexLayoutBinding, 
			layoutBindings.fragmentLayoutBinding, 
			layoutBindings.samplerUniformLayoutBinding,
			layoutBindings.allStagesUniformLayoutBinding,
			layoutBindings.specularUniformLayoutBinding,
		};

		std::vector<VkDescriptorSetLayoutBinding> primitiveBindings = {
			layoutBindings.vertexLayoutBinding, 
			layoutBindings.fragmentLayoutBinding, 
			layoutBindings.samplerUniformLayoutBinding,
			layoutBindings.allStagesUniformLayoutBinding,
			layoutBindings.specularUniformLayoutBinding,
			layoutBindings.samplerUniformLayoutBinding,
			layoutBindings.vertexLayoutBinding, 
			layoutBindings.vertexLayoutBinding 
		};

		std::vector<VkDescriptorType> shadowMapTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		};

		std::vector<VkDescriptorType> primitiveTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		};
		
		std::vector<VkDescriptorSetLayoutBinding> meshBindings = {
		layoutBindings.vertexLayoutBinding, 
		layoutBindings.samplerUniformLayoutBinding,
		layoutBindings.vertexLayoutBinding,
		layoutBindings.samplerUniformLayoutBinding,
			};

		std::vector<VkDescriptorType> meshTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};

		std::vector<VkDescriptorSetLayoutBinding> cubemapBindings = {
			layoutBindings.samplerUniformLayoutBinding,
			layoutBindings.vertexLayoutBinding
		};

		std::vector<VkDescriptorType> cubemapTypes = {
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		};

		shadowMapPipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/shadowmapVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/shadowmapFrag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.setAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0)
			.setAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 4)
			.setAttributeDescription(0, 2, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8)
			.setAttributeDescription(0, 3, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 12)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.setMSAASamples(VK_SAMPLE_COUNT_1_BIT)
			.setDescriptor(shadowMapBindings, shadowMapTypes, VulkanConfig::OBJECT_COUNT, device)
			.setStencilTest(VK_TRUE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS)	
			.setStencilWriteMask(0xFF)	
			.setDepthTest(VK_TRUE)
			.setDepthWrite(VK_TRUE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS)
			.setCullMode(VK_CULL_MODE_NONE)
			.setCullFace(VK_FRONT_FACE_CLOCKWISE)
			.setRenderPass(renderPasses.shadowMapRenderPass)
			.build(device);

		shadowMapPrimitivePipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/shadowMapPrimitiveVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/shadowMapPrimitiveFrag.spv"}})
			.setRenderPass(renderPasses.shadowMapRenderPass)
			.build(device);

		primitivePipeline = 
			pipelineBuilder
			.setShaderPaths(
				{{VK_SHADER_STAGE_VERTEX_BIT, "shaders/grass/grassVert.spv"}, 
				{VK_SHADER_STAGE_GEOMETRY_BIT, "shaders/grass/grassGeom.spv"},
				{VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/grass/grassFrag.spv"}})
			.setBindingDescription(1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
			.setAttributeDescription(1, 4, VK_FORMAT_R32G32B32_SFLOAT, 0)
			.setAttributeDescription(1, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3)
			.setAttributeDescription(1, 6, VK_FORMAT_R32_SFLOAT, sizeof(float) * 6)
			.setAttributeDescription(1, 7, VK_FORMAT_R32_UINT, sizeof(float) * 7)
			.setMSAASamples(VulkanConfig::msaaSamples)
			.setCullMode(VK_CULL_MODE_NONE)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
			.setDescriptor(primitiveBindings, primitiveTypes, VulkanConfig::OBJECT_COUNT, device)
			.setRenderPass(renderPasses.renderPass)
			.build(device);
/*	
		shadowMapMeshPipeline = pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/shadowMapMeshVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/meshFrag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.clearBindingDescription(1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE) // Clear Attribute and Binding Descriptions. Don't think it's working...
			.clearAttributeDescription(1, 4, VK_FORMAT_R32G32B32_SFLOAT, 0)
			.clearAttributeDescription(1, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 4)
			.clearAttributeDescription(1, 6, VK_FORMAT_R32_SFLOAT, sizeof(float) * 8)
			.clearAttributeDescription(1, 7, VK_FORMAT_R32G32_UINT, sizeof(float) * 9)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.setMSAASamples(VK_SAMPLE_COUNT_1_BIT)
			.setDescriptor(meshBindings, meshTypes, VulkanConfig::MESH_COUNT, device)
			.setStencilTest(VK_FALSE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS)	
			.setStencilWriteMask(0xFF)	
			.setDepthTest(VK_TRUE)
			.setDepthWrite(VK_TRUE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS)
			.setCullMode(VK_CULL_MODE_BACK_BIT)
			.setCullFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.setRenderPass(renderPasses.shadowMapRenderPass)
			.build(device);
*/	
		basePipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/vert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/frag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.clearBindingDescription(1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
			.clearAttributeDescription(1, 4, VK_FORMAT_R32G32B32_SFLOAT, 0)
			.clearAttributeDescription(1, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 4)
			.clearAttributeDescription(1, 6, VK_FORMAT_R32_SFLOAT, sizeof(float) * 8)
			.clearAttributeDescription(1, 7, VK_FORMAT_R32_UINT, sizeof(float) * 9)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
			.setMSAASamples(VulkanConfig::msaaSamples)
			.setDescriptor({layoutBindings.vertexLayoutBinding, layoutBindings.samplerUniformLayoutBinding}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 1, device)
			.setStencilTest(VK_TRUE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS)	
			.setStencilWriteMask(0xFF)	
			.setDepthTest(VK_TRUE)
			.setDepthWrite(VK_TRUE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS)
			.setCullMode(VK_CULL_MODE_NONE)
			.setCullFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.setRenderPass(renderPasses.renderPass)
			.build(device);

		stencilPipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/stencilVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/stencilFrag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.setMSAASamples(VulkanConfig::msaaSamples)
			.setDescriptor({layoutBindings.vertexLayoutBinding}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, VulkanConfig::OBJECT_COUNT, device)
			.setStencilTest(VK_TRUE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NOT_EQUAL)	
			.setStencilWriteMask(0x00)	
			.setDepthTest(VK_FALSE)
			.setDepthWrite(VK_FALSE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS)
			.setCullMode(VK_CULL_MODE_NONE)
			.setCullFace(VK_FRONT_FACE_CLOCKWISE)
			.setRenderPass(renderPasses.renderPass)
			.build(device);

		lightPipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/lightVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/lightFrag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.setMSAASamples(VulkanConfig::msaaSamples)
			.setDescriptor({layoutBindings.vertexLayoutBinding}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, VulkanConfig::MAX_POINT_LIGHTS, device)
			.setStencilTest(VK_TRUE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS)	
			.setStencilWriteMask(0xFF)	
			.setDepthTest(VK_TRUE)
			.setDepthWrite(VK_TRUE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS)
			.setCullMode(VK_CULL_MODE_NONE)
			.setCullFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.setRenderPass(renderPasses.renderPass)
			.build(device);
/*	
		meshPipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/meshVert.spv"},{VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/meshFrag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.setMSAASamples(VulkanConfig::msaaSamples)
			.setDescriptor(meshBindings, meshTypes, VulkanConfig::MESH_COUNT, device)
			.setStencilTest(VK_FALSE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS)	
			.setStencilWriteMask(0xFF)	
			.setDepthTest(VK_TRUE)
			.setDepthWrite(VK_TRUE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS)
			.setCullMode(VK_CULL_MODE_BACK_BIT)
			.setCullFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.setRenderPass(renderPasses.renderPass)
			.build(device);
*/	
		cubemapPipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/cubemapVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/cubemapFrag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.setMSAASamples(VulkanConfig::msaaSamples)
			.setDescriptor(cubemapBindings, cubemapTypes, 1,device)
			.setStencilTest(VK_FALSE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS)	
			.setStencilWriteMask(0xFF)	
			.setDepthTest(VK_TRUE)
			.setDepthWrite(VK_TRUE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
			.setCullMode(VK_CULL_MODE_NONE)
			.setCullFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.setRenderPass(renderPasses.renderPass)
			.build(device);

		postProcessingPipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/postprocessingVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/postprocessingFrag.spv"}})
			.setBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.setMSAASamples(VK_SAMPLE_COUNT_1_BIT)
			.setDescriptor({layoutBindings.samplerUniformLayoutBinding}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 1, device)
			.setStencilTest(VK_FALSE)
			.setStencilState(VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS)	
			.setStencilWriteMask(0xFF)	
			.setDepthTest(VK_FALSE)
			.setDepthWrite(VK_TRUE)
			.setDepthCompareOp(VK_COMPARE_OP_LESS)
			.setCullMode(VK_CULL_MODE_NONE)
			.setCullFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.setRenderPass(renderPasses.postProcessingRenderPass)
			.build(device);

		screenSpacePipeline = 
			pipelineBuilder
			.setShaderPaths({{VK_SHADER_STAGE_VERTEX_BIT, "shaders/screenSpaceQuadVert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/screenSpaceQuadFrag.spv"}})
			.setMSAASamples(VulkanConfig::msaaSamples)
			.setDescriptor({layoutBindings.samplerUniformLayoutBinding}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, 1, device)
			.setRenderPass(renderPasses.renderPass)
			.build(device);

	}

}
