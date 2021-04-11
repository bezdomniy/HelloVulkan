#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <set>
#include <vector>

#include <iostream>

#include "VkBootstrap.h"

#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "Window.h"

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
    void init(Window &window);
    void destroy(Window &window);
    void addBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkCommandPool &commandPool, size_t &size);
    VkInstance &getInstance();
//    VkPhysicalDevice &getPhysical();
    VkDevice &getLogical();
    VkQueue &getQueue();

    void createCommandPool(VkCommandPool &commandPool);
    void createCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandPool &commandPool);
    void runCommandBuffer(VkCommandBuffer &commandBuffer, VkCommandPool &commandPool, bool end, bool free);
    void destroyCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandPool &commandPool, bool end);

    VulkanBuffer &getBuffer(uint32_t index);
    VulkanTexture &getOutputTexture();
    std::vector<VulkanBuffer> &getBuffers();
//    QueueFamilyIndices &getQueueFamilyIndices();

    //    TODO add implicit casting so this class returns logical device, instead of having to call getLogical
private:
    vkb::Instance vkbInstance;
//    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    //    VkPhysicalDeviceFeatures deviceFeatures;
    vkb::Device vkbDevice;
//    QueueFamilyIndices indices;
    std::vector<vkb::CustomQueueDescription> queueDescriptions;
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

    void findQueueFamilies();

    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);
};
