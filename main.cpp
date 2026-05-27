#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "./engine/engine.h"
#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <map>
#include <unordered_map>
#include <array>
#include <random>
#include "particle.h"
//#include "./renderer/vulkanApp/grassScene/grassScene.h"
#include "./renderer/vulkanApp/shadowMappingScene/shadowMappingScene.h"

int main()
{
	Engine<ShadowMappingScene, GLFWWindowContext, VulkanRenderer> engine{};
	try
	{
		engine.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
