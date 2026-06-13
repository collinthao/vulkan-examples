#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "./core/engine/engine.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
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
#include "./bindings/particle.h"

#include "./examples/basicApp/basicApp.h"
#include "./examples/grassScene/grassScene.h"
#include "./examples/shadowMappingScene/shadowMappingScene.h"
#include "./examples/omniDirectionalShadowMappingScene/omniDirectionalShadowMappingScene.h"

int main()
{
	Engine<BasicApp, GLFWWindowContext, VulkanRenderer> engine{};
	try
	{
		engine.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failure" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
