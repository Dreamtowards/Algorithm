
#include "vkx.hpp"

#include <iostream>
#include <sstream>
#include <set>

#pragma region base

#define DECL_CTX_device_allocator const vk::Device& device = vkx::ctx().Device; const vk::AllocationCallbacks* allocator = vkx::ctx().Allocator;

void vkx::check(const vk::Result& r)
{
    if (r != vk::Result::eSuccess)
    {
        assert(false, "vkx::check() fail.");
    }
}
void vkx::check(const VkResult& r)
{
    vkx::check((vk::Result)r);
}

#define VKX_CHECK(result) vkx::check(result)


#pragma endregion











#pragma region AllocateMemory

static uint32_t _FindMemoryType(uint32_t memTypeFilter, vk::MemoryPropertyFlags memPropertiesFlags)
{
    vk::PhysicalDeviceMemoryProperties memProperties = vkx::ctx().PhysDeviceMemoryProperties;

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & memPropertiesFlags) == memPropertiesFlags) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

vk::DeviceMemory vkx::AllocateMemory(
    vk::MemoryRequirements memRequirements,  // size, alignment, memoryType
    vk::MemoryPropertyFlags memProperties)
{
    DECL_CTX_device_allocator;

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _FindMemoryType(memRequirements.memoryTypeBits, memProperties);

    return device.allocateMemory(allocInfo, allocator);
}

#pragma endregion


#pragma region CommandBuffer

void vkx::AllocateCommandBuffers(
    int numCmdbuf,
    vk::CommandBuffer* pCmdbufs,
    vk::CommandBufferLevel level,
    vk::CommandPool commandPool,
    vk::Device device)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = numCmdbuf;

    VKX_CHECK(device.allocateCommandBuffers(&allocInfo, pCmdbufs));
}

void vkx::BeginCommandBuffer(
    vk::CommandBuffer cmdbuf,
    vk::CommandBufferUsageFlags usageFlags)
{
    cmdbuf.begin(vk::CommandBufferBeginInfo{ .flags = usageFlags });
}

