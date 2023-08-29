
#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS  // use Designated Initializers
//#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>


#define slice_t vk::ArrayProxy

#define VKX_BACKEND_eGLFW 1
#define VKX_BACKEND VKX_BACKEND_eGLFW

#include <functional>


namespace vkx
{
	

	class Image
	{
	public:
		vk::Image image;
		vk::DeviceMemory imageMemory;
		vk::ImageView imageView;

		vk::Format format;
		int width;
		int height;


		Image() {}

		Image(vk::Image image, vk::DeviceMemory imageMemory, vk::Format format, int width, int height, vk::ImageView imageView) :
			image(image), imageMemory(imageMemory), format(format), width(width), height(height), imageView(imageView) {}

	};

	void check(const vk::Result& r);
	void check(const VkResult& r);

	template<typename T>
	T& check(const vk::ResultValue<T>& r)
	{
		vkx::check(r.result);
		return r.value;
	}

	#pragma region ctx

	struct QueueFamilyIndices
	{
		uint32_t GraphicsFamily = -1;
		uint32_t PresentFamily = -1;  //Surface Present.

		bool isComplete() const { return GraphicsFamily != -1 && PresentFamily != -1; }
	};

	class InstanceContext
	{
	public:
		vk::AllocationCallbacks* Allocator = nullptr;

		vk::Instance Instance;
		vk::SurfaceKHR SurfaceKHR;

		vk::PhysicalDevice PhysDevice;
		vk::PhysicalDeviceProperties	PhysDeviceProperties;
		vk::PhysicalDeviceFeatures		PhysDeviceFeatures;
		vk::PhysicalDeviceMemoryProperties PhysDeviceMemoryProperties;

		vk::Device Device;
		vk::Queue GraphicsQueue;
		vk::Queue PresentQueue;
		vkx::QueueFamilyIndices QueueFamily;

		vk::CommandPool CommandPool;
		vk::DescriptorPool DescriptorPool;
		
		vk::SwapchainKHR			SwapchainKHR;
		vk::Extent2D				SwapchainExtent;
		std::vector<vk::Image>		SwapchainImages;
		std::vector<vk::ImageView>	SwapchainImageViews;
		std::vector<vk::Framebuffer>SwapchainFramebuffers;
		vkx::Image 					SwapchainDepthImage;
		vk::SurfaceFormatKHR		SwapchainSurfaceFormat;  // dependnt by CreateSwapchain and CreateMainRenderPass
		vk::Format 					SwapchainDepthImageFormat;

		vk::RenderPass				MainRenderPass;

		vk::Sampler ImageSampler;  // default sampler. VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT

		static const uint32_t InflightFrames = 3;  // Frames InFlights
		vk::CommandBuffer CommandBuffers[InflightFrames];
		vk::Fence CommandBufferFences[InflightFrames];

		vk::Semaphore SemaphoreImageAcquired[InflightFrames];
		vk::Semaphore SemaphoreRenderComplete[InflightFrames];

		std::function<void(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
		)> DebugMessengerCallback = [](auto, auto, auto) {};
	};

	// Instance Context. available once vkx::Init() initialized.
	vkx::InstanceContext& ctx();

	std::vector<const char*>	_Glfw_GetRequiredInstanceExtensions();
	vk::SurfaceKHR				_Glfw_CreateSurfaceKHR(vk::Instance inst, const void* wnd);

	void Init(
		const void* surfaceWindowHandle,
		bool enableValidationLayer = true);

	void Destroy();


	void RecreateSwapchain(bool onlyCreate = false);

	#pragma endregion






	#pragma region Memory

	vk::DeviceMemory AllocateMemory(
		vk::MemoryRequirements memRequirements,  // size, alignment, memoryType
		vk::MemoryPropertyFlags memProperties);


	#pragma endregion


	#pragma region CommandBuffer, QueueSubmit

	void AllocateCommandBuffers(
		int numCmdbuf,
		vk::CommandBuffer* pCmdbufs,
		vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary,
		vk::CommandPool commandPool = vkx::ctx().CommandPool,
		vk::Device device = vkx::ctx().Device);


	void BeginCommandBuffer(
		vk::CommandBuffer cmdbuf,
		vk::CommandBufferUsageFlags usageFlags);


	// Allocate & Record & Submit Onetime CommandBuffer
	void SubmitCommandBuffer(
		const std::function<void(vk::CommandBuffer)>& fn_record,
		vk::Queue queue = vkx::ctx().GraphicsQueue,
		vk::CommandPool commandPool = vkx::ctx().CommandPool,
		vk::Device device = vkx::ctx().Device);

	void QueueSubmit(
		vk::Queue queue,
		std::span<const vk::CommandBuffer> cmdbufs,
		std::span<const vk::Semaphore> waits = {},
		vk::PipelineStageFlags* waitStages = nullptr,  // vk::PipelineStageFlagBits::eColorAttachmentOutput
		std::span<const vk::Semaphore> signals = {},
		vk::Fence fence = {});

	#pragma endregion


	#pragma region Image, ImageView.

	void CreateImage(
		int width, int height,
		vk::Image* pImage,  // out
		vk::DeviceMemory* pImageMemory,  // out
		vk::Format format = vk::Format::eB8G8R8A8Unorm,  // ?UNorm SRGB
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlags memProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
		bool creatingCubeMap = false);

	vkx::Image CreateDepthImage(int width, int height);

	vk::ImageView CreateImageView(
		vk::Image image,
		vk::Format format = vk::Format::eR8G8B8A8Unorm,  // ?? SRGB or UNORM
		vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType imageViewType = vk::ImageViewType::e2D);

	vk::Sampler CreateImageSampler(
		vk::Filter magFilter = vk::Filter::eNearest,
		vk::Filter minFilter = vk::Filter::eNearest,
		vk::SamplerAddressMode addressModeUVW = vk::SamplerAddressMode::eRepeat);

	#pragma endregion


	#pragma region RenderPass, Framebuffer

	vk::RenderPass CreateRenderPass(
		slice_t<const vk::AttachmentDescription> attachments,
		slice_t<const vk::SubpassDescription> subpasses,
		slice_t<const vk::SubpassDependency> dependencies = {});

	vk::AttachmentDescription IAttachmentDesc(
		vk::Format format,
		vk::ImageLayout finalLayout);

	vk::AttachmentReference IAttachmentRef(
		uint32_t attachment,
		vk::ImageLayout layout);

	vk::SubpassDescription IGraphicsSubpass(
		slice_t<const vk::AttachmentReference> colorAttachmentRefs,
		const vk::AttachmentReference& depthStencilAttachment = {});

	vk::SubpassDependency ISubpassDependency(
		uint32_t srcSubpass, uint32_t dstSubpass,
		vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
		vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
		vk::DependencyFlags dependencyFlags = {});


	vk::Framebuffer CreateFramebuffer(
		vk::Extent2D wh,
		vk::RenderPass renderPass,
		slice_t<const vk::ImageView> attachments);

	#pragma endregion

}