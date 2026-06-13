cmake_minimum_required(VERSION 3.22)

project(vulkan_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

set(SOURCES
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/camera/camera.cpp
    ${SOURCE_DIR}/fileContext/fileContext.cpp
    ${SOURCE_DIR}/model/model.cpp
    ${SOURCE_DIR}/config/vulkanConfig.cpp
    ${SOURCE_DIR}/core/buffer/buffer.cpp
    ${SOURCE_DIR}/core/commandBuffer/commandBuffer.cpp
    ${SOURCE_DIR}/core/descriptors/descriptor.cpp
    ${SOURCE_DIR}/core/image/image.cpp
    ${SOURCE_DIR}/core/image/texture/texture.cpp
    ${SOURCE_DIR}/core/image/texture/sampler/sampler.cpp
    ${SOURCE_DIR}/core/pipeline/pipeline.cpp
    ${SOURCE_DIR}/core/queueFamily/queueFamily.cpp
    ${SOURCE_DIR}/core/renderer/vulkanRenderer.cpp
    ${SOURCE_DIR}/core/renderer/vulkanApp/vulkanApp.cpp
    ${SOURCE_DIR}/core/vulkanImage/vulkanImage.cpp
    ${SOURCE_DIR}/core/windowContext/glfwWindowContext.cpp
    ${SOURCE_DIR}/core/windowContext/glfwWindowContext.cpp
    ${SOURCE_DIR}/examples/basicApp/basicApp.cpp
    ${SOURCE_DIR}/examples/grassScene/grassScene.cpp
    ${SOURCE_DIR}/examples/shadowMappingScene/shadowMappingScene.cpp
    ${SOURCE_DIR}/examples/omniDirectionalShadowMappingScene/omniDirectionalShadowMappingScene.cpp
)

add_executable(vulkan_examples ${SOURCES})

target_include_directories(vulkan_examples PRIVATE
    ${SOURCE_DIR}
    ${VENDOR_DIR}/glm/include
    ${VENDOR_DIR}/stb/include
)

find_package(Vulkan REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(vulkan_examples PRIVATE
    Vulkan::Vulkan
    assimp::assimp
    glfw
    glm::glm
)

if(WIN32)
    target_compile_definitions(vulkan_examples PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(vulkan_examples PRIVATE VK_USE_PLATFORM_XCB_KHR)
elseif(APPLE)
    target_compile_definitions(vulkan_examples PRIVATE VK_USE_PLATFORM_METAL_EXT)
endif()

add_compile_definitions(NOMINMAX)

target_compile_definitions(vulkan_examples PRIVATE
 NOMINMAX PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
