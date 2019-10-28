#ifndef TYPESVULKAN_H
#define TYPESVULKAN_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>
#include <ctype.h>
#include <unordered_map>
#include <tuple>

struct CharBitmap
{
    uint16_t width;
    uint16_t height;
    uint8_t * data_view;
    int16_t relative_baseline;
    int16_t horizontal_advance;
    FT_Long glyph_index;
};

struct NormalizedPosition
{
    float x;
    float y;
};

typedef struct RGBA_8UNORM {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA_8UNORM;

struct FontBitmap
{
    std::unordered_map<char, std::tuple<CharBitmap, NormalizedPosition>> char_data;
    uint32_t texture_width;
    uint32_t texture_height;

    uint32_t current_x_cell;
    uint32_t current_y_cell;
    uint16_t cell_size;

    FT_Face face;

    RGBA_8UNORM * bitmap_data;

    static bool instanciate_char_bitmap(FontBitmap& font_bitmap, FT_Face& face, const char c);
};

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

enum class MemoryUsageType { VERTEX_BUFFER = 0, INDICES_BUFFER, SIZE };

struct MemoryUsageMap
{
    uint32_t offset;
    uint32_t size;
};

struct VulkanApplicationPipeline
{
    VkRenderPass renderPass = nullptr;
    VkDescriptorSetLayout descriptorSetLayout = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;
    VkPipeline graphicsPipeline = nullptr;

    VkBuffer vertexBuffer = nullptr;
    VkBuffer indexBuffer = nullptr;

    VkDescriptorPool descriptorPool = nullptr;
    std::vector<VkDescriptorSet> descriptorSets;

    VkImage textureImage = nullptr;
    VkDeviceMemory textureImageMemory = nullptr;
    VkImageView textureImageView = nullptr;
    VkSampler textureSampler = nullptr;

    // Refactor Start
    uint8_t * pipelineMappedMemory;
    uint32_t pipelineMemorySize;
    // TODO: Create something that overloads the [] operator to take a MemoryUsageType as an index value so static_cast is no longer required
    MemoryUsageMap usageMap[static_cast<uint16_t>(MemoryUsageType::SIZE)];
    uint32_t numIndices;
    uint32_t numVertices;

    VkDeviceMemory pipelineMemory;
    UITypeMeshBinding uiComponentsMap[100];

    // Refactor End

    std::vector<VkFramebuffer> swapChainFramebuffers;

//    glm::vec2 * vertexDataStart;
    uint16_t vertexStride;

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
    FontBitmap fontBitmap;

    void setPipeline(VulkanApplicationPipeline&& pipeline, PipelineType tag)
    {
        pipelines[static_cast<size_t>(tag)] = std::move(pipeline);
    }
};

#endif // TYPESVULKAN_H
