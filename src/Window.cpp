
#include "Window.h"

Window::Window() {
    
}

Window::Window(const VkInstance &instance, uint32_t width, uint32_t height) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(width, height, "Vulkan RayTracer", nullptr, nullptr);
    
    createSurface(instance);
}

Window::~Window() {   
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::createSurface(const VkInstance &instance) {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
