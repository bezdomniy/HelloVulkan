#include "VulkanApplication.h"

void VulkanApplication::initVulkan()
{
    instance.init();
    device.init(instance);
    createCommandPool();

    //    Output buffer
    device.addBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, outBufferSize);

    // Uniform buffer
    device.addBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBufferSize);

    createShapes();
    
    VkCommandBuffer copyCmd;
    VkBufferCopy copyRegion = {};
    
    addSSBOBuffer(shapes.data(), shapesBufferSize, copyCmd, copyRegion);
    addSSBOBuffer(mesh, meshBufferSize, copyCmd, copyRegion);
    
    addSSBOBuffer(bvh, bvhBufferSize, copyCmd, copyRegion);
    addSSBOBuffer(blas.data(), blasBufferSize, copyCmd, copyRegion);

    std::vector<VkDescriptorType> bufferTypes = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    pipeline.init(device.getBuffers(), bufferTypes, "../../src/shaders/comp.spv");

    createCommandBuffer(commandBuffer);
    finaliseMainCommandBuffer();
}

void VulkanApplication::mainLoop()
{
    updateUniformBuffers();
    runCommandBuffer(commandBuffer, false, false);
    vkDeviceWaitIdle(device.getLogical());

    saveRenderedImage();
}

void VulkanApplication::cleanup()
{
    if (mesh)
    {
        //        free(bvh->nodes);
//        free(bvh);
        delete [] mesh;
    }

    for (auto &buf : device.getBuffers())
    {
        buf.destroy();
    }
    destroyCommandBuffer(commandBuffer, false);
    vkDestroyCommandPool(device.getLogical(), commandPool, nullptr);

    pipeline.destroy();
    vkDestroyDevice(device.getLogical(), nullptr);
    
}

void VulkanApplication::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = device.getQueueFamilyIndices().computeFamily.value();

    if (vkCreateCommandPool(device.getLogical(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanApplication::createCommandBuffer(VkCommandBuffer &cmdBuffer)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device.getLogical(), &allocInfo, &cmdBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void VulkanApplication::destroyCommandBuffer(VkCommandBuffer &cmdBuffer, bool end)
{
    if (end)
    {
        vkEndCommandBuffer(cmdBuffer);
    }

    vkFreeCommandBuffers(device.getLogical(), commandPool, 1, &cmdBuffer);
}

void VulkanApplication::finaliseMainCommandBuffer()
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getPipelineLayout(), 0, 1, &pipeline.getDescriptorSet(), 0, NULL);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdDispatch(commandBuffer, (uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanApplication::runCommandBuffer(VkCommandBuffer cmdBuffer, bool end, bool free)
{
    if (cmdBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    if (end)
    {
        vkEndCommandBuffer(cmdBuffer);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    VkFence fence;

    if (vkCreateFence(device.getLogical(), &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create fence!");
    }
    // Submit to the queue
    if (vkQueueSubmit(device.getQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer to queue!");
    }
    // Wait for the fence to signal that command buffer has finished executing
    if (vkWaitForFences(device.getLogical(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to wait for fence!");
    }
    vkDestroyFence(device.getLogical(), fence, nullptr);
    if (free)
    {
        vkFreeCommandBuffers(device.getLogical(), commandPool, 1, &cmdBuffer);
    }
}

void VulkanApplication::saveRenderedImage()
{
    void *mappedMemory = nullptr;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device.getLogical(), device.getBuffer(0).getMemory(), 0, outBufferSize, 0, &mappedMemory);
    glm::vec4 *pmappedMemory = (glm::vec4 *)mappedMemory;

    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<unsigned char> image;
    image.reserve(WIDTH * HEIGHT * 4);
    for (int i = 0; i < WIDTH * HEIGHT; i += 1)
    {
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].r)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].g)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].b)));
        image.push_back((unsigned char)(255.0f * (pmappedMemory[i].a)));
    }
    // Done reading, so unmap.
    vkUnmapMemory(device.getLogical(), device.getBuffer(0).getMemory());

    // Now we save the acquired color data to a .png.
    unsigned error = lodepng::encode("mandelbrot.png", image, WIDTH, HEIGHT);
    if (error)
        printf("encoder error %d: %s", error, lodepng_error_text(error));
}

void VulkanApplication::updateUniformBuffers()
{
    float timer = 0.f;
    //    ubo.lightPos.x = 0.0f + sin(glm::radians(timer * 360.0f)) * cos(glm::radians(timer * 360.0f)) * 2.0f;
    //    ubo.lightPos.y = 0.0f + sin(glm::radians(timer * 360.0f)) * 2.0f;
    //    ubo.lightPos.z = 0.0f + cos(glm::radians(timer * 360.0f)) * 2.0f;

    ubo.lightPos.x = 10.0f;
    ubo.lightPos.y = 10.0f;
    ubo.lightPos.z = -10.0f;
    ubo.lightPos.w = 1.0f; //TODO check if this is right - should it be 0?

    ubo.camera = Primitives::makeCamera(glm::vec4(0.f, 1.5f, -5.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 0.f), 800, 600, 1.0472);

    auto &uniformBuffer = device.getBuffer(1);
    uniformBuffer.map();
    memcpy(uniformBuffer.mapped, &ubo, sizeof(ubo));
    uniformBuffer.unmap();
}

void VulkanApplication::createShapes()
{
    Primitives::Material mat{glm::vec4(0.537f, 0.831f, 0.914f, 1.f), 0.1f, 0.7f, 0.3f, 200};

    //    glm::mat4 t(1.0f);
//        glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(1.2,1.2,1.2));
    glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.6, 0.6, 0.6));
