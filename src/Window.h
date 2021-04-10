#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>

class Window {
public:
    Window();
    Window(const VkInstance &instance, uint32_t width, uint32_t height);
    ~Window();
    
    VkSurfaceKHR surface;
private:
    GLFWwindow* window;
    void createSurface(const VkInstance &instance);
};
