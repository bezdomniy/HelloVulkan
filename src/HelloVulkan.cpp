// HelloVulkan.cpp : Defines the entry point for the application.
//

#include "HelloVulkan.h"

GLFWwindow* initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

	return window;
}

VkInstance createInstance() {
	VkResult result = VK_SUCCESS;

	VkInstance instance;

	const VkApplicationInfo applicationInfo
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"HelloVulkanApp",
		1,
		"HelloVulkan",
		1,
		VK_MAKE_VERSION(1,0,0)
	};

	const VkInstanceCreateInfo createInfo
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&applicationInfo,
		0,
		nullptr,
		0,
		nullptr
	};

	result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (result != VK_SUCCESS) throw;

	return instance;
}

std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance& instance)
{
	std::vector<VkPhysicalDevice> physicalDevices;

	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	if (physicalDeviceCount > 0)
	{
		physicalDevices.resize(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
	}

	return physicalDevices;
}

std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice& physicalDevice)
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

VkPhysicalDeviceFeatures getRequiredDeviceFeatures(VkPhysicalDevice& physicalDevice)
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

VkDevice createLogicalDevice(VkPhysicalDevice& physicalDevice)
{
	VkResult result = VK_SUCCESS;
	
	std::vector<VkQueueFamilyProperties> queueFamilyProperties =  getQueueFamilyProperties(physicalDevice);
	VkPhysicalDeviceFeatures requiredFeatures = getRequiredDeviceFeatures(physicalDevice);

	std::vector<VkLayerProperties> layerProperties = getLayerProperties();
	std::vector<VkExtensionProperties> extensionProperties = getExtensionProperties();

	VkDevice logicalDevice;
	const VkDeviceQueueCreateInfo deviceQueueInfo
	{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr,
		0,
		0,
		1,
		nullptr		
	};
	
	const VkDeviceCreateInfo createInfo
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		1,
		&deviceQueueInfo,
		0,
		nullptr,
		0,
		nullptr,
		&requiredFeatures
	};

	result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);

	if (result != VK_SUCCESS) throw;

	return logicalDevice;
}

void closeVulkan(VkInstance& instance, VkDevice& device)
{
	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}

int main() {
	GLFWwindow* window = initWindow();

	VkInstance instance = createInstance();
	std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices(instance);

	std::cout << "Number of physical devices: " << physicalDevices.size() << std::endl;

	
	
	VkDevice device = createLogicalDevice(physicalDevices[0]);
	
	
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	closeVulkan(instance, device);

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}