#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <set>
#include <vector>

#include <iostream>

#include "VulkanBuffer.h"
// #include <memory>
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
    ~VulkanDevice();
    void init(VkInstance& vkInstance);
    uint32_t addBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    VkPhysicalDevice& getPhysical();
    VkDevice& getLogical();
    VkQueue& getQueue();
    VulkanBuffer& getBuffer(uint32_t index);
    QueueFamilyIndices& getQueueFamilyIndices();
private:
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;
    QueueFamilyIndices indices;
    VkQueue computeQueue;
    std::vector<VulkanBuffer> buffers;

    void findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void pickPhysicalDevice(VkInstance& vkInstance);
    void createLogicalDevice();
};