//        glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.003,0.003,0.003));
    glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(-0.5f, 1.f, 0.5f));
    glm::mat4 sT = translate * scale;
    
//    glm::mat4 sT(1.0);
    //    Primitives::Shape s = Primitives::makeSphere(mat, sT);
//    mesh = Primitives::makeMesh("../../assets/models/cube.obj", mat, sT, meshBufferSize);
    
    std::tie(bvh,blas) = Primitives::makeBVH("../../assets/models/cube.obj", mat, sT, bvhBufferSize);
    blasBufferSize = blas.size() * sizeof(Primitives::NodeBLAS);

    //    bvhBufferSize += 16;

    glm::mat4 pT(1.0);
    Primitives::Shape p = Primitives::makePlane(mat, pT);

    //    shapes.push_back(s);
    shapes.push_back(p);
    shapesBufferSize = sizeof(Primitives::Shape) * shapes.size();
    //    bvhBufferSize = sizeof(*bvh) + 16; //TODO is this plus 16, and why is size so low
}

void VulkanApplication::addSSBOBuffer(void* buffer, size_t bufferSize, VkCommandBuffer copyCmd, VkBufferCopy copyRegion) {
    device.addBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize);

    device.addBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, buffer);

    createCommandBuffer(copyCmd);
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(copyCmd, device.getBuffer(device.getBuffers().size()-1).getBuffer(), device.getBuffer(device.getBuffers().size()-2).getBuffer(), 1, &copyRegion);
    runCommandBuffer(copyCmd, true, true);

    device.getBuffer(device.getBuffers().size()-1).destroy();
    device.getBuffers().pop_back();
}

void VulkanApplication::run()
{
    initVulkan();
    mainLoop();
    cleanup();
}

VulkanApplication::VulkanApplication() : pipeline(device.getLogical())
{
}

VulkanApplication::~VulkanApplication()
{
}

int main()
{
    VulkanApplication app;

    app.outBufferSize = sizeof(glm::vec4) * WIDTH * HEIGHT;
    app.uniformBufferSize = sizeof(UBOCompute);
    //    app.uniformBufferSize = 0;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "### " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
