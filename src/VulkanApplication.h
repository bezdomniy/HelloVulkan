// HelloVulkan.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

#include <vector>
#include <set>
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanApplication
{
private:
    VkInstance instance;
    std::vector<VkPhysicalDevice> physicalDevices;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSurfaceKHR surface;

    QueueFamilyIndices queueFamilyIndices;
    VkDevice logicalDevice;

    VkSwapchainKHR swapChain;

    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    void getQueueFamilyIndices();
    std::vector<VkDeviceQueueCreateInfo> getDeviceQueueCreateInfos();
    SwapChainSupportDetails querySwapChainSupport();
    VkPhysicalDeviceFeatures getRequiredDeviceFeatures(VkPhysicalDevice &physicalDevice);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

public:
    VulkanApplication();
    ~VulkanApplication();

    GLFWwindow *window;

    void initWindow();
    void createInstance();
    void createLogicalDevice();
    void getPhysicalDevices();
    void createSwapChain();

    void closeVulkan();
};
