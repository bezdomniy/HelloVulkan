//
//  VulkanBuffer.h
//  HelloVulkan
//
//  Created by Ilia Chibaev on 27/12/20.
//

#pragma once

#include <vulkan/vulkan.h>

class VulkanBuffer {
private:
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo descriptorBufferInfo;
    
    VkDevice& device;
    VkPhysicalDevice& physicalDevice;
    
public:
    VulkanBuffer(VkDevice& parentDevice, VkPhysicalDevice& parentPhysicalDevice);
    void init(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    virtual ~VulkanBuffer();
    
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    
    void* mapped = nullptr;
    
    VkBuffer& getBuffer();
    VkDeviceMemory& getMemory();
    VkDescriptorBufferInfo& getDescriptor();
};
