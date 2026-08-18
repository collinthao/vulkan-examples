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
#include "./examples/textureMapping/textureMapping.h"
#include "./examples/depthTesting/depthTesting.h"
#include "./examples/antialiasing/antialiasing.h"
#include "./examples/modelLoading/modelLoading.h"
#include "./examples/phong/phong.h"
#include "./examples/directionalLight/directionalLight.h"
#include "./examples/postProcessing/postProcessing.h"
#include "./examples/shadowMapping/shadowMapping.h"
#include "./examples/omnidirectionalShadowMapping/omnidirectionalShadowMapping.h"
#include "./examples/normalMapping/normalMapping.h"

int main()
{
	Engine<NormalMapping, GLFWWindowContext, VulkanRenderer> engine{};
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
