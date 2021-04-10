#include <vulkan/vulkan.h>

class VulkanTexture
{
public:
    // vks::VulkanDevice *device;
    VkImage image;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
     VkImageView view;
    // uint32_t width, height;
    // uint32_t mipLevels;
    // uint32_t layerCount;
     VkDescriptorImageInfo descriptor;
     VkSampler sampler;
};
