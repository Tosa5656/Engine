#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstring>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class Instance
{
public:
    Instance();
    ~Instance();

    void Create(const VkApplicationInfo &appInfo);

    VkInstance GetInstance();
    VkDebugUtilsMessengerEXT GetDebugMessenger();
    const std::vector<const char*> GetValidationLayers();
    bool IsExtensionValidationEnabled();
    std::vector<const char*> GetRequiredExtensions();
    bool CheckValidationLayersSupport();

    void SetupDebugMessenger();
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    static VKAPI_ATTR VkBool32 DebugReportCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
private:

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    const bool m_enableValidationLayers = false;
#else
    const bool m_enableValidationLayers = true;
#endif
};