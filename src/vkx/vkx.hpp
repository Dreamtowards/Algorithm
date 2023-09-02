
#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS  // use Designated Initializers
//#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>


#define vkx_slice_t vk::ArrayProxy
#define vkx_optional_t vk::Optional

#define VKX_CTX_device vk::Device& device = vkx::ctx().Device;
#define VKX_CTX_device_allocator const vk::Device& device = vkx::ctx().Device; \
    const vk::AllocationCallbacks* allocator = vkx::ctx().Allocator; \
    auto& vkxc = vkx::ctx();

#define VKX_BACKEND_eGLFW 1
#define VKX_BACKEND VKX_BACKEND_eGLFW


#include <functional>
#include <iostream>   // for default DebugMessengerCallback impl.


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


		Image(vk::Image image = {}, vk::DeviceMemory imageMemory = {}, vk::Format format = {}, int width = 0, int height = 0, vk::ImageView imageView = {}) :
			image(image), imageMemory(imageMemory), format(format), width(width), height(height), imageView(imageView) {}

	};

	const vk::Result&	check(const vk::Result& r);
	const VkResult&		check(const VkResult& r);

	template<typename T>
	const T& check(const vk::ResultValue<T>& r)
	{
		vkx::check(r.result);
		return r.value;
	}

	uint32_t FormatSize(vk::Format fmt);

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

		uint32_t CurrentSwapchainImage = 0;

		const uint32_t InflightFrames = 3;  // Frames InFlights
		uint32_t CurrentInflightFrame = 0;

		std::vector<vk::CommandBuffer>	CommandBuffers;
		std::vector<vk::Fence>			CommandBufferFences;
		std::vector<vk::Semaphore> SemaphoreImageAcquired;
		std::vector<vk::Semaphore> SemaphoreRenderComplete;

		std::function<void(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
		)> DebugMessengerCallback = [](auto messageSeverity, auto messageType, auto pCallbackData)
			{
				const char* sSERV = "VERBO";
				if (messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo) sSERV = "INFO";
				else if (messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) sSERV = "WARN";
				else if (messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) sSERV = "ERROR";

				const char* sTYPE = "GENERAL";
				if (messageType == vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation) sTYPE = "VALIDATION";
				else if (messageType == vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance) sTYPE = "PERFORMANCE";
				else if (messageType == vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding) sTYPE = "DeviceAddressBinding";

				if (messageSeverity != vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose &&
					messageType		!= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
				{
					std::cerr << "\n============================ VkMSG ["<<sSERV<<"] ["<<sTYPE<<"] @"<<pCallbackData->pMessageIdName<<" ============================\n";
					std::cerr << "VkDebugMessenger[" << sSERV << "][" << sTYPE << "]: " << pCallbackData->pMessage << "\n";
					std::cerr <<   "===============================================================\n\n";
					std::cerr.flush();
					if (messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
					{
						throw 4;
					}
				}
			};
	};

	// Instance Context. available once vkx::Init() initialized.
	vkx::InstanceContext& ctx();

	std::vector<const char*>	_Glfw_GetRequiredInstanceExtensions();
	vk::SurfaceKHR				_Glfw_CreateSurfaceKHR(vk::Instance inst, const void* wnd);

	void Init(
		const void* surfaceWindowHandle,
		bool enableValidationLayer = true);

	void Destroy();


	void RecreateSwapchain(bool destroy = true, bool create = true);

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

	// ICmd
	vk::CommandBufferBeginInfo ICommandBufferBegin(
		vk::CommandBufferUsageFlags usageFlags);


	// Allocate & Record & Submit Onetime CommandBuffer
	void SubmitCommandBuffer(
		const std::function<void(vk::CommandBuffer)>& fn_record,
		vk::Queue queue = vkx::ctx().GraphicsQueue,
		vk::CommandPool commandPool = vkx::ctx().CommandPool,
		vk::Device device = vkx::ctx().Device);

	void QueueSubmit(
		vk::Queue queue,
		vkx_slice_t<vk::CommandBuffer> cmdbufs,
		vkx_slice_t<vk::Semaphore> waits = {},
		vkx_slice_t<vk::PipelineStageFlags> waitStages = {},  // vk::PipelineStageFlagBits::eColorAttachmentOutput
		vkx_slice_t<vk::Semaphore> signals = {},
		vk::Fence fence = {});

	vk::Result QueuePresentKHR(
		vk::Queue presentQueue,
		vkx_slice_t<vk::Semaphore> waitSemaphores,
		vkx_slice_t<vk::SwapchainKHR> swapchains,
		vkx_slice_t<uint32_t> imageIndices);

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
		vkx_slice_t<const vk::AttachmentDescription> attachments,
		vkx_slice_t<const vk::SubpassDescription> subpasses,
		vkx_slice_t<const vk::SubpassDependency> dependencies = {});

	vk::AttachmentDescription IAttachmentDesc(
		vk::Format format,
		vk::ImageLayout finalLayout);

	vk::AttachmentReference IAttachmentRef(
		uint32_t attachment,
		vk::ImageLayout layout);

	vk::SubpassDescription IGraphicsSubpass(
		vkx_slice_t<const vk::AttachmentReference> colorAttachmentRefs,
		const vk::AttachmentReference& depthStencilAttachment = {});

	vk::SubpassDependency ISubpassDependency(
		uint32_t srcSubpass, uint32_t dstSubpass,
		vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
		vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
		vk::DependencyFlags dependencyFlags = {});

	// ICmd
	vk::RenderPassBeginInfo IRenderPassBegin(
		vk::RenderPass renderPass,
		vk::Framebuffer framebuffer,
		vk::Extent2D renderAreaExtent,
		vkx_slice_t<vk::ClearValue> clearValues);

	vk::ClearValue ClearValueColor(float r = 0, float g = 0, float b = 0, float a = 1);

	vk::ClearValue ClearValueDepthStencil(float depth = 1.0f, uint32_t stencil = 0);

	vk::Framebuffer CreateFramebuffer(
		vk::Extent2D wh,
		vk::RenderPass renderPass,
		vkx_slice_t<const vk::ImageView> attachments);

	#pragma endregion


#pragma region Graphics Pipeline


	vk::PipelineColorBlendAttachmentState IPipelineColorBlendAttachment(
		bool blendEnable = true,
		vk::BlendFactor srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		vk::BlendFactor dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		vk::BlendOp colorBlendOp = vk::BlendOp::eAdd,
		vk::BlendFactor srcAlphaBlendFactor = vk::BlendFactor::eOne,
		vk::BlendFactor dstAlphaBlendFactor = vk::BlendFactor::eZero,
		vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd,
		vk::ColorComponentFlags colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);


	struct FnArg_CreateGraphicsPipeline
	{
		vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
		std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	};

	vk::Pipeline CreateGraphicsPipeline(
		vkx_slice_t<std::pair<std::span<const char>, vk::ShaderStageFlagBits>> shaderStageSources,
		std::initializer_list<vk::Format> vertexInputAttribFormats,
		vk::PipelineLayout pipelineLayout,
		FnArg_CreateGraphicsPipeline args = {},
		vk::RenderPass renderPass = {},
		uint32_t subpass = 0);

	vk::PipelineLayout CreatePipelineLayout(
		vkx_slice_t<vk::DescriptorSetLayout> setLayouts,
		vkx_slice_t<vk::PushConstantRange> pushConstantRanges = {});

#pragma endregion

}