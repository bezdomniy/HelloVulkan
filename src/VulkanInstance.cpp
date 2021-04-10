#include "VulkanInstance.h"

std::vector<const char*> VulkanInstance::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VulkanInstance::VulkanInstance() {

}

void VulkanInstance::init() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    
    glfwInit();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

//    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
    if (enableValidationLayers) {
//        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//        createInfo.ppEnabledLayerNames = validationLayers.data();
//
//        populateDebugMessengerCreateInfo(debugCreateInfo);
//        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        
        
        debugCreateInfo.flags                              = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
        debugCreateInfo.pfnCallback                        = debugCallback;

        createInfo.pNext = &debugCreateInfo;
        
        VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
        VkValidationFeaturesEXT      features{VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
        features.disabledValidationFeatureCount = 0;
        features.enabledValidationFeatureCount  = 1;
        features.pDisabledValidationFeatures    = nullptr;
        features.pEnabledValidationFeatures     = enabled;

        features.pNext                          = createInfo.pNext;
        createInfo.pNext                     = &features;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

//    setupDebugMessenger();
}

void VulkanInstance::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

VulkanInstance::~VulkanInstance() {
    if (enableValidationLayers) {
//        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
    }

    vkDestroyInstance(vkInstance, nullptr);
}

//VkInstance& VulkanInstance::get() {
//    return vkInstance;
//}
