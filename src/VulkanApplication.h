#pragma once

#include <vulkan/vulkan.h>

#include "VulkanInstance.h"
#include "VulkanDevice.h"

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

class VulkanApplication {
public:
    void run();
    uint32_t inBufferSize;
    uint32_t outBufferSize;

private:
    VulkanInstance instance;
    VulkanDevice device;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void recreatePipeline();
    void createPipeline();
    void createCommandPool();
    void createCommandBuffers();

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void createDescriptorSetLayout();
    void createDescriptorSet();

    void runCommandBuffer();

    void saveRenderedImage();

    static std::vector<char> readFile(const std::string& filename);
    };
