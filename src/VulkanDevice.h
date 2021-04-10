#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <set>
#include <vector>

#include <iostream>

#include "VulkanBuffer.h"
#include "VulkanTexture.h"
// #include <memory>
// #include <fstream>
// #include <stdexcept>
// // #include <algorithm>
// // #include <vector>
// #include <cstring>
// #include <cstdlib>
// #include <cstdint>

extern const bool enableValidationLayers;
extern const std::vector<const char *> validationLayers;

const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

const std::vector<const char *> deviceExtensions = {
    //    "VK_KHR_ray_query"
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> computeFamily;

    bool isComplete()
    {
        return computeFamily.has_value();
    }
};

class VulkanDevice
{
public:
    VulkanDevice();
    ~VulkanDevice();
    void init(const VkInstance &vkInstance);
    void addBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkCommandPool &commandPool, size_t &size);
    VkPhysicalDevice &getPhysical();
    VkDevice &getLogical();
    VkQueue &getQueue();
    
    void createCommandPool(VkCommandPool &commandPool);
    void createCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandPool &commandPool);
    void runCommandBuffer(VkCommandBuffer &commandBuffer, VkCommandPool &commandPool, bool end, bool free);
    void destroyCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandPool &commandPool, bool end);

    VulkanBuffer &getBuffer(uint32_t index);
    VulkanTexture &getOutputTexture();
    std::vector<VulkanBuffer> &getBuffers();
    QueueFamilyIndices &getQueueFamilyIndices();

    //    TODO add implicit casting so this class returns logical device, instead of having to call getLogical
private:
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    //    VkPhysicalDeviceFeatures deviceFeatures;
    VkDevice vkDevice = VK_NULL_HANDLE;
    QueueFamilyIndices indices;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkQueue computeQueue;
    std::vector<VulkanBuffer> buffers;
    VulkanTexture outputTexture;
    
    void setImageLayout(VkCommandBuffer cmdbuffer,
                        VkImage image,
                        VkImageAspectFlags aspectMask,
                        VkImageLayout oldImageLayout,
                        VkImageLayout newImageLayout);

    void createBuffer(VkBuffer &buffer, VkBufferUsageFlags usageFlags, uint32_t size);
    void allocateBufferMemory(VkBuffer &buffer, VkDeviceMemory &memory, uint32_t size, uint32_t memoryIndex);

    void findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void pickPhysicalDevice(const VkInstance &vkInstance);
    void createLogicalDevice();

    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);
};
