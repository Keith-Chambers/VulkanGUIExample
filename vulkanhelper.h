#ifndef VULKANHELPER_H
#define VULKANHELPER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <vector>

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
std::vector<char> readFile(const std::string& filename);

void createGenericFrameBuffers(const VkRenderPass& renderPass,
                               const VkDevice& device,
                               const std::vector<VkImageView>& swapChainImageViews,
                               std::vector<VkFramebuffer>& outFrameBuffers,
                               VkExtent2D swapChainExtent);

void createBufferOnMemory(  VkDevice device,
                            VkDeviceSize size,
                            uint32_t memoryOffset,
                            VkBufferUsageFlags usage,
                            VkDeviceMemory& bufferMemory,
                            VkBuffer& outBuffer );

void flushMemoryToDeviceLocal(const VkDevice device,
                              const VkPhysicalDevice physicalDevice,
                              const VkQueue graphicsQueue,
                              const VkCommandPool commandPool,
                              VkBuffer srcBuffer,
                              VkBuffer dstBuffer,
                              VkDeviceMemory& dstBufferMemory,
                              VkDeviceSize bufferSize,
                              VkBufferUsageFlags usage );

void createBuffer(  VkDevice device,
                    VkPhysicalDevice physicalDevice,
                    VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer& buffer,
                    VkDeviceMemory& bufferMemory);

void copyBuffer(    VkDevice device,
                    VkCommandPool commandPool,
                    VkQueue graphicsQueue,
                    VkBuffer srcBuffer,
                    VkBuffer dstBuffer,
                    VkDeviceSize size);

void createImage(   VkDevice device,
                    VkPhysicalDevice physicalDevice,
                    uint32_t width,
                    uint32_t height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkImage& image,
                    VkDeviceMemory& imageMemory);

void allocateMemory( VkDevice device,
                     VkDeviceSize size,
                     uint32_t memoryTypeIndex,
                     VkDeviceMemory& bufferMemory);

uint32_t findMemoryType(    VkPhysicalDevice physicalDevice,
                            uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

void createTextureImage(    VkDevice device,
                            VkPhysicalDevice physicalDevice,
                            VkCommandPool commandPool,
                            VkQueue graphicsQueue,
                            uint8_t * texture_data,
                            uint32_t texture_width,
                            uint32_t texture_height,
                            VkImage& outTextureImage,
                            VkDeviceMemory& outTextureImageMemory);

void transitionImageLayout( VkDevice device,
                            VkCommandPool commandPool,
                            VkQueue graphicsQueue,
                            VkImage image,
                            VkImageLayout oldLayout,
                            VkImageLayout newLayout);

VkCommandBuffer beginSingleTimeCommands(    VkDevice device,
                                            VkCommandPool commandPool );

void endSingleTimeCommands( VkDevice device,
                            VkCommandPool commandPool,
                            VkQueue graphicsQueue,
                            VkCommandBuffer commandBuffer);

void createImageView(VkDevice device, VkImage image, VkFormat format, VkImageView& outTextureImageView);

#endif