void vkx::SubmitCommandBuffer(
    const std::function<void(vk::CommandBuffer)>& fn_record,
    vk::Queue queue,
    vk::CommandPool commandPool,
    vk::Device device)
{
    vk::CommandBuffer cmdbuf;
    vkx::AllocateCommandBuffers(1, &cmdbuf, vk::CommandBufferLevel::ePrimary);

    vkx::BeginCommandBuffer(cmdbuf, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    fn_record(cmdbuf);

    cmdbuf.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdbuf;
    queue.submit(submitInfo);
    queue.waitIdle();

    device.freeCommandBuffers(commandPool, cmdbuf);
}


void vkx::QueueSubmit(
    vk::Queue queue, 
    std::span<const vk::CommandBuffer> cmdbufs,
    std::span<const vk::Semaphore> waits,
    vk::PipelineStageFlags* waitStages,  // vk::PipelineStageFlagBits::eColorAttachmentOutput
    std::span<const vk::Semaphore> signals,
    vk::Fence fence)
{
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = cmdbufs.size();
    submitInfo.pCommandBuffers = cmdbufs.data();

    submitInfo.waitSemaphoreCount = waits.size();
    submitInfo.pWaitSemaphores = waits.data();
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.signalSemaphoreCount = signals.size();
    submitInfo.pSignalSemaphores = signals.data();

    queue.submit(submitInfo, fence);
}

#pragma endregion


#pragma region Image, ImageView. ImageSampler.

void vkx::CreateImage(
    int width,
    int height, 
    vk::Image* pImage,
    vk::DeviceMemory* pImageMemory,
    vk::Format format,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags memProperties,
    vk::ImageTiling tiling,
    bool isCubeMap)
{
    DECL_CTX_device_allocator;

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.format = format;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = isCubeMap ? 6 : 1;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage | vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = tiling;
    //imageInfo.flags = 0; // Optional

    if (isCubeMap) {
        imageInfo.flags |= vk::ImageCreateFlagBits::eCubeCompatible;
    }

    *pImage = device.createImage(imageInfo, allocator);

    vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(*pImage);

    *pImageMemory = vkx::AllocateMemory(memRequirements, memProperties);

    device.bindImageMemory(*pImage, *pImageMemory, 0);
}

vk::ImageView vkx::CreateImageView(
    vk::Image image,
    vk::Format format,
    vk::ImageAspectFlags aspectFlags,
    vk::ImageViewType imageViewType)
{
    DECL_CTX_device_allocator;

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.viewType = imageViewType;
    viewInfo.format = format;
    viewInfo.image = image;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = imageViewType == vk::ImageViewType::eCube ? 6 : 1;  // except CubeMap=6 or VR.
    //viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    //viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    //viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    //viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    return device.createImageView(viewInfo, allocator);
}

vk::Sampler vkx::CreateImageSampler(
    vk::Filter magFilter,
    vk::Filter minFilter,
    vk::SamplerAddressMode addressModeUVW)
{
    DECL_CTX_device_allocator;
    vk::PhysicalDevice physDevice = vkx::ctx().PhysDevice;
    vk::PhysicalDeviceProperties gpuProperties = vkx::ctx().PhysDeviceProperties;

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = magFilter;
    samplerInfo.minFilter = minFilter;
    samplerInfo.addressModeU = addressModeUVW;
    samplerInfo.addressModeV = addressModeUVW;
    samplerInfo.addressModeW = addressModeUVW;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = gpuProperties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp  = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    return device.createSampler(samplerInfo, allocator);
}



static bool _IsFormatHasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}
static void _TransitionImageLayout(
    vk::Image image,
    vk::Format format,
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
    uint32_t layerCount = 1)
{
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;
    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (_IsFormatHasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkx::SubmitCommandBuffer([&](vk::CommandBuffer cmd)
    {
        cmd.pipelineBarrier(
            srcStage, dstStage,
            (vk::DependencyFlags)0,
            {},
            {},
            barrier);
    });
}



static vk::Format _FindSupportedFormat(
    const std::vector<vk::Format>& candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates) {
        vk::FormatProperties props = vkx::ctx().PhysDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format.");
}
static vk::Format _FindDepthFormat() {
    return _FindSupportedFormat(
        { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}


vkx::Image vkx::CreateDepthImage(int width, int height)
{
    vk::Format depthFormat = _FindDepthFormat();

    vk::Image image;
    vk::DeviceMemory imageMemory;
    vkx::CreateImage(width, height, &image, &imageMemory,
        depthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment);

    vk::ImageView imageView = vkx::CreateImageView(image, depthFormat, vk::ImageAspectFlagBits::eDepth);

    _TransitionImageLayout(image, depthFormat,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    return vkx::Image(image, imageMemory, depthFormat, width, height, imageView);
}

#pragma endregion


#pragma region RenderPass, Framebuffer

vk::RenderPass vkx::CreateRenderPass(
    slice_t<const vk::AttachmentDescription> attachments,
    slice_t<const vk::SubpassDescription> subpasses,
    slice_t<const vk::SubpassDependency> dependencies)
{
    DECL_CTX_device_allocator;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    return device.createRenderPass(renderPassInfo, allocator);
}


vk::AttachmentDescription vkx::IAttachmentDesc(
    vk::Format format,
    vk::ImageLayout finalLayout)
{
    vk::AttachmentDescription d{};
    d.format = format;
    d.samples = vk::SampleCountFlagBits::e1;
    d.loadOp  = vk::AttachmentLoadOp::eClear;
    d.storeOp = vk::AttachmentStoreOp::eStore;
    d.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    d.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    d.initialLayout  = vk::ImageLayout::eUndefined;
    d.finalLayout = finalLayout;  // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL / VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    return d;
}

vk::AttachmentReference vkx::IAttachmentRef(
    uint32_t attachment,
    vk::ImageLayout layout)
{
    vk::AttachmentReference a{};
    a.attachment = attachment;
    a.layout = layout;
    return a;
}

vk::SubpassDescription vkx::IGraphicsSubpass(
    slice_t<const vk::AttachmentReference> colorAttachmentRefs,
    const vk::AttachmentReference& depthStencilAttachment)
{
    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = colorAttachmentRefs.size();
    subpass.pColorAttachments = colorAttachmentRefs.data();

    if (depthStencilAttachment.attachment) {  // valid
        subpass.pDepthStencilAttachment = &depthStencilAttachment;
    }
    return subpass;
}


vk::SubpassDependency vkx::ISubpassDependency(
    uint32_t srcSubpass, uint32_t dstSubpass,
    vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
    vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
    vk::DependencyFlags dependencyFlags)
{
    vk::SubpassDependency dependency{};
    dependency.srcSubpass = srcSubpass;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = srcStageMask;
    dependency.dstStageMask = dstStageMask;
    dependency.srcAccessMask = srcAccessMask;
    dependency.dstAccessMask = dstAccessMask;
    dependency.dependencyFlags = dependencyFlags;
    
    return dependency;
}


vk::Framebuffer vkx::CreateFramebuffer(
    vk::Extent2D wh,
    vk::RenderPass renderPass, 
    slice_t<const vk::ImageView> attachments)
{
    DECL_CTX_device_allocator;

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.width = wh.width;
    framebufferInfo.height = wh.height;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.layers = 1;

    return device.createFramebuffer(framebufferInfo, allocator);
}

#pragma endregion







#pragma region Create: Instance & DebugMessenger, ValidationLayer

inline static VkDebugUtilsMessengerEXT g_DebugUtilsMessengerEXT = nullptr;
inline static std::vector<const char*> g_ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

static void _EXT_DestroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    assert(func);
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL _DebugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
    void* pUserData) 
{
    //const char* MSG_SERV = "VERBO";
    //if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) MSG_SERV = "INFO";
    //else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) MSG_SERV = "WARN";
    //else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) MSG_SERV = "ERROR";
    //
    //const char* MSG_TYPE = "GENERAL";
    //if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) MSG_TYPE = "VALIDATION";
    //else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) MSG_TYPE = "PERFORMANCE";
    //
    //if (messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    //{
    //    std::cerr << "VkLayer[" << MSG_SERV << "][" << MSG_TYPE  << "]: " << pCallbackData->pMessage << std::endl;
    //
    //    std::cerr.flush();
    //}

    vkx::ctx().DebugMessengerCallback(
        static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity), 
        static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType),
        pCallbackData);

    return VK_FALSE;
}


// for VkInstance, not VkDevice.
static void _CheckValidationLayersSupport(const std::vector<const char*>& validationLayers) 
{
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
        bool found = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::stringstream ss;
            for (const auto& layerProperties : availableLayers) {
                ss << layerProperties.layerName << ", ";
            }
            throw std::runtime_error(std::format("Required validation layer '{}' is not available. There are {} layers available: {}", layerName, availableLayers.size(), ss.str()));
        }
    }
}

