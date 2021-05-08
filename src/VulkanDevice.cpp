#include "VulkanDevice.h"

VulkanDevice::VulkanDevice()
{
}

void VulkanDevice::init(Window &window)
{
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("Vulkan Ray Tracer")
                        .request_validation_layers()
                        .enable_extension("VK_KHR_get_physical_device_properties2")
                        .use_default_debug_messenger()
                        .build();

    if (!inst_ret)
    {
        throw std::runtime_error("Failed to create Vulkan instance. Error: " + inst_ret.error().message());
    }
    vkbInstance = inst_ret.value();

    window.createSurface(vkbInstance.instance);

    vkb::PhysicalDeviceSelector selector{vkbInstance};
    selector.add_required_extension("VK_KHR_portability_subset");
    auto phys_ret = selector.set_surface(window.surface)
                        .set_minimum_version(1, 1) // require a vulkan 1.1 capable device
                                                   //                        .require_dedicated_transfer_queue()
                        .select();
    if (!phys_ret)
    {
        throw std::runtime_error("Failed to select Vulkan Physical Device. Error: " + phys_ret.error().message());
    }

    vkGetPhysicalDeviceMemoryProperties(phys_ret.value().physical_device, &this->memoryProperties);

    vkb::DeviceBuilder device_builder{phys_ret.value()};
    // automatically propagate needed data from instance & physical device
    auto dev_ret = device_builder.build();
    if (!dev_ret)
    {
        throw std::runtime_error("Could not find a matching memory type" + dev_ret.error().message());
    }
    vkbDevice = dev_ret.value();

    findQueueFamilies();

    buffers.reserve(5);
}

void VulkanDevice::destroy(Window &window) {
    vkDestroyDevice(getLogical(), nullptr);

    window.destroy(getInstance());
    vkb::destroy_debug_utils_messenger(getInstance(), vkbInstance.debug_messenger);
    vkDestroyInstance(getInstance(), nullptr);
}

uint32_t VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < this->memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((this->memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }

    throw std::runtime_error("Could not find a matching memory type");
}

VulkanDevice::~VulkanDevice()
{
    //    vkDestroyDevice(vkDevice, nullptr);
}

void VulkanDevice::findQueueFamilies()
{
    for (uint32_t i = 0; i < static_cast<uint32_t>(vkbDevice.queue_families.size()); i++)
    {
        if (vkbDevice.queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            // Find the first queue family with graphics operations supported
            queueDescriptions.push_back(vkb::CustomQueueDescription(
                i, vkbDevice.queue_families[i].queueCount,
                std::vector<float>(vkbDevice.queue_families[i].queueCount, 1.0f)));

            break;
        }
    }

    if (queueDescriptions.empty())
    {
        throw std::runtime_error("failed to find compute queue!");
    }

    vkGetDeviceQueue(vkbDevice.device, queueDescriptions[0].index, 0, &computeQueue);
}

VkInstance &VulkanDevice::getInstance()
{
    return vkbInstance.instance;
}

VkDevice &VulkanDevice::getLogical()
{
    return vkbDevice.device;
}

VkQueue &VulkanDevice::getQueue()
{
    return computeQueue;
}

void VulkanDevice::addBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data)
{
    buffers.emplace_back(vkbDevice.device);

    createBuffer(buffers.back().getBuffer(), usageFlags, size);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(vkbDevice.device, buffers.back().getBuffer(), &memReqs);

    uint32_t memoryIndex = this->getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);

    allocateBufferMemory(buffers.back().getBuffer(), buffers.back().getMemory(), memReqs.size, memoryIndex);

    buffers.back().init(size, data);

    //    return buffers.size() - 1;
}

