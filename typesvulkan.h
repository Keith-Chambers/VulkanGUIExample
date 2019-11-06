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

#include "entity.h"

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

struct RGBA_8UNORM
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

};

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

struct Point
{
    uint16_t x;
    uint16_t y;
};

struct Rect
{
    Point point;
    uint16_t height;
    uint16_t width;
};

struct Arc
{
    uint16_t radius;
    Point point;
    uint8_t startAngle; // Even
    uint8_t endAngle;   // Even // Until you can replace with Zig types
};

union AreaShape
{
    Rect r;
    Arc a;
};

struct Area
{
    uint8_t rects;
    uint8_t flags; // Can be used to indicate onHover, onMouseClick, etc

    AreaShape subAreas[1];
};

#define BOUNDS_FLAGS_ACTIVE_TRIGGER 0b00000001
#define OPERATION_CODE_FLAGS_INLINE 0b10000000

//#define MOUSE_EVENT_TYPE_HOVER              0b00000001
//#define MOUSE_EVENT_TYPE_LCLICK             0b00000010
//#define MOUSE_EVENT_TYPE_RCLICK             0b00000100
//#define MOUSE_EVENT_TYPE_MCLICK             0b00001000
//#define MOUSE_EVENT_TYPE_SCROLL             0b00010000
//#define MOUSE_EVENT_TYPE_INLINE_OP4_FIRST   0b00100000
//#define MOUSE_EVENT_TYPE_INLINE_OP4_SECOND  0b01000000
//#define MOUSE_EVENT_TYPE_SPECIAL_BIT        0b10000000

//#define MOUSE_BOUNDS_FLAGS_TARGET_ON_HOVER_ENTER    0b00000000
//#define MOUSE_BOUNDS_FLAGS_TARGET_ON_HOVER_EXIT     0b10000000
//#define MOUSE_BOUNDS_FLAGS_TARGET_ON_LCLICK         0b01000000
//#define MOUSE_BOUNDS_FLAGS_TARGET_ON_RCLICK         0b11000000

#define MOUSE_BOUNDS_FLAGS_DOUBLE_OP1               0b00000000 00000001
#define MOUSE_BOUNDS_FLAGS_DOUBLE_OP2               0b00000000 00000010
#define MOUSE_BOUNDS_FLAGS_DOUBLE_OP3               0b00000000 00000100
#define MOUSE_BOUNDS_FLAGS_DOUBLE_OP4               0b00000000 00001000
#define MOUSE_BOUNDS_FLAGS_DOUBLE_OP5               0b00000000 00010000

#define MOUSE_BOUNDS_FLAGS_OP1_CHAIN_INDEX          0b00000000 00010000
#define MOUSE_BOUNDS_FLAGS_OP2_CHAIN_INDEX          0b00000000 00010000
#define MOUSE_BOUNDS_FLAGS_OP3_CHAIN_INDEX          0b00000000 00010000
#define MOUSE_BOUNDS_FLAGS_OP4_CHAIN_INDEX          0b00000000 00010000
#define MOUSE_BOUNDS_FLAGS_OP5_CHAIN_INDEX          0b00000000 00010000

//#define MOUSE_BOUNDS_FLAGS_OVERFLOW_SUB_AREAS       0b00000000 00000000

// If the sum of these flags is greater than 5, then sub areas if overflowed implicitly
#define MOUSE_BOUNDS_FLAGS_1_ON_HOVER_ENTER         0b0000000000000000
#define MOUSE_BOUNDS_FLAGS_2_ON_HOVER_ENTER         0b1000000000000000
#define MOUSE_BOUNDS_FLAGS_3_ON_HOVER_ENTER         0b0100000000000000
#define MOUSE_BOUNDS_FLAGS_0_ON_HOVER_ENTER         0b1100000000000000

#define MOUSE_BOUNDS_FLAGS_1_ON_HOVER_EXIT          0b0000000000000000
#define MOUSE_BOUNDS_FLAGS_2_ON_HOVER_EXIT          0b0010000000000000
#define MOUSE_BOUNDS_FLAGS_3_ON_HOVER_EXIT          0b0001000000000000
#define MOUSE_BOUNDS_FLAGS_0_ON_HOVER_EXIT          0b0011000000000000

#define MOUSE_BOUNDS_FLAGS_1_ON_LCLICK              0b0000000000000000
#define MOUSE_BOUNDS_FLAGS_2_ON_LCLICK              0b0000100000000000
#define MOUSE_BOUNDS_FLAGS_3_ON_LCLICK              0b0000010000000000
#define MOUSE_BOUNDS_FLAGS_0_ON_LCLICK              0b0000110000000000

