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
//    "VK_KHR_ray_query"
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
    void init(const VkInstance& vkInstance);
    uint32_t addBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    VkPhysicalDevice& getPhysical();
    VkDevice& getLogical();
    VkQueue& getQueue();
    VulkanBuffer& getBuffer(uint32_t index);
    std::vector<VulkanBuffer>& getBuffers();
    QueueFamilyIndices& getQueueFamilyIndices();
    
//    TODO add implicit casting so this class returns logical device, instead of having to call getLogical
private:
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;
    QueueFamilyIndices indices;
    VkQueue computeQueue;
    std::vector<VulkanBuffer> buffers;

    void findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void pickPhysicalDevice(const VkInstance& vkInstance);
    void createLogicalDevice();
};
