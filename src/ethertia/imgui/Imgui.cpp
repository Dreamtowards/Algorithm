
#include "Imgui.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <vkx/vkx.hpp>

#include <ethertia/render/Window.h>
#include <ethertia/util/BenchmarkTimer.h>
#include <ethertia/util/Log.h>

static void ImplImgui_VulkanCheckResult(VkResult r)
{
    if (r != VK_SUCCESS) {
        Log::warn("ImGui Vulkan Error");
    }
    vkx::check(r);
}

static void InitForVulkan()
{
    ImGui_ImplGlfw_InitForVulkan(Window::Handle(), true);

    VKX_CTX_device_allocator;
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = vkxc.Instance;
    initInfo.PhysicalDevice = vkxc.PhysDevice;
    initInfo.Device = vkxc.Device;
    initInfo.QueueFamily = vkxc.QueueFamily.GraphicsFamily;
    initInfo.Queue = vkxc.GraphicsQueue;
    initInfo.PipelineCache = nullptr;
    initInfo.DescriptorPool = vkxc.DescriptorPool;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = vkxc.SwapchainImages.size();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = (VkAllocationCallbacks*)vkxc.Allocator;
    initInfo.CheckVkResultFn = ImplImgui_VulkanCheckResult;

    // Dynamic Rendering, VK_KHR_dynamic_rendering
    initInfo.UseDynamicRendering = false;
    //initInfo.ColorAttachmentFormat = ;

    ImGui_ImplVulkan_Init(&initInfo, vkxc.MainRenderPass);

    vkx::SubmitCommandBuffer([](vk::CommandBuffer cmdbuf) 
    {
        ImGui_ImplVulkan_CreateFontsTexture(cmdbuf);
    });
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}


void Imgui::Init()
{
    BENCHMARK_TIMER;
    Log::info("Init ImGui.. \1");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    // InitStyle();


    ImGuiIO& io = ImGui::GetIO();

    // Set Before Backend/Impl Init.
    io.ConfigFlags |=
        ImGuiConfigFlags_DockingEnable |    // Enable Docking.
        ImGuiConfigFlags_ViewportsEnable;   // Multiple Windows/Viewports
        ImGuiConfigFlags_DpiEnableScaleFonts |
        ImGuiConfigFlags_DpiEnableScaleViewports;
    //io.ConfigViewportsNoDecoration = false;
    //io.ConfigViewportsNoTaskBarIcon = true;
    //ImGui::GetMainViewport()->DpiScale = 4;

    InitForVulkan();



    // ImNodes
    //ImNodes::CreateContext();
    //ImNodes::GetIO().EmulateThreeButtonMouse.Modifier = &ImGui::GetIO().KeyShift;

}

void Imgui::Destroy()
{
    //ImNodes::DestroyContext();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Imgui::NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //ImGui_ImplGlfw_MousePosWindowScale = 1.0f / Imgui::GlobalScale;
    //ImGui::GetMainViewport()->Size /= Imgui::GlobalScale;
    //ImGui::GetIO().DisplayFramebufferScale *= Imgui::GlobalScale;
}

void Imgui::Render(vk::CommandBuffer cmdbuf)
{
    {
        //PROFILE("Render");

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdbuf);
    }

    // Update Multiple Windows/Viewports
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        //GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        //glfwMakeContextCurrent(backup_current_context);
    }
}

