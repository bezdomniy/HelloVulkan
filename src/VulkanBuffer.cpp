//
//  VulkanBuffer.cpp
//  HelloVulkan
//
//  Created by Ilia Chibaev on 27/12/20.
//

#include "VulkanBuffer.h"
// #include <stdio.h>
#include <cstring>
#include <cassert>

VulkanBuffer::VulkanBuffer(VkDevice &device) : device(device)
{
}

VulkanBuffer::~VulkanBuffer()
{
    //    destroy();
}

void VulkanBuffer::destroy()
{
    if (buffer)
    {
        vkDestroyBuffer(device, buffer, nullptr);
    }
    if (memory)
    {
        vkFreeMemory(device, memory, nullptr);
    }
}

void VulkanBuffer::init(VkDeviceSize size, void *data)
{
    if (data != nullptr)
    {
        //        void *mapped;
        if (vkMapMemory(device, memory, 0, size, 0, &mapped) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to map buffer memory!");
        }
        memcpy(mapped, data, size);
        vkUnmapMemory(device, memory);
    }

    if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to bind buffer memory!");
    }

    descriptorBufferInfo = {};
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = size;
}

VkResult VulkanBuffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    return vkMapMemory(device, memory, offset, size, 0, &mapped);
}

void VulkanBuffer::unmap()
{
    if (mapped)
    {
        vkUnmapMemory(device, memory);
        mapped = nullptr;
    }
}

VkBuffer &VulkanBuffer::getBuffer()
{
    return buffer;
}

VkDeviceMemory &VulkanBuffer::getMemory()
{
    return memory;
}

VkDescriptorBufferInfo &VulkanBuffer::getDescriptor()
{
    return descriptorBufferInfo;
}
