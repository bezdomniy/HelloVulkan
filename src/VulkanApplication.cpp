#include "VulkanApplication.h"

void VulkanApplication::initVulkan() {
    instance.init();
    device.init(instance);
    
    
    
//    Output buffer
    device.addBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, outBufferSize);
    
    // Uniform buffer
    device.addBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBufferSize);
    
    createShapes();
    device.addBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     shapesBufferSize, shapes.data());
    
    std::vector<VkDescriptorType> bufferTypes = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    
    pipeline.init(device.getBuffers(),bufferTypes,"../../src/shaders/comp.spv");
        
    createCommandPool();
    createCommandBuffers();
}

void VulkanApplication::mainLoop() {
    updateUniformBuffers();
    runCommandBuffer();
    vkDeviceWaitIdle(device.getLogical());

    saveRenderedImage();
}


void VulkanApplication::cleanup() {
    vkDestroyCommandPool(device.getLogical(), commandPool, nullptr);
}


void VulkanApplication::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = device.getQueueFamilyIndices().computeFamily.value();

    if (vkCreateCommandPool(device.getLogical(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanApplication::createCommandBuffers() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device.getLogical(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getPipelineLayout(), 0, 1, &pipeline.getDescriptorSet(), 0, NULL);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdDispatch(commandBuffer, (uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanApplication::runCommandBuffer() {
    /*
    Now we shall finally submit the recorded command buffer to a queue.
    */

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // submit a single command buffer
    submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.

    /*
        We create a fence.
    */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    vkCreateFence(device.getLogical(), &fenceCreateInfo, nullptr, &fence);

    /*
    We submit the command buffer on the queue, at the same time giving a fence.
    */
    vkQueueSubmit(device.getQueue(), 1, &submitInfo, fence);
    /*
    The command will not have finished executing until the fence is signalled.
    So we wait here.
    We will directly after this read our buffer from the GPU,
    and we will not be sure that the command has finished executing unless we wait for the fence.
    Hence, we use a fence here.
    */
    vkWaitForFences(device.getLogical(), 1, &fence, VK_TRUE, 100000000000);

    vkDestroyFence(device.getLogical(), fence, nullptr);
}


void VulkanApplication::saveRenderedImage() {
    void* mappedMemory = nullptr;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device.getLogical(), device.getBuffer(0).getMemory(), 0, outBufferSize, 0, &mappedMemory);
    glm::vec4* pmappedMemory = (glm::vec4 *)mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<unsigned char> image;
    image.reserve(WIDTH * HEIGHT * 4);
    for (int i = 0; i < WIDTH*HEIGHT; i += 1) {
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].r)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].g)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].b)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].a)));
    }
    // Done reading, so unmap.
    vkUnmapMemory(device.getLogical(), device.getBuffer(0).getMemory());

    // Now we save the acquired color data to a .png.
    unsigned error = lodepng::encode("mandelbrot.png", image, WIDTH, HEIGHT);
    if (error) printf("encoder error %d: %s", error, lodepng_error_text(error));
}

void VulkanApplication::updateUniformBuffers()
{
    float timer = 0.f;
//    ubo.lightPos.x = 0.0f + sin(glm::radians(timer * 360.0f)) * cos(glm::radians(timer * 360.0f)) * 2.0f;
//    ubo.lightPos.y = 0.0f + sin(glm::radians(timer * 360.0f)) * 2.0f;
//    ubo.lightPos.z = 0.0f + cos(glm::radians(timer * 360.0f)) * 2.0f;
    
    ubo.lightPos.x = -10.0f ;
    ubo.lightPos.y = 10.0f ;
    ubo.lightPos.z = 0.0f ;
    
    ubo.camera = Primitives::makeCamera(glm::vec4(0.f, 1.5f, -5.f, 1.f), glm::vec4(0.f,1.f,0.f,1.f), glm::vec4(0.f,1.f,0.f,0.f), 800, 600, 1.0472);
    
    
    auto& uniformBuffer = device.getBuffer(1);
    uniformBuffer.map();
    memcpy(uniformBuffer.mapped, &ubo, sizeof(ubo));
    uniformBuffer.unmap();
}

void VulkanApplication::createShapes() {
    Primitives::Material mat {glm::vec3(0.0f,0.0f, 0.5f)};
    
//    glm::mat4 t(1.0f);
    glm::mat4 t =
              glm::translate(glm::mat4(1.0), glm::vec3(-0.5f,1.f,0.5f));

    
    Primitives::Shape s = Primitives::makeSphere(mat, t);
    
    shapes.push_back(s);
    shapesBufferSize = sizeof(Primitives::Shape) * shapes.size();
}

void VulkanApplication::run() {
    initVulkan();
    mainLoop();
    cleanup();
}

VulkanApplication::VulkanApplication() : pipeline(device.getLogical()) {
}

VulkanApplication::~VulkanApplication() {
    
}

int main() {
    VulkanApplication app;

    app.outBufferSize = sizeof(glm::vec4) * WIDTH * HEIGHT;
    app.uniformBufferSize = sizeof(UBOCompute);
//    app.uniformBufferSize = 0;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "### " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
