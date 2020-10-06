# CMakeList.txt : CMake project for HelloVulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source to this project's executable.
add_executable (HelloVulkan "VulkanApplication.cpp" "VulkanApplication.h")

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

message("Vulkan found? " ${VULKAN_FOUND})
message("Alright, where is it? " ${Vulkan_LIBRARY})
message("And I can include it? " ${Vulkan_INCLUDE_DIR})
message("${Vulkan}")

target_include_directories(HelloVulkan PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(HelloVulkan PRIVATE glfw ${GLM_LIBRARY} ${Vulkan_LIBRARY})