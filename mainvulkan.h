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
#include "input.h"

void recreateSwapChain(VulkanApplication& app);

void drawFrame(VulkanApplication& app);
void mainLoop(VulkanApplication& app);

bool removeArrayIndex(uint16_t *array, uint16_t arraySize, uint16_t arrayIndex);

void loopLogic(VulkanApplication& app, std::chrono::milliseconds delta);
void loadInitialMeshData(VulkanApplication& app, uint32_t delta);

int16_t doublePercentageToInt16(double value);
float int16PercentageToFloat(int16_t value);
double int16PercentageToDouble(int16_t value);

static void handleOperation(VulkanApplication& app, uint16_t operationIndex);

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
