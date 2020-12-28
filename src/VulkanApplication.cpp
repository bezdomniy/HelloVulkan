#include "VulkanApplication.h"

void VulkanApplication::initVulkan() {
    instance.init();
    device.init(instance);
    
//    Input buffer
//    device.addBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, inBufferSize);
    
//    Output buffer
    device.addBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, outBufferSize);
    
//    createDescriptorSetLayout();
//    createDescriptorSet();
//    createPipeline();
    
    std::vector<VkDescriptorType> types = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    
    pipeline.init(device.getBuffers(),types,"../../src/shaders/comp.spv");
        
    createCommandPool();
    createCommandBuffers();
}

void VulkanApplication::mainLoop() {
//    updateUniformBuffers();
    runCommandBuffer();
    vkDeviceWaitIdle(device.getLogical());

    saveRenderedImage();
}


void VulkanApplication::cleanup() {
    vkDestroyCommandPool(device.getLogical(), commandPool, nullptr);
}

//void VulkanApplication::recreatePipeline() {
//    vkDeviceWaitIdle(device.getLogical());
//
//    createPipeline();
//    createCommandBuffers();
//}
//
//void VulkanApplication::createPipeline() {
//    auto compShaderCode = readFile("../../src/shaders/comp.spv");
//
//    VkShaderModule compShaderModule = createShaderModule(compShaderCode);
//
//    VkPipelineShaderStageCreateInfo compShaderStageInfo{};
//    compShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    compShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
//    compShaderStageInfo.module = compShaderModule;
//    compShaderStageInfo.pName = "main";
//
//    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
//    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutCreateInfo.setLayoutCount = 1;
//    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
//
//    if (vkCreatePipelineLayout(device.getLogical(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create pipeline layout!");
//    }
//
//    VkComputePipelineCreateInfo pipelineCreateInfo = {};
//    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
//    pipelineCreateInfo.stage = compShaderStageInfo;
//    pipelineCreateInfo.layout = pipelineLayout;
//
//    vkCreateComputePipelines(
//            device.getLogical(), VK_NULL_HANDLE,
//            1, &pipelineCreateInfo,
//            NULL, &pipeline);
//
//    vkDestroyShaderModule(device.getLogical(), compShaderModule, nullptr);
//}


void VulkanApplication::createCommandPool() {
    // QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

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

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getPipelineLayout(), 0, 1, &pipeline.getDescriptorSet(), 0, NULL);

    vkCmdDispatch(commandBuffer, (uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

//VkShaderModule VulkanApplication::createShaderModule(const std::vector<char>& code) {
//    VkShaderModuleCreateInfo createInfo{};
//    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//    createInfo.codeSize = code.size();
//    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
//
//    VkShaderModule shaderModule;
//    if (vkCreateShaderModule(device.getLogical(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create shader module!");
//    }
//
//    return shaderModule;
//}
//
//void VulkanApplication::createDescriptorSetLayout() {
//    /*
//    Here we specify a descriptor set layout. This allows us to bind our descriptors to
//    resources in the shader.
//    */
//
//    /*
//    Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to the binding point
//    0. This binds to
//        layout(std140, binding = 0) buffer buf
//    in the compute shader.
//    */
//    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
//    descriptorSetLayoutBinding.binding = 0; // binding = 0
//    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//    descriptorSetLayoutBinding.descriptorCount = 1;
//    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
//
//    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
//    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    descriptorSetLayoutCreateInfo.bindingCount = 1; // only a single binding in this descriptor set layout.
//    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
//
//    // Create the descriptor set layout.
//    vkCreateDescriptorSetLayout(device.getLogical(), &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout);
//}
//
//void VulkanApplication::createDescriptorSet() {
//    /*
//    So we will allocate a descriptor set here.
//    But we need to first create a descriptor pool to do that.
//    */
//
//    /*
//    Our descriptor pool can only allocate a single storage buffer.
//    */
//    VkDescriptorPoolSize descriptorPoolSize = {};
//    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//    descriptorPoolSize.descriptorCount = 1;
//
//    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
//    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//    descriptorPoolCreateInfo.maxSets = 1; // we only need to allocate one descriptor set from the pool.
//    descriptorPoolCreateInfo.poolSizeCount = 1;
//    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
//
//    // create descriptor pool.
//    vkCreateDescriptorPool(device.getLogical(), &descriptorPoolCreateInfo, NULL, &descriptorPool);
//
//    /*
//    With the pool allocated, we can now allocate the descriptor set.
//    */
//    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
//    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
//    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
//    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
//
//    // allocate descriptor set.
//    vkAllocateDescriptorSets(device.getLogical(), &descriptorSetAllocateInfo, &descriptorSet);
//
//    /*
//    Next, we need to connect our actual storage buffer with the descrptor.
//    We use vkUpdateDescriptorSets() to update the descriptor set.
//    */
//
//    // Specify the buffer to bind to the descriptor.
//    VkDescriptorBufferInfo descriptorBufferInfo = {};
//    descriptorBufferInfo.buffer = device.getBuffer(0).getBuffer();
//    descriptorBufferInfo.offset = 0;
//    descriptorBufferInfo.range = outBufferSize;
//
//    VkWriteDescriptorSet writeDescriptorSet = {};
//    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    writeDescriptorSet.dstSet = descriptorSet; // write to this descriptor set.
//    writeDescriptorSet.dstBinding = 0; // write to the first, and only binding.
//    writeDescriptorSet.descriptorCount = 1; // update a single descriptor.
//    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
//    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
//
//    // perform the update of the descriptor set.
//    vkUpdateDescriptorSets(device.getLogical(), 1, &writeDescriptorSet, 0, NULL);
//}

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
    vkCreateFence(device.getLogical(), &fenceCreateInfo, NULL, &fence);

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

    vkDestroyFence(device.getLogical(), fence, NULL);
}

std::vector<char> VulkanApplication::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void VulkanApplication::saveRenderedImage() {
    void* mappedMemory = NULL;
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
    ubo.lightPos.x = 0.0f + sin(glm::radians(timer * 360.0f)) * cos(glm::radians(timer * 360.0f)) * 2.0f;
    ubo.lightPos.y = 0.0f + sin(glm::radians(timer * 360.0f)) * 2.0f;
    ubo.lightPos.z = 0.0f + cos(glm::radians(timer * 360.0f)) * 2.0f;
    ubo.camera.pos = glm::vec3(0.0f, 0.0f, -4.0f) * -1.0f;
//    compute.uniformBuffer.map();
//    memcpy(uniformBuffer.mapped, &ubo, sizeof(ubo));
//    compute.uniformBuffer.unmap();
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

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