static vk::Instance _CreateInstance(
    bool enableValidationLayer, 
    const std::vector<const char*>& in_extensions)
{
    vk::ApplicationInfo appInfo{
        .pApplicationName = "N/A",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_0
    };

    vk::InstanceCreateInfo instInfo{};
    instInfo.pApplicationInfo = &appInfo;


    // Instance Extensions

    std::vector<const char*> extensions = in_extensions;

#ifdef __APPLE__
    // for prevents VK_ERROR_INCOMPATIBLE_DRIVER err on MacOS MoltenVK sdk.
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    instInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    // for supports VK_KHR_portability_subset logical-device-extension on MacOS.
    extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

    if (enableValidationLayer) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instInfo.enabledExtensionCount = extensions.size();
    instInfo.ppEnabledExtensionNames = extensions.data();



    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
    if (enableValidationLayer)
    {
        _CheckValidationLayersSupport(g_ValidationLayers);

        instInfo.enabledLayerCount = g_ValidationLayers.size();
        instInfo.ppEnabledLayerNames = g_ValidationLayers.data();

        debugMessengerInfo.messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugMessengerInfo.messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        debugMessengerInfo.pfnUserCallback = _DebugMessengerCallback;
        debugMessengerInfo.pUserData = nullptr; // Optional
        instInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerInfo;
    }

    vk::Instance instance = vk::createInstance(instInfo, vkx::ctx().Allocator);

    if (enableValidationLayer)
    {
        // Setup EXT DebugMessenger
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        assert(func != nullptr);
        vkx::check(func(instance, (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerInfo, nullptr, &g_DebugUtilsMessengerEXT));
    }


    return instance;
}
#pragma endregion

#pragma region Create: PhysDevice, Device, Queue


static VkPhysicalDevice _PickPhysicalDevice(
    vk::Instance inst,
    vk::PhysicalDeviceProperties* out_pProps, 
    vk::PhysicalDeviceFeatures* out_pFeats,
    vk::PhysicalDeviceMemoryProperties* out_pMemProps)
{
    std::vector<vk::PhysicalDevice> gpus = inst.enumeratePhysicalDevices();
    vk::PhysicalDevice gpu = gpus.front();

    *out_pProps = gpu.getProperties();
    *out_pFeats = gpu.getFeatures();
    *out_pMemProps = gpu.getMemoryProperties();
    return gpu;
}



static vkx::QueueFamilyIndices _FindQueueFamilies(vk::PhysicalDevice& physDevice, vk::SurfaceKHR& surfaceKHR)
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = physDevice.getQueueFamilyProperties();

    vkx::QueueFamilyIndices queueFamilyIdxs;

    // todo: Find the BestFit queue for specific QueueFlag. https://stackoverflow.com/a/57210037
    int i = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIdxs.GraphicsFamily = i;
        }

        uint32_t supportPresent = physDevice.getSurfaceSupportKHR(i, surfaceKHR);
        if (supportPresent) {
            queueFamilyIdxs.PresentFamily = i;
        }

        if (queueFamilyIdxs.isComplete())
            break;
        i++;
    }

    return queueFamilyIdxs;
}

