#include "VulkanApplication.h"
// #include <optional>

VulkanApplication::VulkanApplication()
{
	initWindow();
	createInstance();
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	getPhysicalDevices();
	std::cout << "Number of physical devices: " << physicalDevices.size() << std::endl;

	createLogicalDevice();

	vkGetDeviceQueue(logicalDevice, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);

	createSwapChain();
}

VulkanApplication::~VulkanApplication()
{
	closeVulkan();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void VulkanApplication::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);
}

void VulkanApplication::createInstance()
{
	VkResult result = VK_SUCCESS;

	// std::vector<const char *> instance_layers;
	std::vector<const char *> instance_extensions;
	// instance_layers.push_back("VK_LAYER_KHRONOS_validation");
	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	uint32_t instance_extension_count = 0;
	const char **instance_extensions_buffer = glfwGetRequiredInstanceExtensions(&instance_extension_count);
	for (uint32_t i = 0; i < instance_extension_count; ++i)
	{
		// Push back required instance extensions as well
		instance_extensions.push_back(instance_extensions_buffer[i]);
	}

	const VkApplicationInfo applicationInfo{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"HelloVulkanApp",
		1,
		"HelloVulkan",
		1,
		VK_MAKE_VERSION(1, 0, 0)};

	const VkInstanceCreateInfo createInfo{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&applicationInfo,
		(uint32_t)validationLayers.size(),
		validationLayers.data(),
		(uint32_t)instance_extensions.size(),
		instance_extensions.data()};

	result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (result != VK_SUCCESS)
		throw;
}

void VulkanApplication::getPhysicalDevices()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	if (physicalDeviceCount > 0)
	{
		physicalDevices.resize(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
	}
}

std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice &physicalDevice)
{
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;

	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

	if (queueFamilyPropertyCount > 0)
	{
		queueFamilyProperties.resize(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
	}

	return queueFamilyProperties;
}

std::vector<VkLayerProperties> getLayerProperties()
{
	std::vector<VkLayerProperties> instanceLayerProperties;

	uint32_t numInstanceLayers = 0;
	vkEnumerateInstanceLayerProperties(&numInstanceLayers, nullptr);

	if (numInstanceLayers > 0)
	{
		instanceLayerProperties.resize(numInstanceLayers);
		vkEnumerateInstanceLayerProperties(&numInstanceLayers, instanceLayerProperties.data());
	}

	return instanceLayerProperties;
}

std::vector<VkExtensionProperties> getExtensionProperties()
{
	std::vector<VkExtensionProperties> instanceExtensionProperties;

	uint32_t numInstanceExtensions = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, nullptr);

	if (numInstanceExtensions > 0)
	{
		instanceExtensionProperties.resize(numInstanceExtensions);
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, instanceExtensionProperties.data());
	}

	return instanceExtensionProperties;
}

VkPhysicalDeviceFeatures VulkanApplication::getRequiredDeviceFeatures(VkPhysicalDevice &physicalDevice)
{
	VkPhysicalDeviceFeatures supportedFeatures;
	VkPhysicalDeviceFeatures requiredFeatures = {};

	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	// Set whatever features you need here
	requiredFeatures.multiDrawIndirect = supportedFeatures.multiDrawIndirect;
	requiredFeatures.tessellationShader = VK_TRUE;
	requiredFeatures.geometryShader = VK_TRUE;

	return requiredFeatures;
}

void VulkanApplication::createLogicalDevice()
{
	VkResult result = VK_SUCCESS;

	getQueueFamilyIndices();
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = getDeviceQueueCreateInfos();

	VkPhysicalDeviceFeatures requiredFeatures = getRequiredDeviceFeatures(physicalDevices[0]);

	std::vector<VkLayerProperties> layerProperties = getLayerProperties();
	std::vector<VkExtensionProperties> extensionProperties = getExtensionProperties();

	const VkDeviceCreateInfo createInfo{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(queueCreateInfos.size()),
		queueCreateInfos.data(),
		(uint32_t)validationLayers.size(),
		validationLayers.data(),
		static_cast<uint32_t>(deviceExtensions.size()),
		deviceExtensions.data(),
		&requiredFeatures};

	result = vkCreateDevice(physicalDevices[0], &createInfo, nullptr, &logicalDevice);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device.");
}

void VulkanApplication::closeVulkan()
{
	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDeviceWaitIdle(logicalDevice);
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanApplication::getQueueFamilyIndices()
{
	std::vector<VkQueueFamilyProperties> queueFamilyProperties = getQueueFamilyProperties(physicalDevices[0]);

	VkBool32 presentSupport = false;

	uint32_t i = 0;
	for (const auto &queueFamily : queueFamilyProperties)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], i, surface, &presentSupport);

		if (presentSupport)
		{
			queueFamilyIndices.presentFamily = i;
		}

		if (queueFamilyIndices.isComplete())
		{
			break;
		}

		i++;
	}
}

std::vector<VkDeviceQueueCreateInfo> VulkanApplication::getDeviceQueueCreateInfos()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	return queueCreateInfos;
}

SwapChainSupportDetails VulkanApplication::querySwapChainSupport()
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[0], surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[0], surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[0], surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[0], surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[0], surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = {WIDTH, HEIGHT};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VulkanApplication::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t indices[] = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};

	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

int main()
{
	VulkanApplication app;

	while (!glfwWindowShouldClose(app.window))
	{
		glfwPollEvents();
	}

	return 0;
}