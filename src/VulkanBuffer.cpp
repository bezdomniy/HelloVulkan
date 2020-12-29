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
 
VulkanBuffer::VulkanBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice) : device(device), physicalDevice(physicalDevice) {
}

VulkanBuffer::~VulkanBuffer() {

}

void VulkanBuffer::init(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data) {

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);

    // Create the memory backing up the buffer handle
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
    VkMemoryRequirements memReqs;

    VkMemoryAllocateInfo memAlloc {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkGetBufferMemoryRequirements(device, buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    bool memTypeFound = false;
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & 1) == 1) {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
                memAlloc.memoryTypeIndex = i;
                memTypeFound = true;
            }
        }
        memReqs.memoryTypeBits >>= 1;
    }
    assert(memTypeFound);
    vkAllocateMemory(device, &memAlloc, nullptr, &memory);

    if (data != nullptr) {
        void *mapped;
        vkMapMemory(device, memory, 0, size, 0, &mapped);
        memcpy(mapped, data, size);
        vkUnmapMemory(device, memory);
    }

    vkBindBufferMemory(device, buffer, memory, 0);
    
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

VkBuffer& VulkanBuffer::getBuffer() {
    return buffer;
}

VkDeviceMemory& VulkanBuffer::getMemory() {
    return memory;
}

VkDescriptorBufferInfo& VulkanBuffer::getDescriptor() {
    return descriptorBufferInfo;
}
