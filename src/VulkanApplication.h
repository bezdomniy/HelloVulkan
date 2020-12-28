#pragma once

#include <vulkan/vulkan.h>

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"

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
    float aspectRatio;                        // Aspect ratio of the viewport
    glm::vec4 fogColor = glm::vec4(0.0f);
    struct {
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 4.0f);
        glm::vec3 lookat = glm::vec3(0.0f, 0.5f, 0.0f);
        float fov = 10.0f;
    } camera;
} ubo;

class VulkanApplication {
public:
    VulkanApplication();
    ~VulkanApplication();
    void run();
    uint32_t inBufferSize;
    uint32_t outBufferSize;

private:
    VulkanInstance instance;
    VulkanDevice device;
    VulkanPipeline pipeline;

//    VkDescriptorPool descriptorPool;
//    VkDescriptorSet descriptorSet;
//    VkDescriptorSetLayout descriptorSetLayout;
//    
//    VkPipelineLayout pipelineLayout;
//    VkPipeline pipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
//    void recreatePipeline();
//    void createPipeline();
    void createCommandPool();
    void createCommandBuffers();

//    VkShaderModule createShaderModule(const std::vector<char>& code);
//
//    void createDescriptorSetLayout();
//    void createDescriptorSet();

    void runCommandBuffer();

    void saveRenderedImage();
    
    void updateUniformBuffers();

    static std::vector<char> readFile(const std::string& filename);
    };
