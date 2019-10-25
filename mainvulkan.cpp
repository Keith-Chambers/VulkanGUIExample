#include "mainvulkan.h"

static size_t currentFrame = 0;
static bool framebufferResized = false;

static double updatedFrameWidthRatio = 1.0;
static double updatedFrameHeightRatio = 1.0;

static uint16_t currentWindowHeight = vconfig::INITIAL_WINDOW_HEIGHT;
static uint16_t currentWindowWidth = vconfig::INITIAL_WINDOW_WIDTH;

const uint8_t VERTICES_PER_SQUARE = 4;
const uint8_t INDICES_PER_SQUARE = 6;

void recreateSwapChain(VulkanApplication& app)
{
    int width = 0, height = 0;

    while(width == 0 || height == 0)
    {
        glfwGetFramebufferSize(app.window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(app.device);

    cleanupSwapChain(app);

    createSwapChain(app.physicalDevice, app.device, app.surface, app.swapChain, app.swapChainImages, app.swapChainImageFormat, app.swapChainExtent, app.window);

    // Create Image View BEGIN
    app.swapChainImageViews.resize(app.swapChainImages.size());

    for (size_t i = 0; i < app.swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = app.swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = app.swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(app.device, &createInfo, nullptr, &app.swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

    // Create Image View END

    // TODO: Everything seems in order. Please disperse.
    bool hacky_bool_fix_later = true;

    updatedFrameWidthRatio = static_cast<double>(width) / currentWindowWidth;
    updatedFrameHeightRatio = static_cast<double>(height) / currentWindowHeight;

    for(VulkanApplicationPipeline& pipeline : app.pipelines)
    {
        vkDestroyDescriptorSetLayout(app.device, pipeline.descriptorSetLayout, nullptr);

        GenericGraphicsPipelineTargets pipelineSetup
        {
            & pipeline.graphicsPipeline,
            & pipeline.pipelineLayout,
            & pipeline.renderPass,
            & pipeline.descriptorSetLayout,
            & pipeline.swapChainFramebuffers
        };

        if(! updateGenericGraphicsPipeline(app.device, app.swapChainExtent, app.swapChainImageViews, static_cast<uint8_t>(app.swapChainImages.size()), pipelineSetup, pipeline.setupCache)) {
            throw std::runtime_error("Failed to update pipeline");
        }

        hacky_bool_fix_later = false;

        uint32_t numVertices = pipeline.numVertices;
        uint8_t * currentVertex = reinterpret_cast<uint8_t *>(pipeline.vertexDataStart);
        uint32_t vertexStride = pipeline.vertexDataStride;

        // TODO: There's a better way to do this..
        for(uint32_t i = 0; i < numVertices; i++)
        {
            reinterpret_cast<glm::vec2 *>(currentVertex)->x += 1;
            reinterpret_cast<glm::vec2 *>(currentVertex)->x /= 2;

            reinterpret_cast<glm::vec2 *>(currentVertex)->x /= static_cast<float>(updatedFrameWidthRatio);

            reinterpret_cast<glm::vec2 *>(currentVertex)->x *= 2;
            reinterpret_cast<glm::vec2 *>(currentVertex)->x -= 1;

            reinterpret_cast<glm::vec2 *>(currentVertex)->y += 1;
            reinterpret_cast<glm::vec2 *>(currentVertex)->y /= 2;

            reinterpret_cast<glm::vec2 *>(currentVertex)->y /= static_cast<float>(updatedFrameHeightRatio);

            reinterpret_cast<glm::vec2 *>(currentVertex)->y *= 2;
            reinterpret_cast<glm::vec2 *>(currentVertex)->y -= 1;

            currentVertex += vertexStride;
        }
    }

    assert(width > 0);
    assert(height > 0);

    currentWindowWidth = static_cast<uint16_t>(width);
    currentWindowHeight = static_cast<uint16_t>(height);

    // Create Description Pool Begin

    std::array<VkDescriptorPoolSize, 1> descriptorPoolSizes = {};

    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[0].descriptorCount = static_cast<uint32_t>(app.swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(app.swapChainImages.size());

    if (vkCreateDescriptorPool(app.device, &poolInfo, nullptr, &app.descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // Create Description Pool END

    for(VulkanApplicationPipeline& pipeline : app.pipelines)
    {
        if(pipeline.descriptorSetLayout != nullptr)
        {
            std::vector<VkDescriptorSetLayout> layouts(app.swapChainImages.size(), pipeline.descriptorSetLayout);

            VkDescriptorSetAllocateInfo descriptorAllocInfo = {};
            descriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorAllocInfo.descriptorPool = app.descriptorPool;
            descriptorAllocInfo.descriptorSetCount = static_cast<uint32_t>(app.swapChainImages.size());
            descriptorAllocInfo.pSetLayouts = layouts.data();

            pipeline.descriptorSets.resize(app.swapChainImages.size());

            if (vkAllocateDescriptorSets(app.device, &descriptorAllocInfo, pipeline.descriptorSets.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            // TODO: This isn't generic..
            for (size_t i = 0; i < app.swapChainImages.size(); i++) {

                VkDescriptorImageInfo imageInfo = {};

                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = pipeline.textureImageView;
                imageInfo.sampler = pipeline.textureSampler;

                assert(pipeline.textureImageView && "Image view -> null");
                assert(pipeline.textureSampler && "Sampler -> null");

                std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

                descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[0].dstSet = pipeline.descriptorSets[i];
                descriptorWrites[0].dstBinding = 0;
                descriptorWrites[0].dstArrayElement = 0;
                descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[0].descriptorCount = 1;
                descriptorWrites[0].pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(app.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            }
        }
    }

    app.commandBuffers.resize(app.swapChainImages.size());

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool = app.commandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    assert(app.commandBuffers.size() <= UINT32_MAX);

    commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(app.commandBuffers.size());

    if (vkAllocateCommandBuffers(app.device, &commandBufferAllocInfo, app.commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkClearValue clearColor = { /* .color = */  {  /* .float32 = */  { 0.0f, 0.0f, 0.0f, 1.0f } } };

    for (size_t i = 0; i < app.commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(app.commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        for(size_t layerIndex = 0; layerIndex < PipelineType::SIZE; layerIndex++)
        {
            VulkanApplicationPipeline& pipeline = app.pipelines[ app.pipelineDrawOrder[layerIndex] ];

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = pipeline.renderPass;
            renderPassInfo.framebuffer = pipeline.swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = app.swapChainExtent;

            bool requiresInitialClear = (layerIndex == 0);

            renderPassInfo.clearValueCount = (requiresInitialClear) ? 1 : 0;
            renderPassInfo.pClearValues = (requiresInitialClear) ? &clearColor : nullptr;

            vkCmdBeginRenderPass(app.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(app.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);

                VkBuffer vertexBuffers[] = {pipeline.vertexBuffer};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(app.commandBuffers[i], 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(app.commandBuffers[i], pipeline.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

                if(pipeline.pipelineLayout != nullptr && pipeline.descriptorSets.size() != 0) {
                    vkCmdBindDescriptorSets(app.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &pipeline.descriptorSets[i], 0, nullptr);
                }

                vkCmdDrawIndexed(app.commandBuffers[i], static_cast<uint32_t>(pipeline.drawIndices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(app.commandBuffers[i]);
        }

        if (vkEndCommandBuffer(app.commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

int main()
{
    VulkanApplication app = setupApplication();

    mainLoop(app);
    cleanup(app);

    puts("Clean termination");

    return 1;
}

void drawFrame(VulkanApplication& app)
{
    vkWaitForFences(app.device, 1, & app.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(app.device, app.swapChain, UINT64_MAX, app.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(app);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { app.imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &app.commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { app.renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(app.device, 1, & app.inFlightFences[currentFrame]);

    if (vkQueueSubmit(app.graphicsQueue, 1, &submitInfo, app.inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {app.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(app.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain(app);
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void mainLoop(VulkanApplication& app)
{
    using namespace std::chrono_literals;

    uint32_t framesPerSec = 0;

    constexpr uint16_t targetFPS = 50;

    std::chrono::milliseconds sinceLastFPSPrint = 0ms;

    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long, std::milli>>  frameStart;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long, std::milli>>  frameEnd;

    std::chrono::duration<long, std::milli> msPerFrame(1000 / targetFPS);
    std::chrono::duration<long, std::milli> frameLength;

    while(!glfwWindowShouldClose(app.window))
    {
        frameStart = std::chrono::time_point_cast<std::chrono::duration<long, std::milli>>( std::chrono::steady_clock::now() );

        glfwPollEvents();
        //glfwWaitEventsTimeout(0.0005);

        if(sinceLastFPSPrint >= 1000ms)
        {
            printf("FPS: %d\n", framesPerSec);
            framesPerSec = 0;
            sinceLastFPSPrint = 0ms;
        }

        drawFrame(app);
        framesPerSec++;

        frameEnd = std::chrono::time_point_cast<std::chrono::duration<long, std::milli>>( std::chrono::steady_clock::now() );
        sinceLastFPSPrint += msPerFrame;

        frameLength = frameEnd - frameStart;

        if(frameLength >= msPerFrame) {
            continue;
        }

        std::this_thread::sleep_for(msPerFrame - frameLength);
    }

    vkDeviceWaitIdle(app.device);
}

void createVertexBuffer(    const VkDevice device,
                            const VkPhysicalDevice physicalDevice,
                            const VkQueue graphicsQueue,
                            const VkCommandPool commandPool,
                            uint8_t * vertexData,
                            const uint32_t vertexDataSize,
                            VkBuffer &outVertexBuffer,
                            VkDeviceMemory& outVertexBufferMemory,
                            void ** outMappedMemory )
{

    if(vertexDataSize == 0) {
        return;
    }

    if(outVertexBuffer != nullptr)
    {
        puts("Warning: Existing Vertex Buffer being deallocated");
        vkDestroyBuffer(device, outVertexBuffer, nullptr);
        vkFreeMemory(device, outVertexBufferMemory, nullptr);
    }

    VkDeviceSize bufferSize = vertexDataSize;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(   device,
                    physicalDevice,
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory);

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, outMappedMemory);
        memcpy(*outMappedMemory, vertexData, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);


    createBuffer(   device,
                    physicalDevice,
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    outVertexBuffer,
                    outVertexBufferMemory);

    copyBuffer( device,
                commandPool,
                graphicsQueue,
                stagingBuffer,
                outVertexBuffer,
                bufferSize);

    vkMapMemory(device, outVertexBufferMemory, 0, bufferSize, 0, outMappedMemory);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void createIndexBuffer( const VkDevice device,
                        const VkPhysicalDevice physicalDevice,
                        const VkQueue graphicsQueue,
                        const VkCommandPool commandPool,
                        const std::vector<uint16_t>& indices,
                        VkBuffer &outIndicesBuffer,
                        VkDeviceMemory& outIndicesBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    if(bufferSize == 0) {
        return;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(   device,
                    physicalDevice,
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer,
                    stagingBufferMemory);

    void * mappedIndicesData;

    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &mappedIndicesData);
        memcpy(mappedIndicesData, indices.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(   device,
                    physicalDevice,
                    bufferSize,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    outIndicesBuffer,
                    outIndicesBufferMemory );

    copyBuffer( device,
                commandPool,
                graphicsQueue,
                stagingBuffer,
                outIndicesBuffer,
                bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    // Disable parameters not used warnings
    (void)window;
    (void)width;
    (void)height;

    framebufferResized = true;
}

//void renderText(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char * text, const char * fontPath, uint8_t fontSize)
//{

//}

VulkanApplication setupApplication()
{
    setvbuf(stdout, nullptr, _IOLBF, 0);

    // Create Swapchain BEGIN
    VulkanApplication app;

    initializeVulkan(app);

    glfwSetFramebufferSizeCallback(app.window, framebufferResizeCallback);

    app.pipelineDrawOrder[0] = PipelineType::PrimativeShapes;
    app.pipelineDrawOrder[1] = PipelineType::Texture;

    app.pipelines[PipelineType::Texture] = {};
    VulkanApplicationPipeline& texturesPipeline = app.pipelines[PipelineType::Texture];

{
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    descriptorSetLayoutBindings.push_back(samplerLayoutBinding);

    GenericGraphicsPipelineSetup textureGraphicsPipelineCreateInfo;

    textureGraphicsPipelineCreateInfo.vertexShaderPath = "shaders/vert.spv";
    textureGraphicsPipelineCreateInfo.fragmentShaderPath = "shaders/frag.spv";
    textureGraphicsPipelineCreateInfo.device = app.device;
    textureGraphicsPipelineCreateInfo.swapChainImageFormat = app.swapChainImageFormat;
    textureGraphicsPipelineCreateInfo.vertexBindingDescription = Vertex::getBindingDescription();
    textureGraphicsPipelineCreateInfo.vertexAttributeDescriptions = Vertex::getAttributeDescriptions();
    textureGraphicsPipelineCreateInfo.swapChainExtent = app.swapChainExtent;
    textureGraphicsPipelineCreateInfo.descriptorSetLayoutBindings = descriptorSetLayoutBindings;
    textureGraphicsPipelineCreateInfo.swapChainSize = static_cast<uint8_t>(app.swapChainImages.size());
    textureGraphicsPipelineCreateInfo.swapChainImageViews = app.swapChainImageViews;

    GenericGraphicsPipelineTargets texturesPipelineSetup
    {
        & texturesPipeline.graphicsPipeline,
        & texturesPipeline.pipelineLayout,
        & texturesPipeline.renderPass,
        & texturesPipeline.descriptorSetLayout,
        & texturesPipeline.swapChainFramebuffers
    };

    if(! createGenericGraphicsPipeline(textureGraphicsPipelineCreateInfo, texturesPipelineSetup, texturesPipeline.setupCache, false)) {
        throw std::runtime_error("Failed to create the first pipeline");
    }

    assert(texturesPipeline.setupCache.descriptorSetLayoutBindings.size() == 1);

}
    // Create Graphics Pipeline END

    // Create SECOND Pipeline

    // There aren't any descriptorSets being used for this pipeline
    std::vector<VkDescriptorSetLayoutBinding> primativeShapesPipelineDescriptorSetLayoutBindings;

    GenericGraphicsPipelineSetup graphicsPipelineCreateInfo;

    GenericGraphicsPipelineSetup primativeShapesGraphicsPipelineCreateInfo;

    primativeShapesGraphicsPipelineCreateInfo.vertexShaderPath = "shaders/simple_vert.spv";
    primativeShapesGraphicsPipelineCreateInfo.fragmentShaderPath = "shaders/simple_frag.spv";
    primativeShapesGraphicsPipelineCreateInfo.device = app.device;
    primativeShapesGraphicsPipelineCreateInfo.swapChainImageFormat = app.swapChainImageFormat;
    primativeShapesGraphicsPipelineCreateInfo.vertexBindingDescription = BasicVertex::getBindingDescription();
    primativeShapesGraphicsPipelineCreateInfo.vertexAttributeDescriptions = BasicVertex::getAttributeDescriptions();
    primativeShapesGraphicsPipelineCreateInfo.swapChainExtent = app.swapChainExtent;
    primativeShapesGraphicsPipelineCreateInfo.descriptorSetLayoutBindings = primativeShapesPipelineDescriptorSetLayoutBindings;
    primativeShapesGraphicsPipelineCreateInfo.swapChainSize = static_cast<uint8_t>(app.swapChainImages.size());
    primativeShapesGraphicsPipelineCreateInfo.swapChainImageViews = app.swapChainImageViews;

    app.pipelines[PipelineType::PrimativeShapes] = {};
    VulkanApplicationPipeline& primativeShapesPipeline = app.pipelines[PipelineType::PrimativeShapes];

    GenericGraphicsPipelineTargets primativeShapesPipelineSetup
    {
        & primativeShapesPipeline.graphicsPipeline,
        & primativeShapesPipeline.pipelineLayout,
        & primativeShapesPipeline.renderPass,
        & primativeShapesPipeline.descriptorSetLayout,
        & primativeShapesPipeline.swapChainFramebuffers
    };

    if(! createGenericGraphicsPipeline(primativeShapesGraphicsPipelineCreateInfo, primativeShapesPipelineSetup, primativeShapesPipeline.setupCache, true)) {
        throw std::runtime_error("Failed to create the second pipeline");
    }

    // Create Command Pool BEGIN

    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(app.physicalDevice, app.surface);

    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(app.device, &commandPoolInfo, nullptr, &app.commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }

    // Create Command Pool END

    FT_Library ft;

    if(FT_Init_FreeType(&ft)) {
        assert(false && "Failed to setup freetype");
    }

    FT_Face face;

    if(FT_New_Face(ft, vconfig::DEFAULT_FONT_PATH, 0, &face)) {
        assert(false && "Failed to load font");
    }

    FT_Select_Charmap(face, FT_ENCODING_UNICODE );

//    assert(FT_HAS_KERNING( face ));

    FT_Set_Pixel_Sizes(face, 0, 28);

    FontBitmap fontBitmap;
    fontBitmap.face = face;

    std::string unique_chars = "abcdefghijklmnopqrstuvwxyz \"ABCDEFGHIJKLMNOPQRSTUVWXYZ.<>!?";
    uint16_t cell_size = 40; // TODO: No assert for when this gets too small
    uint16_t cells_per_line = 10;

    instanciateFontBitmap(fontBitmap, face, unique_chars.c_str(), cells_per_line, cell_size );

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // TODO: Part of pipeline setup??
    createTextureImage( app.device,
                        app.physicalDevice,
                        app.commandPool,
                        app.graphicsQueue,
                        reinterpret_cast<uint8_t *>(fontBitmap.bitmap_data),
                        fontBitmap.texture_width,
                        fontBitmap.texture_height,
                        texturesPipeline.textureImage,
                        texturesPipeline.textureImageMemory);

    createImageView(app.device, texturesPipeline.textureImage, VK_FORMAT_R8G8B8A8_UNORM, texturesPipeline.textureImageView);

    // Create Texture Sampler BEGIN
    VkSamplerCreateInfo samplerInfo = {};

    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(app.device, &samplerInfo, nullptr, &texturesPipeline.textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    // Create Texture Sampler END

//    glm::vec3 color = { 0.0f, 1.0f, 0.0f};

    std::vector<Vertex> vertices = {
//        {{-0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},   // Top, left        // Right, bottom
//        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},    // Top, right       // Left, bottom
//        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},     // Bottom, right    // Left, Top
//        {{-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}     // Bottom, left     // Right, Top
    };

//    std::vector<uint16_t> indices = {
////        0, 1, 2, 2, 3, 0
//    };

    uint16_t stride = static_cast<uint16_t>(
                reinterpret_cast<uint8_t *>(&vertices[1].texCoord) - reinterpret_cast<uint8_t *>(&vertices[0].texCoord) );

    std::string otherText = "How are you doing today? I hope you are doing well!";

    vertices.resize( vertices.size() + (otherText.size() * VERTICES_PER_SQUARE) );
    texturesPipeline.drawIndices.resize( texturesPipeline.drawIndices.size() + (otherText.size() * INDICES_PER_SQUARE) );

    generateTextMeshes(texturesPipeline.drawIndices.data(), &(vertices.data())->pos, sizeof(Vertex), 0, fontBitmap, &vertices[0].texCoord, stride, otherText, 150, 25);

    std::string moreText = "Im doing pretty good indeed";
{
    uint16_t currentVertexIndex =   static_cast<uint16_t>(vertices.size());
    uint16_t currentIndicesIndex =  static_cast<uint16_t>(texturesPipeline.drawIndices.size());

    vertices.resize(vertices.size() + moreText.size() * 4);
    texturesPipeline.drawIndices.resize( texturesPipeline.drawIndices.size() + (moreText.size() * 6) );

    generateTextMeshes( &texturesPipeline.drawIndices[currentIndicesIndex],
                        &vertices[currentVertexIndex].pos,
                        sizeof(Vertex),
                        currentVertexIndex,
                        fontBitmap,
                        &vertices[currentVertexIndex].texCoord,
                        stride,
                        moreText, 150, 250);
}
//    assert(vertices.size() > 0);

    void * mappedVertexData = nullptr;

    createVertexBuffer( app.device,
                        app.physicalDevice,
                        app.graphicsQueue,
                        app.commandPool,
                        reinterpret_cast<uint8_t *>(vertices.data()),
                        static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])),
                        texturesPipeline.vertexBuffer,
                        texturesPipeline.vertexBufferMemory,
                        &mappedVertexData );

//    assert(mappedVertexData != nullptr);

    createIndexBuffer(  app.device,
                        app.physicalDevice,
                        app.graphicsQueue,
                        app.commandPool,
                        texturesPipeline.drawIndices,
                        texturesPipeline.indexBuffer,
                        texturesPipeline.indexBufferMemory);

    texturesPipeline.vertexDataStart = reinterpret_cast<glm::vec2 *>(mappedVertexData);
    texturesPipeline.vertexDataStride = static_cast<uint32_t>( reinterpret_cast<uint8_t *>(&vertices[1].pos) - reinterpret_cast<uint8_t *>(&vertices[0].pos) );
    texturesPipeline.numVertices = static_cast<uint32_t>(vertices.size());

    // Create Vertex & Index Buffers for second pipeline

    std::vector<BasicVertex> simpleShapesVertices = {
        {{-0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}}
    };

    primativeShapesPipeline.drawIndices = {
        0, 1, 2, 2, 3, 0
    };

    createVertexBuffer( app.device,
                        app.physicalDevice,
                        app.graphicsQueue,
                        app.commandPool,
                        reinterpret_cast<uint8_t *>(simpleShapesVertices.data()),
                        static_cast<uint32_t>(simpleShapesVertices.size() * sizeof(simpleShapesVertices[0])),
                        primativeShapesPipeline.vertexBuffer,
                        primativeShapesPipeline.vertexBufferMemory,
                        & mappedVertexData);

    createIndexBuffer(  app.device,
                        app.physicalDevice,
                        app.graphicsQueue,
                        app.commandPool,
                        primativeShapesPipeline.drawIndices,
                        primativeShapesPipeline.indexBuffer,
                        primativeShapesPipeline.indexBufferMemory);

    primativeShapesPipeline.vertexDataStart = reinterpret_cast<glm::vec2 *>(mappedVertexData);
    primativeShapesPipeline.vertexDataStride = static_cast<uint32_t>( reinterpret_cast<uint8_t *>(&simpleShapesVertices[1].pos) - reinterpret_cast<uint8_t *>(&simpleShapesVertices[0].pos) );
    primativeShapesPipeline.numVertices = static_cast<uint32_t>(simpleShapesVertices.size());

    // Create Description Pool Begin

    std::array<VkDescriptorPoolSize, 1> descriptorPoolSizes = {};

    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSizes[0].descriptorCount = static_cast<uint32_t>(app.swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(app.swapChainImages.size());

//    VkDescriptorPool descriptorPool; // TODO: This is a hack, should be stored somewhere proper

    if (vkCreateDescriptorPool(app.device, &poolInfo, nullptr, &app.descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // Create Description Pool END

    // Create Description Sets BEGIN
    std::vector<VkDescriptorSetLayout> layouts(app.swapChainImages.size(), texturesPipeline.descriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorAllocInfo = {};
    descriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocInfo.descriptorPool = app.descriptorPool;
    descriptorAllocInfo.descriptorSetCount = static_cast<uint32_t>(app.swapChainImages.size());
    descriptorAllocInfo.pSetLayouts = layouts.data();

    texturesPipeline.descriptorSets.resize(app.swapChainImages.size());

    if (vkAllocateDescriptorSets(app.device, &descriptorAllocInfo, texturesPipeline.descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < app.swapChainImages.size(); i++)
    {

        VkDescriptorImageInfo imageInfo = {};

        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texturesPipeline.textureImageView;
        imageInfo.sampler = texturesPipeline.textureSampler;

        assert(texturesPipeline.textureImageView && "Image view = null");
        assert(texturesPipeline.textureSampler && "Sampler = null");

        std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = texturesPipeline.descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(app.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    // Create Description Sets END

    // Create Command Buffers BEGIN

    app.commandBuffers.resize(texturesPipeline.swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool = app.commandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(app.commandBuffers.size());

    if (vkAllocateCommandBuffers(app.device, &commandBufferAllocInfo, app.commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkClearValue clearColor = { /* .color = */  {  /* .float32 = */  { 0.0f, 0.0f, 0.0f, 1.0f } } };

    for (size_t i = 0; i < app.commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(app.commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        for(size_t layerIndex = 0; layerIndex < PipelineType::SIZE; layerIndex++)
        {
            VulkanApplicationPipeline& pipeline = app.pipelines[ app.pipelineDrawOrder[layerIndex] ];

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = pipeline.renderPass;
            renderPassInfo.framebuffer = pipeline.swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = app.swapChainExtent;

            bool requiresInitialClear = (layerIndex == 0);

            renderPassInfo.clearValueCount = (requiresInitialClear) ? 1 : 0;
            renderPassInfo.pClearValues = (requiresInitialClear) ? &clearColor : nullptr;

            vkCmdBeginRenderPass(app.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(app.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);

                VkBuffer vertexBuffers[] = {pipeline.vertexBuffer};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(app.commandBuffers[i], 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(app.commandBuffers[i], pipeline.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

                if(pipeline.pipelineLayout != nullptr && pipeline.descriptorSets.size() != 0) {
                    vkCmdBindDescriptorSets(app.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &pipeline.descriptorSets[i], 0, nullptr);
                }

                vkCmdDrawIndexed(app.commandBuffers[i], static_cast<uint32_t>(pipeline.drawIndices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(app.commandBuffers[i]);
        }

        if (vkEndCommandBuffer(app.commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    // Create Command Buffers END

    // Create Sync Objects BEGIN

    app.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    app.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    app.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(app.device, &semaphoreInfo, nullptr, &app.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(app.device, &semaphoreInfo, nullptr, &app.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(app.device, &fenceInfo, nullptr, &app.inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    // Create Sync Objects END

    return app;
}

bool updateGenericGraphicsPipeline( VkDevice device,
                                    VkExtent2D swapchainExtent,
                                    std::vector<VkImageView>& swapChainImageViews,
                                    uint8_t swapchainSize,
                                    GenericGraphicsPipelineTargets& out,
                                    PipelineSetupData &setupCache)
{

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &setupCache.attachmentReference;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &setupCache.attachmentDescription;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &setupCache.subpassDependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, out.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    if(setupCache.descriptorSetLayoutBindings.size() == 0) {
        out.descriptorSetLayout = nullptr;
    } else {

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(setupCache.descriptorSetLayoutBindings.size());
        layoutInfo.pBindings = setupCache.descriptorSetLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, out.descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    // TODO: Get rid of VertexInputInfo structure as it can be reconstructed and isn't providing any value as-is
    setupCache.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    setupCache.vertexInputInfo.vertexBindingDescriptionCount = 1;
    setupCache.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(setupCache.vertexAttributeDescriptions.size());
    setupCache.vertexInputInfo.pVertexBindingDescriptions = &setupCache.vertexBindingDescription;
    setupCache.vertexInputInfo.pVertexAttributeDescriptions = setupCache.vertexAttributeDescriptions.data();

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainExtent.width);
    viewport.height = static_cast<float>(swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;

//    VkPipelineViewportStateCreateInfo viewportState = {};
    setupCache.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    setupCache.viewportState.viewportCount = 1;
    setupCache.viewportState.pViewports = &viewport;
    setupCache.viewportState.scissorCount = 1;
    setupCache.viewportState.pScissors = &scissor;

    setupCache.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if(setupCache.descriptorSetLayoutBindings.size() == 0)
    {
        setupCache.pipelineLayoutInfo.setLayoutCount = 0;
        setupCache.pipelineLayoutInfo.pSetLayouts = nullptr;
    } else
    {
        setupCache.pipelineLayoutInfo.setLayoutCount = 1;
        setupCache.pipelineLayoutInfo.pSetLayouts = out.descriptorSetLayout;
    }

    if (vkCreatePipelineLayout(device, &setupCache.pipelineLayoutInfo, nullptr, out.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = setupCache.shaderStages;
    pipelineInfo.pVertexInputState = &setupCache.vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &setupCache.inputAssembly;
    pipelineInfo.pViewportState = &setupCache.viewportState;
    pipelineInfo.pRasterizationState = &setupCache.rasterizer;
    pipelineInfo.pMultisampleState = &setupCache.multisampling;
    pipelineInfo.pColorBlendState = &setupCache.colorBlending;
    pipelineInfo.layout = *out.pipelineLayout;
    pipelineInfo.renderPass = *out.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, out.graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    out.swapChainFramebuffers->resize(swapchainSize);

    createGenericFrameBuffers(  *out.renderPass,
                                device,
                                swapChainImageViews,
                                *out.swapChainFramebuffers,
                                swapchainExtent );

    return true;
}

// TODO: clearFirst is a bit of a hack. Should come up with something more flexible
bool createGenericGraphicsPipeline(const GenericGraphicsPipelineSetup& params, GenericGraphicsPipelineTargets& out, PipelineSetupData &outSetup, bool clearFirst)
{

    outSetup.descriptorSetLayoutBindings = params.descriptorSetLayoutBindings;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = params.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = (clearFirst) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = (clearFirst) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    outSetup.attachmentDescription = colorAttachment;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    outSetup.attachmentReference = colorAttachmentRef;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    outSetup.subpassDependency = dependency;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(params.device, &renderPassInfo, nullptr, out.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    if(outSetup.descriptorSetLayoutBindings.size() == 0) {
        out.descriptorSetLayout = nullptr;
    } else {

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(outSetup.descriptorSetLayoutBindings.size());
        layoutInfo.pBindings = outSetup.descriptorSetLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(params.device, &layoutInfo, nullptr, out.descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    auto vertShaderCode = readFile(params.vertexShaderPath.c_str());
    auto fragShaderCode = readFile(params.fragmentShaderPath.c_str());

    VkShaderModule vertShaderModule = createShaderModule(params.device, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(params.device, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    outSetup.shaderStages[0] = vertShaderStageInfo;
    outSetup.shaderStages[1] = fragShaderStageInfo;

    outSetup.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    outSetup.vertexBindingDescription = params.vertexBindingDescription;
    outSetup.vertexAttributeDescriptions = params.vertexAttributeDescriptions;

    outSetup.vertexInputInfo.vertexBindingDescriptionCount = 1;
    outSetup.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(outSetup.vertexAttributeDescriptions.size());
    outSetup.vertexInputInfo.pVertexBindingDescriptions = &outSetup.vertexBindingDescription;
    outSetup.vertexInputInfo.pVertexAttributeDescriptions = outSetup.vertexAttributeDescriptions.data();

//    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    outSetup.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    outSetup.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    outSetup.inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(params.swapChainExtent.width);
    viewport.height = static_cast<float>(params.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = params.swapChainExtent;

//    VkPipelineViewportStateCreateInfo viewportState = {};
    outSetup.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    outSetup.viewportState.viewportCount = 1;
    outSetup.viewportState.pViewports = &viewport;
    outSetup.viewportState.scissorCount = 1;
    outSetup.viewportState.pScissors = &scissor;

//    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    outSetup.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    outSetup.rasterizer.depthClampEnable = VK_FALSE;
    outSetup.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    outSetup.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    outSetup.rasterizer.lineWidth = 1.0f;
    outSetup.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    outSetup.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    outSetup.rasterizer.depthBiasEnable = VK_FALSE;

//    VkPipelineMultisampleStateCreateInfo multisampling = {};
    outSetup.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    outSetup.multisampling.sampleShadingEnable = VK_FALSE;
    outSetup.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

//    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    outSetup.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    outSetup.colorBlendAttachment.blendEnable = VK_TRUE;
    outSetup.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    outSetup.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

    outSetup.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    outSetup.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

    outSetup.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    outSetup.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;

//    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    outSetup.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    outSetup.colorBlending.logicOpEnable = VK_FALSE;
    outSetup.colorBlending.logicOp = VK_LOGIC_OP_COPY;
    outSetup.colorBlending.attachmentCount = 1;
    outSetup.colorBlending.pAttachments = &outSetup.colorBlendAttachment;
    outSetup.colorBlending.blendConstants[0] = 0.0f;
    outSetup.colorBlending.blendConstants[1] = 0.0f;
    outSetup.colorBlending.blendConstants[2] = 0.0f;
    outSetup.colorBlending.blendConstants[3] = 0.0f;

//    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    outSetup.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if(outSetup.descriptorSetLayoutBindings.size() == 0)
    {
        outSetup.pipelineLayoutInfo.setLayoutCount = 0;
        outSetup.pipelineLayoutInfo.pSetLayouts = nullptr;
    } else
    {
        outSetup.pipelineLayoutInfo.setLayoutCount = 1;
        outSetup.pipelineLayoutInfo.pSetLayouts = out.descriptorSetLayout;
    }

    if (vkCreatePipelineLayout(params.device, &outSetup.pipelineLayoutInfo, nullptr, out.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = outSetup.shaderStages;
    pipelineInfo.pVertexInputState = &outSetup.vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &outSetup.inputAssembly;
    pipelineInfo.pViewportState = &outSetup.viewportState;
    pipelineInfo.pRasterizationState = &outSetup.rasterizer;
    pipelineInfo.pMultisampleState = &outSetup.multisampling;
    pipelineInfo.pColorBlendState = &outSetup.colorBlending;
    pipelineInfo.layout = *out.pipelineLayout;
    pipelineInfo.renderPass = *out.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(params.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, out.graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    out.swapChainFramebuffers->resize(params.swapChainSize);

    createGenericFrameBuffers(  *out.renderPass,
                                params.device,
                                params.swapChainImageViews,
                                *out.swapChainFramebuffers,
                                params.swapChainExtent );

    return true;
}
