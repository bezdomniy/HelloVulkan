#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <fstream>
#include <iostream>

#include "VulkanBuffer.h"

class VulkanPipeline {
private:
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkShaderModule shaderModule;
    VkPipelineCache pipelineCache;
    
    void createDescriptorPool();
    void createDescriptorSet(std::vector<VulkanBuffer>& buffers, std::vector<VkDescriptorType>& types, const VkDescriptorImageInfo& imageDescriptorInfo);
    void createDescriptorSetLayout(std::vector<VkDescriptorType>& types);
    void createPipelineLayout();
    void createPipeline();
    void createShader(const std::string& shaderPath);
    void createPipelineCache();
    
    VkDevice& device;
    
public:
    VulkanPipeline(VkDevice& parentDevice);
    ~VulkanPipeline();
    void destroy();
    operator VkPipeline() const { return pipeline; };
    void init(std::vector<VulkanBuffer>& buffers, std::vector<VkDescriptorType>& types, const VkDescriptorImageInfo& imageDescriptorInfo, const std::string& shaderPath);
    
    const VkPipelineLayout& getPipelineLayout();
    const VkDescriptorSet& getDescriptorSet();
};
