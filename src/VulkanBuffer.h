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
    
public:
    VulkanBuffer();
    void init(VkPhysicalDevice physicalDevice, VkDevice device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    virtual ~VulkanBuffer();
    VkBuffer& getBuffer();
    VkDeviceMemory& getMemory();
};