void VulkanDevice::createImage(uint32_t width, uint32_t height, VkFormat format, VkCommandPool &commandPool, size_t &size)
{
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = {width, height, 1};
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Image will be sampled in the fragment shader and used as storage target in the compute shader
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.flags = 0;

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    VkMemoryRequirements memReqs;

    vkCreateImage(vkbDevice.device, &imageCreateInfo, nullptr, &this->outputTexture.image);
    vkGetImageMemoryRequirements(vkbDevice.device, this->outputTexture.image, &memReqs);

    size = memReqs.size;

    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = this->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); //TODO: figure out optimal combo of these flags
    vkAllocateMemory(vkbDevice.device, &memAllocInfo, nullptr, &this->outputTexture.deviceMemory);
    vkBindImageMemory(vkbDevice.device, this->outputTexture.image, this->outputTexture.deviceMemory, 0);

    VkCommandBuffer layoutCmd;
    createCommandBuffer(layoutCmd, commandPool);

    this->outputTexture.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    setImageLayout(
        layoutCmd,
        this->outputTexture.image,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        this->outputTexture.imageLayout);

    runCommandBuffer(layoutCmd, commandPool, true, true);

    // Create sampler
    VkSamplerCreateInfo sampler{};
    sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.maxAnisotropy = 1.0f;
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler.addressModeV = sampler.addressModeU;
    sampler.addressModeW = sampler.addressModeU;
    sampler.mipLodBias = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    sampler.maxLod = 0.0f;
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vkCreateSampler(vkbDevice.device, &sampler, nullptr, &this->outputTexture.sampler);

    // Create image view
    VkImageViewCreateInfo view{};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    view.image = this->outputTexture.image;
    vkCreateImageView(vkbDevice.device, &view, nullptr, &this->outputTexture.view);

    this->outputTexture.descriptor.imageView = this->outputTexture.view;
    this->outputTexture.descriptor.sampler = this->outputTexture.sampler; // Works without this one and the next one, why?
    this->outputTexture.descriptor.imageLayout = this->outputTexture.imageLayout;
}

VulkanBuffer &VulkanDevice::getBuffer(uint32_t index)
{
    return buffers.at(index);
}

std::vector<VulkanBuffer> &VulkanDevice::getBuffers()
{
    return buffers;
}

VulkanTexture &VulkanDevice::getOutputTexture()
{
    return outputTexture;
}

void VulkanDevice::createBuffer(VkBuffer &buffer, VkBufferUsageFlags usageFlags, uint32_t size)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(vkbDevice.device, &bufferCreateInfo, nullptr, &buffer);
}

void VulkanDevice::allocateBufferMemory(VkBuffer &buffer, VkDeviceMemory &memory, uint32_t size, uint32_t memoryIndex)
{
    // Create the memory backing up the buffer handle

    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    memAlloc.allocationSize = size;
    memAlloc.memoryTypeIndex = memoryIndex;

    if (vkAllocateMemory(vkbDevice.device, &memAlloc, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
}

void VulkanDevice::createCommandPool(VkCommandPool &commandPool)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueDescriptions[0].index;

    if (vkCreateCommandPool(getLogical(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanDevice::createCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandPool &commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(getLogical(), &allocInfo, &cmdBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void VulkanDevice::runCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandPool &commandPool, bool end, bool free)
{
    if (cmdBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    if (end)
    {
        vkEndCommandBuffer(cmdBuffer);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    VkFence fence;

    if (vkCreateFence(getLogical(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create fence!");
    }
    // Submit to the queue
    if (vkQueueSubmit(getQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer to queue!");
    }
    // Wait for the fence to signal that command buffer has finished executing
    if (vkWaitForFences(getLogical(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to wait for fence!");
    }
    vkDestroyFence(getLogical(), fence, nullptr);
    if (free)
    {
        vkFreeCommandBuffers(getLogical(), commandPool, 1, &cmdBuffer);
    }
}

void VulkanDevice::destroyCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandPool &commandPool, bool end)
{
    if (end)
    {
        vkEndCommandBuffer(cmdBuffer);
    }

    vkFreeCommandBuffers(getLogical(), commandPool, 1, &cmdBuffer);
}

void VulkanDevice::setImageLayout(VkCommandBuffer cmdbuffer,
                                  VkImage image,
                                  VkImageAspectFlags aspectMask,
                                  VkImageLayout oldImageLayout,
                                  VkImageLayout newImageLayout)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        imageMemoryBarrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (imageMemoryBarrier.srcAccessMask == 0)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
        cmdbuffer,
         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
         VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}
