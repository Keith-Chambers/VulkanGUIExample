#ifndef INIT_VULKAN_INCLUDE
#define INIT_VULKAN_INCLUDE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <optional>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <set>

#include "typesvulkan.h"

const int WIDTH = 800;
const int HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const bool enableValidationLayers = true;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void cleanupSwapChain(VulkanApplication& app, std::vector<VulkanApplicationPipeline *> pipelines);
void cleanupSwapChain(VulkanApplication& app);
void cleanup(VulkanApplication& app);

SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
void initializeVulkan(VulkanApplication& app);
bool checkValidationLayerSupport();
void createInstance(VkInstance * instance);
std::vector<const char*> getRequiredExtensions();
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
void setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);

VkResult CreateDebugUtilsMessengerEXT(  VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator,
                                        VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT& debugMessenger, const VkAllocationCallbacks* pAllocator);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

void createSwapChain(   VkPhysicalDevice physicalDevice,
                        VkDevice device,
                        const VkSurfaceKHR surface,
                        VkSwapchainKHR& swapChain,
                        std::vector<VkImage>& swapChainImages,
                        VkFormat& swapChainImageFormat,
                        VkExtent2D& swapChainExtent,
                        GLFWwindow * window );


bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice& device);

void createSurface(VkInstance instance, GLFWwindow * window, VkSurfaceKHR * surface);
void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice& physicalDevice, VkSurfaceKHR surface);
void createLogicalDevice(const VkPhysicalDevice physicalDevice, VkDevice * device, VkQueue& graphicsQueue, VkQueue& presentQueue, const VkSurfaceKHR surface);

void initWindow(GLFWwindow ** window);

#endif
