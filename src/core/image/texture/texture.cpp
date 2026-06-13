#include "./texture.h"
#include "../../buffer/buffer.h"
#include <cmath>
#include <cstring>
#define max(a,b)

void Image::Texture::loadTexture(const std::string imagePath, int& width, int& height, int& channels, int layers,stbi_uc*& texture)
{
	texture = stbi_load(imagePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!texture)
	{
		std::cout << "failed to load texture image! Path: " + imagePath << "\n";
		throw std::runtime_error("failed to load texture image! Path: " + imagePath);
	}
}

void Image::Texture::create(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsAndComputeQueue)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels;

	loadTexture(imagePath, texWidth, texHeight, texChannels, 1, pixels);
	
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	stbi_image_free(pixels);

	Image::create(texWidth, 
			texHeight, 
			mipLevels,
			1,0, VK_IMAGE_TYPE_2D,
			VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			image, 
			imageMemory,
			VK_IMAGE_LAYOUT_UNDEFINED, device, physicalDevice);

	Image::transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1, device, graphicsAndComputeQueue, VK_IMAGE_ASPECT_COLOR_BIT);
	Image::copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1, device, graphicsAndComputeQueue);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	Image::generateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 1, device, physicalDevice, graphicsAndComputeQueue);
	
}
