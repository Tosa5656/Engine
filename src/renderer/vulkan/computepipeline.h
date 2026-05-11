#pragma once

#include <vector>
#include <string>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

class ComputePipeline
{
public:
    ComputePipeline();
    ~ComputePipeline();

    void Create(Device* device, const std::string& compShaderPath, const std::vector<VkDescriptorSetLayout>& setLayouts);
    void Shutdown(Device* device);

    VkPipeline GetPipeline();
    VkPipelineLayout GetPipelineLayout();

private:
    VkShaderModule CreateShaderModule(const std::vector<char>& code, Device* device);
    std::vector<char> ReadFile(const std::string& filename);

    VkPipeline m_computePipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};
