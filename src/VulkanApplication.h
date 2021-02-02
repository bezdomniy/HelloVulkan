#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <vulkan/vulkan.h>

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"

#include "Primitives.h"

#include "lodepng.h"
#include "ImageWriter.h"

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <vector>

const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 1200;
const uint32_t WORKGROUP_SIZE = 32;

const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

struct UBOCompute
{ // Compute shader uniform block object
    glm::vec4 lightPos;
    alignas(16) Primitives::Camera camera;
} ubo;

class VulkanApplication
{
public:
    VulkanApplication();
    ~VulkanApplication();
    void run();
    size_t uniformBufferSize;
    size_t shapesBufferSize;
    size_t meshBufferSize;
    size_t bvhBufferSize;
    size_t blasBufferSize;
    size_t outBufferSize;

    std::vector<Primitives::Shape> shapes;
    Primitives::Mesh *mesh;
    
    Primitives::BVH *bvh;
    std::vector<Primitives::NodeBLAS> blas;

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
    void createCommandBuffer(VkCommandBuffer &cmdBuffer);
    void destroyCommandBuffer(VkCommandBuffer &cmdBuffer, bool end);
    void finaliseMainCommandBuffer();
    //    void flushCommandBuffer(VkCommandBuffer commandBuffer, bool free);
    void addSSBOBuffer(void* buffer, size_t bufferSize, VkCommandBuffer& copyCmd, VkBufferCopy copyRegion);

    void runCommandBuffer(VkCommandBuffer commandBuffer, bool end, bool free);

    void saveRenderedImage();

    void updateUniformBuffers();
    void createShapes();
};
