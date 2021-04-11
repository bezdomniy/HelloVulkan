#include "VulkanApplication.h"

void VulkanApplication::initVulkan()
{
    // instance.init();

    window = Window(WIDTH, HEIGHT);
    device.init(window);
    device.createCommandPool(commandPool);

    //    Output buffer
    // device.addBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, outBufferSize);
    device.createImage(WIDTH, HEIGHT, VK_FORMAT_R32G32B32A32_SFLOAT, commandPool, this->outBufferSize);

    // Uniform buffer //TODO: make the offset for this start at 1, not 0
    device.addBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBufferSize);

    createShapes();

    VkCommandBuffer copyCmd;
    VkBufferCopy copyRegion = {};

    addSSBOBuffer(shapes.data(), shapesBufferSize, copyCmd, copyRegion);
    addSSBOBuffer(mesh, meshBufferSize, copyCmd, copyRegion);

    addSSBOBuffer(bvh, bvhBufferSize, copyCmd, copyRegion);
    addSSBOBuffer(blas.data(), blasBufferSize, copyCmd, copyRegion);

    std::vector<VkDescriptorType> bufferTypes = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    pipeline.init(device.getBuffers(), bufferTypes, device.getOutputTexture().descriptor, "../../../src/shaders/comp.spv");

    device.createCommandBuffer(commandBuffer, commandPool);
    finaliseMainCommandBuffer();
}

void VulkanApplication::mainLoop()
{
    updateUniformBuffers();
    device.runCommandBuffer(commandBuffer, commandPool, false, false);
    vkDeviceWaitIdle(device.getLogical());

    saveRenderedImage();
}

void VulkanApplication::cleanup()
{
    if (mesh)
    {
        //        free(bvh->nodes);
        //        free(bvh);
        delete[] mesh;
    }

    for (auto &buf : device.getBuffers())
    {
        buf.destroy();
    }
    device.getOutputTexture().destroy(device.getLogical());
    device.destroyCommandBuffer(commandBuffer, commandPool, false);
    vkDestroyCommandPool(device.getLogical(), commandPool, nullptr);

    pipeline.destroy();
    device.destroy(window);
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

void VulkanApplication::saveRenderedImage()
{
    void *mappedMemory = nullptr;

    VkImageSubresource subres = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        0};
    VkSubresourceLayout layout;
    vkGetImageSubresourceLayout(device.getLogical(), device.getOutputTexture().image, &subres, &layout);

    // Map the buffer memory, so that we can read from it on the CPU.
    //    vkMapMemory(device.getLogical(), device.getBuffer(0).getMemory(), 0, outBufferSize, 0, &mappedMemory);
    vkMapMemory(device.getLogical(), device.getOutputTexture().deviceMemory, layout.offset, VK_WHOLE_SIZE, 0, &mappedMemory);
    //    glm::vec4 *pmappedMemory = (glm::vec4 *)mappedMemory;

    glm::vec4 *pmappedMemory = new glm::vec4[WIDTH * HEIGHT];
    memcpy(pmappedMemory, mappedMemory, outBufferSize);
    // Done reading, so unmap.
    vkUnmapMemory(device.getLogical(), device.getOutputTexture().deviceMemory);

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

    delete[] pmappedMemory;

    // Now we save the acquired color data to a .png.
    //unsigned error = lodepng::encode("mandelbrot.png", image, WIDTH, HEIGHT);
    //if (error)
    //    printf("encoder error %d: %s", error, lodepng_error_text(error));

    ImageWriter::writeToPPM("mandelbrot.ppm", image, WIDTH, HEIGHT);
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

    ubo.camera = Primitives::makeCamera(glm::vec4(1.f, 3.f, -5.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 0.f), WIDTH, HEIGHT, 1.0472f);

    auto &uniformBuffer = device.getBuffer(0);
    uniformBuffer.map();
    memcpy(uniformBuffer.mapped, &ubo, sizeof(ubo));
    uniformBuffer.unmap();
}

void VulkanApplication::createShapes()
{
    Primitives::Material mat{glm::vec4(0.537f, 0.831f, 0.914f, 1.f), 0.1f, 0.7f, 0.3f, 200};
    Primitives::Material mat2{glm::vec4(0.637f, 0.231f, 0.114f, 1.f), 0.1f, 0.7f, 0.3f, 200};

    //    glm::mat4 t(1.0f);
    //        glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(1.2,1.2,1.2));
    //    glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.6, 0.6, 0.6));
    glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.003, 0.003, 0.003));
    // glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.02, 0.02, 0.02));
    glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(-0.5f, 0.f, 0.5f));
    glm::mat4 sT = translate * scale;

    //    glm::mat4 sT(1.0);
    //    Primitives::Shape s = Primitives::makeSphere(mat, sT);
    mesh = Primitives::makeMesh("../../../assets/models/cube.obj", mat, sT, meshBufferSize);

    std::tie(bvh, blas) = Primitives::makeBVH("../../../assets/models/lucy.obj", mat, sT, bvhBufferSize);
    blasBufferSize = blas.size() * sizeof(Primitives::NodeBLAS);

    //    bvhBufferSize += 16;

    glm::mat4 pT(1.0);
    Primitives::Shape p = Primitives::makePlane(mat2, pT);

    //    shapes.push_back(s);
    shapes.push_back(p);
    shapesBufferSize = sizeof(Primitives::Shape) * shapes.size();
    //    bvhBufferSize = sizeof(*bvh) + 16; //TODO is this plus 16, and why is size so low
}

void VulkanApplication::addSSBOBuffer(void *buffer, size_t bufferSize, VkCommandBuffer &copyCmd, VkBufferCopy copyRegion)
{
    device.addBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize);

    device.addBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, buffer);

    device.createCommandBuffer(copyCmd, commandPool);
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(copyCmd, device.getBuffer(device.getBuffers().size() - 1).getBuffer(), device.getBuffer(device.getBuffers().size() - 2).getBuffer(), 1, &copyRegion);
    device.runCommandBuffer(copyCmd, commandPool, true, true);

    //    TODO: change to .back()
    device.getBuffer(device.getBuffers().size() - 1).destroy();
    device.getBuffers().pop_back();
}

// #include <chrono>
// using namespace std::chrono;

void VulkanApplication::run()
{
    // auto start = high_resolution_clock::now();
    initVulkan();
    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<seconds>(stop - start);
    // std::cout << duration.count() << std::endl;

    // auto start = high_resolution_clock::now();
    mainLoop();
    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<microseconds>(stop - start);
    // std::cout << duration.count() << std::endl;

    // start = high_resolution_clock::now();
    cleanup();

    // stop = high_resolution_clock::now();
    // duration = duration_cast<microseconds>(stop - start);
    // std::cout << duration.count() << std::endl;
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

    //    app.outBufferSize = sizeof(glm::vec4) * WIDTH * HEIGHT;
    app.uniformBufferSize = sizeof(UBOCompute);
    //    app.uniformBufferSize = 0;

    //    try
    //    {
    app.run();
    //    }
    //    catch (const std::exception &e)
    //    {
    //        std::cerr << "### " << e.what() << std::endl;
    //        return EXIT_FAILURE;
    //    }

    return EXIT_SUCCESS;
}
