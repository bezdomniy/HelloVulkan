#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Validation.h"

extern const bool enableValidationLayers;

class VulkanInstance
{
public:
    VulkanInstance();
    ~VulkanInstance();

    void init();

    //    VulkanInstance(const VulkanInstance&) = delete;
    //    VulkanInstance(VulkanInstance&&) = delete;
    //    VulkanInstance& operator = (const VulkanInstance&) = delete;
    //    VulkanInstance& operator = (VulkanInstance&&) = delete;

    operator VkInstance() const { return vkInstance; };

    //    VkInstance& get();
private:
    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<const char *> getRequiredExtensions();
    void setupDebugMessenger();
};
