#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <set>
#include <vector>

#include <iostream>
// #include <fstream>
// #include <stdexcept>
// // #include <algorithm>
// // #include <vector>
// #include <cstring>
// #include <cstdlib>
// #include <cstdint>

extern const bool enableValidationLayers;
extern const std::vector<const char*> validationLayers;

const std::vector<const char*> deviceExtensions = {
};

struct QueueFamilyIndices {
    std::optional<uint32_t> computeFamily;

    bool isComplete() {
        return computeFamily.has_value();
    }
};

class VulkanDevice {
public:
    VulkanDevice();
    virtual ~VulkanDevice();
    void init(VkInstance& vkInstance);
    VkPhysicalDevice& getPhysical();
    VkDevice& getLogical();
    VkQueue& getQueue();
    QueueFamilyIndices& getQueueFamilyIndices();
private:
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;
    QueueFamilyIndices indices;
    VkQueue computeQueue;

    void findQueueFamilies();
    bool isDeviceSuitable();
    bool checkDeviceExtensionSupport();
    void pickPhysicalDevice(VkInstance& vkInstance);
    void createLogicalDevice();
};