static vk::Device _CreateLogicalDevice(
    const vk::PhysicalDevice& physDevice,
    const vkx::QueueFamilyIndices& queueFamily,
    vk::Queue* out_GraphicsQueue,
    vk::Queue* out_PresentQueue)
{
    vk::DeviceCreateInfo deviceInfo{};

    // Queue Family
    float queuePriority = 1.0f;  // 0.0-1.0

    std::set<uint32_t> uniqQueueFamilyIdx = { queueFamily.GraphicsFamily, queueFamily.PresentFamily };
    std::vector<vk::DeviceQueueCreateInfo> arrQueueCreateInfo;
    for (uint32_t queueFamilyIdx : uniqQueueFamilyIdx) {
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfo.queueFamilyIndex = queueFamilyIdx;
        queueCreateInfo.queueCount = 1;
        arrQueueCreateInfo.push_back(queueCreateInfo);
    }

    deviceInfo.pQueueCreateInfos = arrQueueCreateInfo.data();
    deviceInfo.queueCreateInfoCount = arrQueueCreateInfo.size();

    // Device Features
    vk::PhysicalDeviceFeatures deviceFeatures = vkx::ctx().PhysDeviceFeatures;
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceInfo.pEnabledFeatures = &deviceFeatures;

    // Device Extensions  (needs check is supported?)
    std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
            "VK_KHR_portability_subset"
#endif
    };

    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceInfo.enabledExtensionCount = deviceExtensions.size();

    vk::Device device = physDevice.createDevice(deviceInfo, vkx::ctx().Allocator);

    *out_GraphicsQueue = device.getQueue(queueFamily.GraphicsFamily, 0);
    *out_PresentQueue  = device.getQueue(queueFamily.PresentFamily,  0);
    return device;
}

