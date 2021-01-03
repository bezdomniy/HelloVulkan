
#include "VulkanPipeline.h"

VulkanPipeline::VulkanPipeline(VkDevice& device) : device(device) {
}

VulkanPipeline::~VulkanPipeline() {

}

void VulkanPipeline::destroy() {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyShaderModule(device, shaderModule, nullptr);
    vkDestroyPipelineCache(device, pipelineCache, nullptr);
    
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

void VulkanPipeline::init(std::vector<VulkanBuffer>& buffers, std::vector<VkDescriptorType>& types, const std::string& shaderPath) {
    createDescriptorPool();
    createDescriptorSetLayout(types);
    createPipelineLayout();
    createDescriptorSet(buffers, types);
    createShader(shaderPath);
    createPipelineCache();
    createPipeline();
}

void VulkanPipeline::createDescriptorPool() {
    VkDescriptorPoolSize uniformDescriptorPoolSize = {};
    uniformDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformDescriptorPoolSize.descriptorCount = 1;
    
    VkDescriptorPoolSize imageDescriptorPoolSize = {}; // For output image
    imageDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    imageDescriptorPoolSize.descriptorCount = 1;
    
    VkDescriptorPoolSize primitivesDescriptorPoolSize = {}; // For buffers of shape primitives
    primitivesDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    primitivesDescriptorPoolSize.descriptorCount = 2; // Number of primitive buffers here
    
    std::vector<VkDescriptorPoolSize> poolSizes = {uniformDescriptorPoolSize,imageDescriptorPoolSize,primitivesDescriptorPoolSize};

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 4;
    descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    // create descriptor pool.
    if (vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanPipeline::createDescriptorSet(std::vector<VulkanBuffer>& buffers, std::vector<VkDescriptorType>& types) {
    /*
    With the pool allocated, we can now allocate the descriptor set.
    */
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

    // allocate descriptor set.
    if(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets;
    computeWriteDescriptorSets.reserve(buffers.size());
    
    for (int i = 0; i < buffers.size(); i++) {
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet; // write to this descriptor set.
        writeDescriptorSet.dstBinding = i; // write to the first, and only binding.
        writeDescriptorSet.descriptorCount = 1; // update a single descriptor.
        writeDescriptorSet.descriptorType = types.at(i); // storage buffer.
        writeDescriptorSet.pBufferInfo = &buffers.at(i).getDescriptor();
        
        computeWriteDescriptorSets.push_back(writeDescriptorSet);
    }

    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, nullptr);
}

void VulkanPipeline::createDescriptorSetLayout(std::vector<VkDescriptorType>& types) {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    setLayoutBindings.reserve(types.size());
    
    for (int i = 0; i < types.size(); i++) {
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
        descriptorSetLayoutBinding.descriptorType = types.at(i);
        descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        descriptorSetLayoutBinding.binding = i; // binding = 0
        descriptorSetLayoutBinding.descriptorCount = 1;
        
        setLayoutBindings.push_back(descriptorSetLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = setLayoutBindings.size();
    descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();

    // Create the descriptor set layout.
    if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanPipeline::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    
    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void VulkanPipeline::createPipeline() {
    VkPipelineShaderStageCreateInfo compShaderStageInfo{};
    compShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    compShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compShaderStageInfo.module = shaderModule;
    compShaderStageInfo.pName = "main";
    
    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = compShaderStageInfo;
    pipelineCreateInfo.layout = pipelineLayout;

    if (vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCreateInfo,nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline!");
    };
}


void VulkanPipeline::createShader(const std::string& shaderPath) {
    std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
}

void VulkanPipeline::createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
}


const VkPipelineLayout& VulkanPipeline::getPipelineLayout() {
    return pipelineLayout;
}

const VkDescriptorSet& VulkanPipeline::getDescriptorSet() {
    return descriptorSet;
}
