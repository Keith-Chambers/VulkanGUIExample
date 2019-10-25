#ifndef MAIN_VULKAN_INCLUDE
#define MAIN_VULKAN_INCLUDE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <chrono>
#include <thread>
#include <time.h>

#include "vulkanhelper.h"
#include "initvulkan.h"
#include "typesvulkan.h"
#include "text.h"
#include "config.h"

void recreateSwapChain(VulkanApplication& app);

void drawFrame(VulkanApplication& app);
void mainLoop(VulkanApplication& app);

void createVertexBuffer(    const VkDevice device,
                            const VkPhysicalDevice physicalDevice,
                            const VkQueue graphicsQueue,
                            const VkCommandPool commandPool,
                            uint8_t * vertex_data,
                            const uint32_t vertex_data_size,
                            VkBuffer &outVertexBuffer,
                            VkDeviceMemory& outVertexBufferMemory,
                            void ** outMappedMemory );

VulkanApplication setupApplication();

bool createGenericGraphicsPipeline(const GenericGraphicsPipelineSetup& params, GenericGraphicsPipelineTargets& out, PipelineSetupData& outSetup, bool clearFirst);

bool updateGenericGraphicsPipeline( VkDevice device,
                                    VkExtent2D swapchainExtent,
                                    std::vector<VkImageView>& swapChainImageViews,
                                    uint8_t swapchainSize,
                                    GenericGraphicsPipelineTargets& out, PipelineSetupData &setupCache);


#endif