#pragma endregion

#pragma region Create: CommandPool, DescriptorPool, SyncObjects(Semaphores, Fences), Main-RenderPass, etc.

static vk::CommandPool _CreateCommandPool(
    uint32_t queueFamily)
{
    DECL_CTX_device_allocator;

    return device.createCommandPool(
        vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = queueFamily
        }, allocator);
}

static vk::DescriptorPool _CreateDescriptorPool()
{
    DECL_CTX_device_allocator;

    // tho kinda oversize.
    vk::DescriptorPoolSize pool_sizes[] =
    {
            { vk::DescriptorType::eSampler,              1000 },
            { vk::DescriptorType::eCombinedImageSampler, 1000 },
            { vk::DescriptorType::eSampledImage,         1000 },
            { vk::DescriptorType::eStorageImage,         1000 },
            { vk::DescriptorType::eUniformTexelBuffer,   1000 },
            { vk::DescriptorType::eStorageTexelBuffer,   1000 },
            { vk::DescriptorType::eUniformBuffer,        1000 },
            { vk::DescriptorType::eStorageBuffer,        1000 },
            { vk::DescriptorType::eUniformBufferDynamic, 1000 },
            { vk::DescriptorType::eStorageBufferDynamic, 1000 },
            { vk::DescriptorType::eInputAttachment,      1000 }
    };

    return device.createDescriptorPool(
        vk::DescriptorPoolCreateInfo{
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = 1000,
            .poolSizeCount = std::size(pool_sizes),
            .pPoolSizes = pool_sizes
        }, allocator);
}

static void _CreateSyncObjects()
{
    DECL_CTX_device_allocator;

    vk::SemaphoreCreateInfo semaphoreInfo{};

    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    auto& g = vkx::ctx();
    for (int i = 0; i < g.InflightFrames; ++i)
    {
        g.SemaphoreImageAcquired[i]  = device.createSemaphore(semaphoreInfo, allocator);
        g.SemaphoreRenderComplete[i] = device.createSemaphore(semaphoreInfo, allocator);
        g.CommandBufferFences[i] = device.createFence(fenceInfo, allocator);
    }
}


static vk::RenderPass _CreateMainRenderPass(
    vk::Format imageFormat  = vkx::ctx().SwapchainSurfaceFormat.format,
    vk::Format depthFormat  = vkx::ctx().SwapchainDepthImage.format)
{
    vk::SubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    vk::AttachmentReference colorAttachmentRef = { 0, vk::ImageLayout::eColorAttachmentOptimal };
    vk::AttachmentReference depthAttachmentRef = { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

    return vkx::CreateRenderPass(
        {{
            vkx::IAttachmentDesc(imageFormat, vk::ImageLayout::ePresentSrcKHR),
            vkx::IAttachmentDesc(depthFormat, vk::ImageLayout::eDepthStencilAttachmentOptimal)
        }},
        vkx::IGraphicsSubpass(
            vkx::IAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal),           // Color
            vkx::IAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)),   // Depth
        dependency);
}

#pragma endregion

#pragma region Create: Swapchain