#define MOUSE_BOUNDS_FLAGS_NUM_ON_HOVER_ENTER_MASK      0b1100000000000000
#define MOUSE_BOUNDS_FLAGS_NUM_ON_HOVER_EXIT_MASK       0b0011000000000000

//#define MOUSE_BOUNDS_FLAGS_1_ON_RCLICK              0b00000000 00000000
//#define MOUSE_BOUNDS_FLAGS_2_ON_RCLICK              0b00000010 00000000
//#define MOUSE_BOUNDS_FLAGS_3_ON_RCLICK              0b00000001 00000000
//#define MOUSE_BOUNDS_FLAGS_0_ON_RCLICK              0b00000011 00000000

//#define MOUSE_BOUNDS_FLAGS_1_ON_MCLICK              0b00000000 00000000
//#define MOUSE_BOUNDS_FLAGS_2_ON_MCLICK              0b00000000 00000010
//#define MOUSE_BOUNDS_FLAGS_3_ON_MCLICK              0b00000000 00000001
//#define MOUSE_BOUNDS_FLAGS_0_ON_MCLICK              0b00000000 00000011

struct MouseBounds
{
    Rect boundsArea;
    uint16_t flags;
    uint8_t onHoverEnterOperation;
    uint8_t onHoverExitOperation;
    uint8_t onLClickOperation;
    uint8_t onRClickOperation;
    uint8_t onMClickOperation;
    uint8_t numSubAreas;   // 0 to just use boundArea
};

// TODO: Implement operation chains
struct OperationChainBounds
{
    uint8_t startIndex;
    uint8_t length; // Can use a bit of this to double startIndex
};

// TODO: Implement key - operation bindings
struct KeyPressOperation
{
    uint8_t key;
    uint8_t operationIndex;
    uint16_t flags;
};

// Naming is hard.
struct OnMouseEventOpBindings
{
    uint16_t numBounds;
    uint16_t numAreas;
    MouseBounds bounds[10];
    AreaShape complexAreas[1];
};

struct ChangeColorOperation
{
    Entity16 targetEntity;
    uint8_t r;  // Better to put all colours to be used somewhere and provide an index
    uint8_t g;  // You can also pre-define colour transitions
    uint8_t b;  // Or use functions to define something more complex
    uint8_t a;
};

struct RelativeMoveOperation
{
    Entity16 targetEntity;
    int16_t addX;
    int16_t addY;
};

struct AnimatedMoveOperation
{
    Entity16 targetEntity;
    uint16_t durationMs;
    uint16_t currentMs; // You can use diffX, currentX (Actual vertex) and % of animation to calucate original position
    int16_t differenceX;
    int16_t differenceY;
    uint8_t moveAnimationFunctionIndex;
    uint8_t scaleAnimationFunctionIndex;
    uint16_t scaleBy; // 1.125 = 1125, 30 = 30000. max is x65, 3 decimals of precision
};

static_assert(sizeof(AnimatedMoveOperation) == 14);

struct ShowWidgetOperation
{
    Entity16 target;
    uint16_t xPos;
    uint16_t yPos;
};

struct DuplicateEntityOperation
{
    Entity16 toCopy;
    uint16_t xPos;
    uint16_t yPos;
};

struct ActiveMoveDrag
{
    uint16_t prevX;
    uint16_t prevY;
    uint16_t currX;
    uint16_t currY;
    Entity16 uiEntity;

    uint8_t padding[4];
};

struct ActiveSelectionDrag
{
    uint16_t beginX;
    uint16_t beginY;
    uint16_t currX;
    uint16_t currY;
    Entity16 dragSelectionUI;

    uint8_t padding[4];
};

union Operation16Union
{
    AnimatedMoveOperation animatedMove;
};

typedef uint8_t Operation;

#define OPERATION_FLAGS_PER_FRAME   0b00000001
#define OPERATION_FLAGS_ACTIVE      0b00000010
#define OPERATION_FLAGS_INVERT      0b00000100

#define OPERATION_FLAGS_FIRST       0b00000000
#define OPERATION_FLAGS_SECOND      0b00100000
#define OPERATION_FLAGS_THIRD       0b00010000
#define OPERATION_FLAGS_FORTH       0b00110000

