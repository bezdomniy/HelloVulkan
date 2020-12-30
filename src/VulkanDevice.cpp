#include "VulkanDevice.h"

VulkanDevice::VulkanDevice() {
    
}

void VulkanDevice::init(const VkInstance& vkInstance) {
    pickPhysicalDevice(vkInstance);
    
    createLogicalDevice();
}

VulkanDevice::~VulkanDevice() {
    vkDestroyDevice(vkDevice, nullptr);
}

void VulkanDevice::findQueueFamilies(VkPhysicalDevice device) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }
}

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {
    findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    return indices.isComplete() && extensionsSupported;
}

void VulkanDevice::pickPhysicalDevice(const VkInstance& vkInstance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            vkPhysicalDevice = device;
            break;
        }
    }

    if (vkPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VulkanDevice::createLogicalDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.computeFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(vkDevice, indices.computeFamily.value(), 0, &computeQueue);
}

VkPhysicalDevice& VulkanDevice::getPhysical() {
    return vkPhysicalDevice;
}

VkDevice& VulkanDevice::getLogical() {
    return vkDevice;
}

VkQueue& VulkanDevice::getQueue() {
    return computeQueue;
}

QueueFamilyIndices& VulkanDevice::getQueueFamilyIndices() {
    return indices;
}

void VulkanDevice::addBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data) {
    buffers.push_back(VulkanBuffer(this->vkDevice, this->vkPhysicalDevice));
    buffers.back().init(usageFlags, memoryPropertyFlags, size, data);

//    return buffers.size() - 1;
}

VulkanBuffer& VulkanDevice::getBuffer(uint32_t index) {
    return buffers.at(index);
}

std::vector<VulkanBuffer>& VulkanDevice::getBuffers() {
    return buffers;
}