static void _CreateSwapchain(
    const vk::PhysicalDevice&	physDevice                  = vkx::ctx().PhysDevice,
    const vk::SurfaceKHR&		surfaceKHR                  = vkx::ctx().SurfaceKHR,
    const vkx::QueueFamilyIndices& queueFamily              = vkx::ctx().QueueFamily,
    vk::RenderPass              renderPass                  = vkx::ctx().MainRenderPass,
    vk::SurfaceFormatKHR        surfaceFormat               = vkx::ctx().SwapchainSurfaceFormat,
    vk::SwapchainKHR&		    out_SwapchainKHR            = vkx::ctx().SwapchainKHR,
    vk::Extent2D&				out_SwapchainExtent         = vkx::ctx().SwapchainExtent,
    std::vector<vk::Image>&		out_SwapchainImages         = vkx::ctx().SwapchainImages,
    std::vector<vk::ImageView>&	out_SwapchainImageViews     = vkx::ctx().SwapchainImageViews,
    vkx::Image&                 out_SwapchainDepthImage     = vkx::ctx().SwapchainDepthImage,
    std::vector<vk::Framebuffer>& out_SwapchainFramebuffers = vkx::ctx().SwapchainFramebuffers)
{
    DECL_CTX_device_allocator;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physDevice.getSurfaceCapabilitiesKHR(surfaceKHR);

    out_SwapchainExtent = surfaceCapabilities.currentExtent;
    assert(surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max(), "VkxError: invalid VkSurfaceCapabilitiesKHR.currentExtent.width");
    
    vk::PresentModeKHR surfacePresentMode = vk::PresentModeKHR::eFifo;  // FIFO is vk guaranteed available.
    {
        for (auto mode : physDevice.getSurfacePresentModesKHR(surfaceKHR)) {
            if (mode == vk::PresentModeKHR::eMailbox) // MAILBOX avoid tearing while still maintaining a fairly low latency by rendering new images that are as up-to-date as possible right until the vertical blank.
                surfacePresentMode = vk::PresentModeKHR::eMailbox;
        }
    }

    uint32_t swapchainImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && swapchainImageCount > surfaceCapabilities.maxImageCount) {
        swapchainImageCount = surfaceCapabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.surface = surfaceKHR;
    swapchainInfo.minImageCount = swapchainImageCount;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = out_SwapchainExtent;
    swapchainInfo.imageArrayLayers = 1;  // normally 1, except VR.
    swapchainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    // Image Share Mode. Queue Family.
    uint32_t queueFamilyIdxs[] = { queueFamily.GraphicsFamily, queueFamily.PresentFamily };
    if (queueFamily.GraphicsFamily != queueFamily.PresentFamily) {
        swapchainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIdxs;
    } else {
        swapchainInfo.imageSharingMode = vk::SharingMode::eExclusive;  // most performance.
    }

    swapchainInfo.preTransform = surfaceCapabilities.currentTransform;  // Non transform
    swapchainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;  // Non Alpha.
    swapchainInfo.presentMode = surfacePresentMode;
    swapchainInfo.clipped = VK_TRUE;  // true: not care the pixels behind other windows for vk optims.  but may cause problem when we read the pixels.
    swapchainInfo.oldSwapchain = nullptr;

    out_SwapchainKHR = device.createSwapchainKHR(swapchainInfo, allocator);

    // Get Swapchain Images.
    out_SwapchainImages = device.getSwapchainImagesKHR(out_SwapchainKHR);  // ?ImageAllocator
    assert(swapchainImageCount == out_SwapchainImages.size());

    // Swapchain ImageViews
    out_SwapchainImageViews.resize(swapchainImageCount);
    for (int i = 0; i < swapchainImageCount; ++i)
    {
        out_SwapchainImageViews[i] = vkx::CreateImageView(out_SwapchainImages[i], surfaceFormat.format);
    }

    // Swapchain Depth Image
    out_SwapchainDepthImage = vkx::CreateDepthImage(out_SwapchainExtent.width, out_SwapchainExtent.height);

    // Swapchain Framebuffers
    out_SwapchainFramebuffers.resize(out_SwapchainImageViews.size());
    for (size_t i = 0; i < out_SwapchainImageViews.size(); i++)
    {
        out_SwapchainFramebuffers[i] = vkx::CreateFramebuffer(out_SwapchainExtent, renderPass,
            { { out_SwapchainImageViews[i], out_SwapchainDepthImage.imageView } });
    }
}

