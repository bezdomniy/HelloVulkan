#pragma once

#include <vulkan/vulkan.h>

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"

#include "Primitives.h"

#include "lodepng.h"

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const uint32_t WORKGROUP_SIZE = 32;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

struct UBOCompute {                            // Compute shader uniform block object
    glm::vec3 lightPos;
    Primitives::Camera camera;
} ubo;

class VulkanApplication {
public:
    VulkanApplication();
    ~VulkanApplication();
    void run();
    uint32_t uniformBufferSize;
    uint32_t shapesBufferSize;
    uint32_t outBufferSize;
    
    std::vector<Primitives::Shape> shapes;

private:
    VulkanInstance instance;
    VulkanDevice device;
    VulkanPipeline pipeline;
    
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createCommandPool();
    void createCommandBuffers();

    void runCommandBuffer();

    void saveRenderedImage();
    
    void updateUniformBuffers();
    void createShapes();

    };
