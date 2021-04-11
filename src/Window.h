#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>

class Window
{
public:
    Window();
    Window(uint32_t width, uint32_t height);
    ~Window();
    
    void destroy(const VkInstance &instance);

    void createSurface(const VkInstance &instance);

    VkSurfaceKHR surface;

private:
    GLFWwindow *window;
};