static void _DestroySwapchain(
    const std::vector<vk::Framebuffer>& swapchainFramebuffers = vkx::ctx().SwapchainFramebuffers,
    const std::vector<vk::ImageView>& swapchainImageViews = vkx::ctx().SwapchainImageViews,
    vkx::Image& swapchainDepthImage = vkx::ctx().SwapchainDepthImage,
    vk::SwapchainKHR swapchainKHR = vkx::ctx().SwapchainKHR)
{
    DECL_CTX_device_allocator;

    //delete swapchainDepthImage;

    for (auto fb : swapchainFramebuffers) {
        vkDestroyFramebuffer(device, fb, nullptr);
    }
    for (auto imageview : swapchainImageViews) {
        vkDestroyImageView(device, imageview, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchainKHR, nullptr);
}

void vkx::RecreateSwapchain(bool onlyCreate)
{
    if (!onlyCreate)
    {
        vkDeviceWaitIdle(vkx::ctx().Device);
        _DestroySwapchain();
    }

    _CreateSwapchain();
    //_CreateSwapchainDepthImage();
    //_CreateSwapchainFramebuffers();
}



#pragma endregion


#pragma region vkx::Init(), vkx::Destroy()


vkx::InstanceContext& vkx::ctx()
{
    static vkx::InstanceContext _ctx{};
    return _ctx;
}

void vkx::Init(
    const void* surfaceWindowHandle,
    bool enableValidationLayer)
{
    auto& i = vkx::ctx();

    i.Instance = 
    _CreateInstance(enableValidationLayer, vkx::_Glfw_GetRequiredInstanceExtensions());
    i.SurfaceKHR =
    vkx::_Glfw_CreateSurfaceKHR(i.Instance, surfaceWindowHandle);

    i.PhysDevice =
    _PickPhysicalDevice(i.Instance, &i.PhysDeviceProperties, &i.PhysDeviceFeatures, &i.PhysDeviceMemoryProperties);

    i.QueueFamily = _FindQueueFamilies(i.PhysDevice, i.SurfaceKHR);
    i.Device =
    _CreateLogicalDevice(i.PhysDevice, i.QueueFamily, &i.GraphicsQueue, &i.PresentQueue);

    i.CommandPool =
    _CreateCommandPool(i.QueueFamily.GraphicsFamily);
    i.DescriptorPool =
    _CreateDescriptorPool();

    i.ImageSampler = vkx::CreateImageSampler();
    _CreateSyncObjects();
    vkx::AllocateCommandBuffers(i.InflightFrames, i.CommandBuffers, vk::CommandBufferLevel::ePrimary);

    // dependent by CreateMainRenderPass
    i.SwapchainSurfaceFormat = i.PhysDevice.getSurfaceFormatsKHR(i.SurfaceKHR).front();  // expected: {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
    i.SwapchainDepthImageFormat = _FindDepthFormat();

    // dependent by CreateSwapchinFramebuffers
    i.MainRenderPass = _CreateMainRenderPass();

    // Create Swapchain
    vkx::RecreateSwapchain(true);
}

void vkx::Destroy()
{

}

#pragma endregion






#pragma region GLFW external

#if VKX_BACKEND == VKX_BACKEND_eGLFW

#include <GLFW/glfw3.h>

std::vector<const char*> vkx::_Glfw_GetRequiredInstanceExtensions()
{
    uint32_t glfwRequiredInstExtensionsCount = 0;
    const char** glfwRequiredInstExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredInstExtensionsCount);
    
    std::vector<const char*> extensions;
    for (int i = 0; i < glfwRequiredInstExtensionsCount; ++i) 
    {
        extensions.push_back(glfwRequiredInstExtensions[i]);
    }
    return extensions;
}

vk::SurfaceKHR vkx::_Glfw_CreateSurfaceKHR(vk::Instance inst, const void* wnd)
{
    VkSurfaceKHR surfaceKHR;
    glfwCreateWindowSurface(inst, (GLFWwindow*)wnd, (VkAllocationCallbacks*)vkx::ctx().Allocator, &surfaceKHR);
    return surfaceKHR;
}

#endif // VKX_BACKEND == VKX_BACKEND_eGLFW

#pragma endregion