// These reuse the first two bits of the premise that they are invalid options for..
// ..operations of size 8 and 16.
#define OPERATION_FLAGS_FIFTH       0b01010000
#define OPERATION_FLAGS_SIXTH       0b01110000
#define OPERATION_FLAGS_SEVENTH     0b11010000
#define OPERATION_FLAGS_EIGHTH      0b11110000

#define OPERATION_FLAGS_SIZE_2      0b00000000
#define OPERATION_FLAGS_SIZE_4      0b10000000
#define OPERATION_FLAGS_SIZE_8      0b01000000
#define OPERATION_FLAGS_SIZE_16     0b11000000


#define OPERATION_CODE_RELATIVE_MOVE 1
#define OPERATION_CODE_ABSOLUTE_MOVE 2

// Different sizes should be able to reuse Operation Codes
#define OPERATION_CODE_DEACTIVATE_OP            3
#define OPERATION_CODE_APPLY_MOVE_TO_BOUNDS     4

struct Operation2
{
    uint8_t opCode;
    uint8_t flags;
};

struct SimpleMouseBoundsMoveOperation
{
    uint16_t boundsIndex;
    int16_t addX;
    int16_t addY;
};

union Operation4Union
{
    uint16_t operationIndex;
};

struct Operation4
{
    uint8_t opCode;
    uint8_t flags;
    Operation4Union opData;
};

static_assert(sizeof(Operation4) == 4);

union Operation8Union
{
    ShowWidgetOperation showWidget;
    RelativeMoveOperation relativeMove;
    ChangeColorOperation changeColor;
    SimpleMouseBoundsMoveOperation  mouseBoundsMove;
};

static_assert(sizeof(Operation8Union) == 6);

struct Operation8
{
    uint8_t opCode;
    uint8_t flags;
    Operation8Union opData;
};

struct Operation16
{
    uint8_t opCode;
    uint8_t flags;
    Operation16Union opData;
};

static_assert(sizeof(ChangeColorOperation) == 6);
static_assert(sizeof(RelativeMoveOperation) == 6);
static_assert(sizeof(DuplicateEntityOperation) == 6);
static_assert(sizeof(ShowWidgetOperation) == 6);

static_assert(sizeof(ActiveMoveDrag) == 14);
static_assert(sizeof(ActiveSelectionDrag) == 14);
static_assert(sizeof(ActiveMoveDrag) == 14);

// TODO: Remove this at some point as not being used
namespace operation
{
    inline Operation * next(Operation * operation) {
        // Move past opCode so pointer points to flags
        operation++;

        if( (*operation) & OPERATION_FLAGS_SIZE_16 ) {
            return operation + 15; // 16 - 1 (For the increment in operation above)
        }
        else if ((*operation) & OPERATION_FLAGS_SIZE_8)
        {
            return operation + 7;
        } else { /* Operation2 */
            assert((*operation) ^ OPERATION_FLAGS_SIZE_16 && "Invalid opcode encountered in 'next' function");
            return operation + 1;
        }
    }

    namespace get {

        inline uint16_t operationIndex(Operation2& operation)
        {
            assert( operation.opCode & OPERATION_FLAGS_SIZE_4  && "Invalid operation size for 'operationIndex'");
            return *reinterpret_cast<uint16_t*>( reinterpret_cast<uint8_t *>(&operation) + 2);
        }

        inline RelativeMoveOperation relativeMove(Operation2& operation)
        {
            assert( operation.opCode & OPERATION_FLAGS_SIZE_8  && "Invalid operation size for 'relativeMove'");
            return *reinterpret_cast<RelativeMoveOperation*>((&operation) + 1);
        }

    }
}

// TODO: Remove
struct OperationIndexOffsets
{
    uint16_t onHoverEnterOffset;
    uint16_t onHoverExitOffset;
    uint16_t onLClickedOffset;
    uint16_t onRClickedOffset;
    uint16_t onMClickedOffset;
};

typedef struct Glyph
{
    uint16_t height;
    uint16_t width;
    RGBA_8UNORM * data;
} Glyph;

static_assert(sizeof(glm::vec3) == sizeof(float) * 3);
static_assert(sizeof(float) == 4);

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

enum class UIType { NOT = 0, SHAPE, BUTTON, TEXT, SIZE };

enum class GraphicsUpdateDependency { STATIC, ALL, SIZE };

struct UITypeMeshBinding
{
    UIType type;
    GraphicsUpdateDependency updateDependency;
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

// How to pack colours for indexing. You can seperate alpha since that will usually be 1, or 0

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

struct RelatedVertices
{
    uint8_t * start;
    uint8_t number;
    uint8_t stride;
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

