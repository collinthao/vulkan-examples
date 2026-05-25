#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <array>
#include <iostream>

// Move to separate file
struct InstanceData {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 scale;
	alignas(4) float rot{ 0.0f };
	alignas(4) uint32_t id{0};

    static inline std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(InstanceData, pos);

        attributeDescriptions[5].binding = 1;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(InstanceData, scale);

	attributeDescriptions[6].binding = 1;
        attributeDescriptions[6].location = 6;
        attributeDescriptions[6].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[6].offset = offsetof(InstanceData, rot);

	attributeDescriptions[7].binding = 1;
        attributeDescriptions[7].location = 7;
        attributeDescriptions[7].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[7].offset = offsetof(InstanceData, id);

        return attributeDescriptions;
    };

    static inline VkVertexInputBindingDescription getBindingDescription()
    {
    	VkVertexInputBindingDescription bindingDescription{};
    
    	bindingDescription.binding = 1;
    	bindingDescription.stride = sizeof(InstanceData);
    	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    
    	return bindingDescription;
    };


};

struct Vertex
{
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec3 normal;
	alignas(16) glm::vec2 texCoord;

    static inline std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

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

        return attributeDescriptions;
    };

    static inline VkVertexInputBindingDescription getBindingDesciption()
    {
    	VkVertexInputBindingDescription bindingDescription{};
    
    	bindingDescription.binding = 0;
    	bindingDescription.stride = sizeof(Vertex);
    	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    	return bindingDescription;
    };

    bool operator==(const Vertex& other) const
    {
    	return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
    };
};
