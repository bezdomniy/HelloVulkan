#pragma once

#include <vulkan/vulkan.h>

class VulkanDevice {
public:
    VulkanDevice();
    virtual ~VulkanDevice();
    VkPhysicalDevice get();
private:
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;

    void pickPhysicalDevice();
    void createLogicalDevice();
};