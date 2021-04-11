//
//  VulkanBuffer.h
//  HelloVulkan
//
//  Created by Ilia Chibaev on 27/12/20.
//

#pragma once

#include <vulkan/vulkan.h>
#include <iostream>

class VulkanBuffer
{
private:
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo descriptorBufferInfo;

    VkDevice &device;

public:
    VulkanBuffer(VkDevice &parentDevice);
    void init(VkDeviceSize size, void *data = nullptr);
    virtual ~VulkanBuffer();

    void destroy();

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void *mapped = nullptr;

    VkBuffer &getBuffer();
    VkDeviceMemory &getMemory();
    VkDescriptorBufferInfo &getDescriptor();
};
