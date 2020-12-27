#pragma once

#include <vulkan/vulkan.h>

#include "VulkanInstance.h"
#include "VulkanDevice.h"

#include "lodepng.h"

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include <set>

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

// struct Pixel {
//     float r, g, b, a;
// };

// const std::vector<const char*> deviceExtensions = {
// };

// struct QueueFamilyIndices {
//     std::optional<uint32_t> computeFamily;

//     bool isComplete() {
//         return computeFamily.has_value();
//     }
// };

class VulkanApplication {
public:
    void run();
    uint32_t bufferSize;

private:
    // GLFWwindow* window;

    VulkanInstance instance;
    // VkDebugUtilsMessengerEXT debugMessenger;
    // VkSurfaceKHR surface;

    // VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; 
    // VkDevice device;
    VulkanDevice device;


    // VkQueue graphicsQueue;
    // VkQueue computeQueue;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;    
    
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    

    void initWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    void initVulkan();
    void mainLoop();
    void cleanup();
    void recreatePipeline();
    // void createInstance();
    // void setupDebugMessenger();
    // void pickPhysicalDevice();
    // void createLogicalDevice();
    void createPipeline();
    void createCommandPool();
    void createCommandBuffers();

    VkShaderModule createShaderModule(const std::vector<char>& code);
    // bool isDeviceSuitable(VkPhysicalDevice device);

    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
    void createBuffer();
    void createDescriptorSetLayout();
    void createDescriptorSet();

    void runCommandBuffer();

    void saveRenderedImage();

    // bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    // QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    // std::vector<const char*> getRequiredExtensions();
    static std::vector<char> readFile(const std::string& filename);
    };