# CMakeList.txt : CMake project for HelloVulkan, include source and define
# project specific logic here.
#
# cmake_minimum_required (VERSION 3.8)

# set (CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(ENV{VK_ICD_FILENAMES} "/Users/iliachibaev/dev/tools/vulkansdk/macOS/share/vulkan/icd.d/MoltenVK_icd.json")
# set(VK_LAYER_PATH "/Users/iliachibaev/dev/tools/vulkansdk/macOS/share/vulkan/explicit_layer.d")

# set(CMAKE_CXX_FLAGS "-target x86_64-apple-macos11.1")
# set(CMAKE_OSX_ARCHITECTURES "x86_64")
# set(CMAKE_IGNORE_PATH "/opt/homebrew/")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
# Add source to this project's executable.

file(GLOB SOURCES
    *.h
    *.cpp
)

add_executable (HelloVulkan ${SOURCES})



find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

# find_package(Vulkan REQUIRED EXACT REQUIRED PATHS )

message("Vulkan found? " ${VULKAN_FOUND})
message("Alright, where is it? " ${Vulkan_LIBRARY})
message("And I can include it? " ${Vulkan_INCLUDE_DIR})
message("${Vulkan}")

target_include_directories(HelloVulkan PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(HelloVulkan PRIVATE glfw ${GLM_LIBRARY} ${Vulkan_LIBRARY} vk-bootstrap)