#include <string>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "mesh.h"
#include <vector>
#include <iostream>
#include <string_view>
#include <thread>
#include "../config/vulkanConfig.h"
#include "../core/buffer/buffer.h"

struct Uniform
{
	std::vector<void *>         mapped;
	std::vector<VkBuffer>       buffer;
	std::vector<VkDeviceMemory> memory;
};

class Model
{
public:
	Model();
	Model(std::string path, std::string texturePath, const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkQueue& queue);
	const aiScene * scene;	
	std::array<Uniform, 2> uniforms; 
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;
	bool gammaCorrection;
	uint32_t nodeIndex = 0;

	#if defined(_WIN32) || defined(_WIN64)
		const std::string ROOT_DIR = std::string{GetExecutableDir()};
	#else
		const std::string ROOT_DIR = PROJECT_ROOT_DIR;
	#endif

	struct
	{
		std::vector<VkBuffer> buffers;
		std::vector<VkBuffer> index;
		std::vector<VkDeviceMemory> memory;
		std::vector<VkDeviceMemory> indexMemory;
	} buffer;

	struct
	{
		VkSampler sampler;
		std::vector<VkImage>     image;
		std::vector<VkImageView> imageView;
		std::vector<VkDeviceMemory> imageMemory;
	} texture;		

	void getMeshes();
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh*mesh, const aiScene * scene);

	template <typename UniformData>
	void bind(UniformData u, uint32_t frame)
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			memcpy(uniforms[frame].mapped[i], &u, sizeof(u));
		};
	};	

	template <typename UniformStruct>
	void setupUniforms()
	{
		VkDeviceSize objectBufferSize = sizeof(UniformStruct);	
		for (size_t j = 0; j < VulkanConfig::MAX_FRAMES_IN_FLIGHT; j++)
		{
			uniforms[j].buffer.resize(meshes.size());
			uniforms[j].mapped.resize(meshes.size());
			uniforms[j].memory.resize(meshes.size());

			for (size_t i = 0; i < meshes.size(); i++)
			{
				Buffer::create(objectBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniforms[j].buffer[i], uniforms[j].memory[i], VulkanConfig::device, VulkanConfig::physicalDevice);
				
				vkMapMemory(VulkanConfig::device, uniforms[j].memory[i], 0, objectBufferSize, 0, &uniforms[j].mapped[i]);
			};
		};
	};

private:

	std::string modelPath;
	std::string texturePath;
	VkDevice device;
	VkQueue queue;
	VkPhysicalDevice physicalDevice;
	std::vector<Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName);
	void setupModelData();
	void setupBuffers(const Mesh& mesh, const int& index);
	void setupIndexBuffers(const Mesh& mesh, const int& index);
	void setupImages(const int& index);
	void setupImageViews(const int& index);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

};

