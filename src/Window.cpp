
#include "Window.h"

Window::Window()
{
}

Window::Window(uint32_t width, uint32_t height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(width, height, "Vulkan RayTracer", nullptr, nullptr);

    // createSurface(instance);
}

Window::~Window() {
    
}

void Window::destroy(const VkInstance &instance)
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

#include <iostream>

void Window::createSurface(const VkInstance &instance)
{
    VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (err) {
        const char* error_msg;
        int ret = glfwGetError(&error_msg);
        if (ret != 0) {
            std::cout << ret << ". Error msg: ";
            if (error_msg != nullptr) std::cout << error_msg;
            std::cout << "\n";
        }
        surface = VK_NULL_HANDLE;
        throw std::runtime_error("failed to create window surface!");
    }
}
