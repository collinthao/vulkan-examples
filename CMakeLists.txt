cmake_minimum_required(VERSION 3.22)

project(vulkan_examples)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_DIR src)
set(VENDOR_DIR vendor)

test
set(SOURCES
${SOURCE_DIR}/main/main.cpp
./camera.cpp
./fileContext.cpp
./model.cpp
./vulkanConfig.cpp
${SOURCE_DIR}/core/buffer/buffer.cpp
${SOURCE_DIR}/core/commandBuffer/commandBuffer.cpp
${SOURCE_DIR}/core/descriptors/descriptor.cpp
${SOURCE_DIR}/core/image/image.cpp
${SOURCE_DIR}/core/image/texture/texture.cpp
${SOURCE_DIR}/core/image/texture/sampler/sampler.cpp
${SOURCE_DIR}/core/pipeline/pipeline.cpp
${SOURCE_DIR}/core/queueFamily/queueFamily.cpp
${SOURCE_DIR}/core/renderer/vulkanRenderer.cpp
${SOURCE_DIR}/core/vulkanImage/vulkanImage.cpp
${SOURCE_DIR}/core/windowContext/glfwWindowContext.cpp
${SOURCE_DIR}/examples/grassScene/grassScene.cpp
${SOURCE_DIR}/examples/shadowMappingScene/shadowMappingScene.cpp
${SOURCE_DIR}/examples/omniDirectionalShadowMappingScene/omniDirectionalShadowMappingScene.cpp
) 

add_executable(vulkan_examples ${SOURCES})

target_include_directories(vulkan_examples PRIVATE 
${SOURCE_DIR}
${VENDOR_DIR}/glfw/include
${VENDOR_DIR}/vulkan/include
${VENDOR_DIR}/glm/include
${VENDOR_DIR}/stb/include
${VENDOR_DIR}/assimp/include)

target_link_libraries(vulkan_examples PRIVATE 
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/glfw/lib/glfw3dll.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/glm/lib/glm.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/vulkan/lib/vulkan-1.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/spirv/lib/SPIRV-Tools.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/spirv/lib/SPIRV-Tools-diff.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/spirv/lib/SPIRV-Tools-opt.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/glslang/lib/GenericCodeGen.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/glslang/lib/glslang.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/glslang/lib/glslang-default-resource-limits.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/glslang/lib/MachineIndependent.lib"
"${CMAKE_CURRENT_SOURCE_DIR}/${VENDOR_DIR}/assimp/lib/assimp-vc143-mt.lib")

target_compile_definitions(vulkan_examples PRIVATE PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