    uint32_t allocatedVerticesMemory;
    uint32_t freeVerticesMemory;
    VkDeviceMemory verticesMemory;
    uint8_t * mappedVerticesMemory;

    uint32_t allocatedIndicesMemory;
    uint32_t freeIndicesMemory;
    VkDeviceMemory indicesMemory;
    uint8_t * mappedIndicesMemory;

    /* Entity Stuff */

    EntitySystemHandle entitySystem;
    static const constexpr uint16_t NUM_EVENTS = 40;
    static const constexpr uint16_t NUM_OPERATIONS = 10;

    // TODO: Array lengths are hardcoded

    uint16_t numPerFrameOperations;
    uint16_t perFrameOperationIndices[10];

    OnMouseEventOpBindings onMouseEventOpBindings;

    uint16_t activeBoundsIndices[10];
    uint16_t numActiveBounds = 0;

    // TODO: Clean up below
    uint8_t numOp16 = 0;
    uint8_t numOp8 = 0;
    uint8_t numOp4 = 0;
    uint8_t numOp2 = 0;

    Operation2 operations[NUM_OPERATIONS];

    Operation2& opAt(uint16_t index)
    {
        if(index < numOp16) {
            return *reinterpret_cast<Operation2*>((reinterpret_cast<Operation16*>(operations) + index));
        }

        uint8_t op2SizeCounter = numOp16 * 8;
        index -= numOp16;

        if(index < numOp8) {
            return *(reinterpret_cast<Operation2*>(reinterpret_cast<Operation8*>(operations) + index) + op2SizeCounter);
        }

        op2SizeCounter += numOp8 * 4;
        index -= numOp8;

        if(index < numOp4) {
            return *(reinterpret_cast<Operation2*>(reinterpret_cast<Operation4*>(operations) + index) + op2SizeCounter);
        }

        op2SizeCounter += numOp4 * 2;
        index -= numOp4;

        assert(index < numOp2 && "index for opAt is too big");

        return *(reinterpret_cast<Operation2*>(reinterpret_cast<Operation2*>(operations) + index) + op2SizeCounter);
    }

    void insertOp16(uint8_t opCode, uint8_t flags, Operation16Union op16Data)
    {
        assert(numOp2 == 0 && numOp4 == 0 && numOp8 == 0 && "You cannot insert an op16 after inserting a smaller sized operation");
        *reinterpret_cast<Operation16 *>(operations + numOp16 * 8) = { opCode, static_cast<uint8_t>(flags | OPERATION_FLAGS_SIZE_16), op16Data };
        numOp16++;
    }

    void insertOp8(uint8_t opCode, uint8_t flags, Operation8Union op8Data)
    {
        assert(numOp2 == 0 && numOp4 == 0 && "You cannot insert an op8 after inserting a smaller sized operation");
        *reinterpret_cast<Operation8 *>(operations + (numOp8 * 4) + (numOp16 * 8)) = { opCode, static_cast<uint8_t>(flags | OPERATION_FLAGS_SIZE_8), op8Data };
        numOp8++;
    }

    void insertOp4(uint8_t opCode, uint8_t flags, Operation4Union op4Data)
    {
        assert(numOp2 == 0 && "You cannot insert an op4 after inserting a smaller sized operation");
        *reinterpret_cast<Operation4 *>(operations + (numOp4 * 2) + (numOp8 * 4) + (numOp16 * 8)) = { opCode, static_cast<uint8_t>(flags | OPERATION_FLAGS_SIZE_4), op4Data };
        numOp4++;
    }

    void insertOp2(uint8_t opCode, uint8_t flags)
    {
        *reinterpret_cast<Operation2 *>(operations + numOp2 + (numOp4 * 2) + (numOp8 * 4) + (numOp16 * 8)) = { opCode, static_cast<uint8_t>(flags | OPERATION_FLAGS_SIZE_2) };
        numOp2++;
    }

    /* Entity Stuff end */

    std::array<PipelineType, static_cast<uint8_t>(PipelineType::SIZE)> pipelineDrawOrder;
    std::array<VulkanApplicationPipeline, static_cast<size_t>(PipelineType::SIZE)> pipelines;
    FontBitmap fontBitmap;

    void setPipeline(VulkanApplicationPipeline&& pipeline, PipelineType tag)
    {
        pipelines[static_cast<size_t>(tag)] = std::move(pipeline);
    }
};

#endif // TYPESVULKAN_H
