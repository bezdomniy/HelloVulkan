#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Validation.h"

class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance(VulkanInstance&&) = delete;
    VulkanInstance& operator = (const VulkanInstance&) = delete;
    VulkanInstance& operator = (VulkanInstance&&) = delete;

    VkInstance& get();
private:
    VkInstance vkInstance = VK_NULL_HANDLE;
    std::vector<const char*> getRequiredExtensions();
    
};