#ifndef TYPESVULKAN_H
#define TYPESVULKAN_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>

typedef struct RGBA_8UNORM {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA_8UNORM;

typedef struct Glyph
{
    uint16_t height;
    uint16_t width;
    RGBA_8UNORM * data;
} Glyph;

struct BasicVertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(BasicVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

        attributeDescriptions.resize(2); // TODO: Needs to be manually changed with below code.

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(BasicVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(BasicVertex, color);

        return attributeDescriptions;
    }
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

        attributeDescriptions.resize(3); // TODO: Needs to be manually changed with below code.

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
}; 

struct GenericGraphicsPipelineSetup
{
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    VkDevice device;
    VkFormat swapChainImageFormat;
    VkVertexInputBindingDescription vertexBindingDescription;
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
    VkExtent2D swapChainExtent;
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings; // ?
    uint8_t swapChainSize;
    std::vector<VkImageView> swapChainImageViews;
};

struct GenericGraphicsPipelineTargets
{
    VkPipeline * graphicsPipeline;
    VkPipelineLayout * pipelineLayout;
    VkRenderPass * renderPass;
    VkDescriptorSetLayout * descriptorSetLayout;
    std::vector<VkFramebuffer> * swapChainFramebuffers;
};

struct PipelineSetupData {

    VkAttachmentDescription attachmentDescription;
    VkAttachmentReference attachmentReference;
    VkSubpassDependency subpassDependency;

    VkPipelineShaderStageCreateInfo shaderStages[2];
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlending;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

    VkVertexInputBindingDescription vertexBindingDescription;
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
};

enum class UITYPE { BUTTON, TEXT };

struct UITypeMeshBinding
{

    UITYPE type;
    uint32_t indexStartIndex;
    uint32_t vertexStartIndex;
    uint16_t vertexLength;
    uint16_t indexLength;
};

struct VulkanApplicationPipeline
{
    VkRenderPass renderPass = nullptr;
    VkDescriptorSetLayout descriptorSetLayout = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;
    VkPipeline graphicsPipeline = nullptr;

    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;        // Remove
    VkBuffer indexBuffer = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;         // Remove

    VkDescriptorPool descriptorPool = nullptr;
    std::vector<VkDescriptorSet> descriptorSets;

    VkImage textureImage = nullptr;
    VkDeviceMemory textureImageMemory = nullptr;
    VkImageView textureImageView = nullptr;
    VkSampler textureSampler = nullptr;

    // Refactor Start

    uint8_t * mappedVertexData;
    uint8_t * mappedIndexData;
    VkDeviceMemory pipelineMemory;
    UITypeMeshBinding uiComponentsMap[100];

    // Refactor End

    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<uint16_t> drawIndices;

    glm::vec2 * vertexDataStart;
    uint32_t vertexDataStride;
    uint32_t numVertices;

    PipelineSetupData setupCache;
};

enum PipelineType {
    Texture = 0,
    PrimativeShapes,
    SIZE
};

struct VulkanApplication
{
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkDescriptorPool descriptorPool;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;

    std::array<PipelineType, static_cast<uint8_t>(PipelineType::SIZE)> pipelineDrawOrder;
    std::array<VulkanApplicationPipeline, static_cast<size_t>(PipelineType::SIZE)> pipelines;

    void setPipeline(VulkanApplicationPipeline&& pipeline, PipelineType tag)
    {
        pipelines[static_cast<size_t>(tag)] = std::move(pipeline);
    }
};

#endif // TYPESVULKAN_H
