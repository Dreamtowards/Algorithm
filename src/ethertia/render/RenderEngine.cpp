//
// Created by Dreamtowards on 2022/8/22.
//


#include "RenderEngine.h"

#include <vkx/vkx.hpp>

#include <ethertia/render/Window.h>
#include <ethertia/render/VertexData.h>
#include <ethertia/imgui/Imgui.h>
#include <ethertia/util/BenchmarkTimer.h>
#include <ethertia/util/Log.h>
#include <ethertia/util/Loader.h>

static vkx::GraphicsPipeline* g_Pipeline;

static vkx::VertexBuffer* g_VBuffer;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct UBO_A
{
    glm::mat4 matProjection;
    glm::mat4 matView;
    glm::mat4 matModel;
    float Time;
};

struct PC_A
{
    glm::mat4 matModel;
};
static std::vector<vkx::UniformBuffer*> g_ubos;

static vkx::Image* g_Tex;

#define VKX_LIST_CREATE(ls, n, init) ls.resize(n); for (int _i = 0; _i < n; ++_i) { ls[_i] = init; }
#define VKX_LIST_DELETE(ls) for (int _i = 0; _i < ls.size(); ++_i) { delete ls[_i]; }



void RenderEngine::Init()
{
    BENCHMARK_TIMER;
    Log::info("RenderEngine initializing..");


    vkx::Init(Window::Handle(), true);

    uint32_t vkApiVersion = vkx::ctx().PhysDeviceProperties.apiVersion;
    Log::info("vulkan {}.{}.{}, {}",
        VK_API_VERSION_MAJOR(vkApiVersion), VK_API_VERSION_MINOR(vkApiVersion), VK_API_VERSION_PATCH(vkApiVersion),
        (const char*)vkx::ctx().PhysDeviceProperties.deviceName);


    VKX_CTX_device_allocator;


    g_Tex = Loader::LoadImage("entity/gravestone/diff.png");


    g_VBuffer = Loader::LoadVertexData(Loader::LoadOBJ("entity/gravestone/mesh.obj"));


    int fif = vkxc.InflightFrames;

    VKX_LIST_CREATE(g_ubos, fif, vkx::CreateUniformBuffer(sizeof(UBO_A)));


    g_Pipeline = vkx::CreateGraphicsPipeline(
        {
            {Loader::LoadFile("./shaders/test/vert.spv"), vk::ShaderStageFlagBits::eVertex},
            {Loader::LoadFile("./shaders/test/frag.spv"), vk::ShaderStageFlagBits::eFragment}
        },
        {
           vk::Format::eR32G32B32Sfloat,
           vk::Format::eR32G32Sfloat,
           vk::Format::eR32G32B32Sfloat
        },
        vkx::CreateDescriptorSetLayout(
        {
            {vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex},
            {vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}
        }),
        {},
        vkx::ctx().MainRenderPass, 0);

    for (size_t i = 0; i < fif; i++)
    {
        vkx::WriteDescriptorSet(g_Pipeline->DescriptorSets[i],
        {
            {.buffer = vkx::IDescriptorBuffer(g_ubos[i]) },
            {.image  = vkx::IDescriptorImage(g_Tex->imageView) }
        });
    }

    Imgui::Init();

    //VertexData vtx;
    //vtx.addVertex({ {-0.5, -0.5, 0}, {0, 1}, {} });
    //vtx.addVertex({ { 0.5, -0.5, 0}, {1, 1}, {} });
    //vtx.addVertex({ { 0.5,  0.5, 0}, {1, 0}, {} });
    //vtx.addVertex({ {-0.5,  0.5, 0}, {0, 0}, {} });
    //
    //vtx.Indices.push_back(0);
    //vtx.Indices.push_back(1);
    //vtx.Indices.push_back(2);
    //vtx.Indices.push_back(0);
    //vtx.Indices.push_back(2);
    //vtx.Indices.push_back(3);
    
    //g_VBuffer = Loader::LoadVertexData(&vtx);



    //TEX_WHITE = Loader::loadTexture(BitmapImage(1, 1, new uint32_t[1]{(uint32_t)~0}));
    //TEX_UVMAP = Loader::loadTexture("misc/uvmap.png");

    //Materials::registerMaterialItems();  // before items tex load.
    //MaterialTextures::load();
    //RendererGbuffer::init();
    //
    //RendererCompose::init(RendererGbuffer::gPosition.Image->m_ImageView,
    //                      RendererGbuffer::gNormal.Image->m_ImageView,
    //                      RendererGbuffer::gAlbedo.Image->m_ImageView);
    //
    //g_ComposeView = RendererCompose::g_FramebufferAttachmentColor.Image->m_ImageView;
            // RendererGbuffer::gAlbedo.Image->m_ImageView;

    Log::info("RenderEngine initialized.\1");
}


void RenderEngine::Destroy()
{
    VKX_CTX_device_allocator;

    device.waitIdle();

    delete g_Pipeline;

    //MaterialTextures::clean();
    //ItemTextures::clean();
    //
    //RendererGbuffer::deinit();
    //
    //delete TEX_WHITE;
    //delete TEX_UVMAP;

    VKX_LIST_DELETE(g_ubos);

    delete g_VBuffer;
    delete g_Tex;

    Imgui::Destroy();

    vkx::Destroy();
}
static glm::vec3 CamPos{0, 0, 10};

static glm::mat4 matModel(glm::vec3 position, glm::mat3 rotation, glm::vec3 scale) {
    glm::mat4 mat{ 1 };

    mat = glm::translate(mat, position);

    mat = mat * glm::mat4(rotation);

    mat = glm::scale(mat, scale);

    return mat;
}

class Camera
{
public:

    glm::vec3 position;

    glm::vec3 eulerangles;

    // readonly
    glm::vec3 direction;

    glm::vec3 pivot;
    float PivotDistance = 0;

    float fov = glm::radians(80.0f);
    float RatioAspect = 1.0f;
    float NearPlane = 0.1f;
    float FarPlane = 10000.0f;

    glm::mat4 matProjection;
    glm::mat4 matView;

    void UpdateMatrix()
    {
        float pitch = eulerangles.x;
        direction.y = sin(pitch);

        float yaw = eulerangles.y;
        direction.x = cos(yaw); // Note that we convert the angle to radians first
        direction.z = sin(yaw);

        matProjection = glm::perspective(fov, RatioAspect, NearPlane, FarPlane);
        matView   = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    }
};
static Camera g_MainCamera;

#include <ethertia/imgui/ImWindows.h>

void ShowSomeWindow(bool* show)
{
    ImGui::Begin("TmpDebug", show);
    Camera& cam = g_MainCamera;
    ImGui::DragFloat3("Position", &cam.position.x);
    ImGui::DragFloat3("EulerAngles\n(Pitch, Yaw, Roll)", &cam.eulerangles.x, 0.05);
    ImGui::DragFloat("Fov", &cam.fov, 0.05);
    ImGui::End();
}

void RenderEngine::Render()
{
    Imgui::NewFrame();


    {
        ImWindows::ShowWindows();

        ImGui::Begin("TmpDebug222");
        if (ImGui::Button("ShowTheWiin"))
        {
            ImWindows::Show(ShowSomeWindow);
        }
        ImGui::End();



    }

    if (Window::IsFramebufferResized())
        vkx::RecreateSwapchain();  // fix: Minimize size0

    VKX_CTX_device_allocator;

    int fif_i = vkxc.CurrentInflightFrame;
    vkx::CommandBuffer cmd{ vkxc.CommandBuffers[fif_i] };

    {

        float t = Window::PreciseTime();

        glm::vec2 dMouse = Window::MouseDelta();
        //g_MainCamera.eulerangles.x += dMouse.y;
        //g_MainCamera.eulerangles.y += dMouse.x;

        g_MainCamera.UpdateMatrix();

        UBO_A ubo;
        ubo.matProjection = g_MainCamera.matProjection;// glm::perspective(glm::radians(80.0f), vkxc.SwapchainExtent.width / (float)vkxc.SwapchainExtent.height, 0.1f, 1000.0f);
        ubo.matView = g_MainCamera.matView;// glm::lookAt(CamPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.matModel = matModel({}, glm::mat3(1.0), { 1,1,1 });
        ubo.Time = t;

        g_ubos[fif_i]->Upload(&ubo);
    }

    {
        // blocking until the CommandBuffer has finished executing
        VKX_CHECK(device.waitForFences(vkxc.CommandBufferFences[fif_i], true, UINT64_MAX));

        // acquire swapchain image, and signal SemaphoreImageAcquired[i] when acquired. (when the presentation engine is finished using the image)
        vkxc.CurrentSwapchainImage =
            vkx::check(device.acquireNextImageKHR(vkxc.SwapchainKHR, UINT64_MAX, vkxc.SemaphoreImageAcquired[fif_i]));

        device.resetFences(vkxc.CommandBufferFences[fif_i]);  // reset the fence to the unsignaled state

        cmd.Reset();
        cmd.Begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    }
    vkx::BeginMainRenderPass(cmd);
    {
        //cmd.BindGraphicsPipeline(g_Pipeline->Pipeline);
        //
        //cmd.SetViewport({}, vkxc.SwapchainExtent);
        //cmd.SetScissor({}, vkxc.SwapchainExtent);
        //
        //cmd.BindVertexBuffers(g_VBuffer->vertexBuffer);
        //cmd.BindIndexBuffer(g_VBuffer->indexBuffer);
        //
        //cmd.cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, g_Pipeline->PipelineLayout, 0, g_Pipeline->DescriptorSets[fif_i], {});
        //
        //cmd.DrawIndexed(g_VBuffer->vertexCount);

        Imgui::Render(cmd);

    }
    vkx::EndMainRenderPass(cmd);

    {
        cmd.End();

        // Submit the CommandBuffer.
        // Submission is VerySlow. try Batch Submit as much as possible, and Submit in another Thread
        vkx::QueueSubmit(vkxc.GraphicsQueue, cmd.cmd,
            vkxc.SemaphoreImageAcquired[fif_i], { vk::PipelineStageFlagBits::eColorAttachmentOutput },
            vkxc.SemaphoreRenderComplete[fif_i],
            vkxc.CommandBufferFences[fif_i]);

        vkx::QueuePresentKHR(vkxc.PresentQueue,
            vkxc.SemaphoreRenderComplete[fif_i], vkxc.SwapchainKHR, vkxc.CurrentSwapchainImage);

        //    vkQueueWaitIdle(vkx::ctx().PresentQueue);  // BigWaste on GPU.

        vkxc.CurrentInflightFrame = (vkxc.CurrentInflightFrame + 1) % vkxc.InflightFrames;
    }
}













//static void renderWorldGeometry(World* world)
//{
//
//    // Geometry of Deferred Rendering
//
//    PROFILE("Geo");
//    auto _ap = GeometryRenderer::fboGbuffer->BeginFramebuffer_Scoped();
//
//    RenderCommand::Clear();
//
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//
//    glDisable(GL_BLEND);  // Blending is inhabited in Deferred Rendering.
//
//
//    Dbg::dbg_NumEntityRendered = 0;
//    for (Entity* entity : world->getEntities()) {
//        if (!entity || !entity->m_Model) continue;
//        if (entity->m_Model->vertexCount == 0) // most are Empty Chunks.
//            continue;
//
//        // Frustum Culling
//        if (!Ethertia::getCamera().testFrustum(entity->getAABB())) {
//            continue;
//        }
//
//        if (entity == (void*)Ethertia::getPlayer() && Ethertia::getCamera().len == 0)
//            continue;
//
//
//        entity->onRender();
//
//
//        ++Dbg::dbg_NumEntityRendered;
//
//        // Debug: draw Norm/Border
//        if (Dbg::dbg_EntityGeo) {
//            DebugRenderer::Inst().renderDebugGeo(entity->m_Model, entity->position(), entity->getRotation());
//        }
//        // Debug: draw Entity that Crosshair Hits.
//        if (Dbg::dbg_HitPointEntityGeo && entity == Ethertia::getHitCursor().hitEntity) {
//            DebugRenderer::Inst().renderDebugGeo(entity->m_Model, entity->position(), entity->getRotation(),
//                           Ethertia::getHitCursor().brushSize, Ethertia::getHitCursor().position);
//        }
//        // Debug: draw Every Entity AABB.
//        if (Dbg::dbg_RenderedEntityAABB) {
//            AABB aabb = entity->getAABB();
//            Imgui::RenderAABB(aabb, Colors::RED);
//        }
//    }
//
//    glEnable(GL_BLEND);
//
//}


//static void renderWorldCompose(World* world)
//{
//    // Compose of Deferred Rendering
//
//    PROFILE("Cmp");
//    auto _ap = ComposeRenderer::fboCompose->BeginFramebuffer_Scoped();
//
//    glClearColor(0,0,0, 1);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
////    glEnable(GL_FRAMEBUFFER_SRGB);  // built-in Gamma Correction
//
//    float daytime = Ethertia::getWorld()->getDayTime();
//
//    SkyboxRenderer::renderWorldSkybox(daytime);
//
//    ParticleRenderer::renderSunMoonTex(daytime);
//
//    std::vector<Light*> renderLights;  // lights to be render.
////    {
////        static Light l{};
////        l.position = Ethertia::getCamera()->position;
////        l.color = glm::vec3(0.4, 1, 0.8);
////        l.direction = Ethertia::getCamera()->direction;
////        l.attenuation = {0,0,0};
////        renderLights.push_back(&l);
////    }
//    {
//        static Light sunLight;
//
//        // CNS 这是不准确的，不能直接取反 否则月亮位置可能会错误
//        sunLight.position = Ethertia::getCamera().position + (-RenderEngine::SunlightDir(daytime) * 100.0f);
//
//
//        float dayBrightness = 1.0 - abs(daytime-0.5) * 2.0;
//        dayBrightness = 0.4 + dayBrightness * 0.6;
//        glm::vec3 SunColor = Dbg::dbg_WorldSunColor * dayBrightness * Dbg::dbg_WorldSunColorBrightnessMul;
//        sunLight.color = SunColor;
//
//        sunLight.attenuation = {1, 0, 0};
//
//        renderLights.push_back(&sunLight);
//    }
//    //fixme: Do it later
////    for (Entity* e : world->m_Entities)
////    {
////        if (EntityLantern* lat = dynamic_cast<EntityLantern*>(e))
////        {
////            renderLights.push_back(lat->getLights());
////        }
////
////        if (EntityTorch* lat = dynamic_cast<EntityTorch*>(e))
////        {
////            renderLights.push_back(lat->getLights());
////
////            ParticleRenderer::m_Particles.push_back(lat->genParticles());
////        }
////    }
//
//
//    // CNS. 让接下来Alpha!=1.0的地方的颜色 添加到背景颜色中 为了接下来的天空颜色叠加
//    // Blend: addictive color when src.alpha != 1.0, for sky background add.
//    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//
//    Texture** gbufferTexs = GeometryRenderer::fboGbuffer->texColor;
//    ComposeRenderer::renderCompose(gbufferTexs[0], gbufferTexs[1], gbufferTexs[2],
//                                   SSAORenderer::fboSSAO->texColor[0],
//                                   ShadowRenderer::fboDepthMap->texDepth, ShadowRenderer::matShadowSpace,
//                                   renderLights);
//
//    GlState::blendMode(GlState::ALPHA);
//
//    for (Entity* e : world->getEntities())
//    {
//        // todo: EntityDroppedItem :: onRenderCompose();
//        if (EntityDroppedItem* eDroppedItem = dynamic_cast<EntityDroppedItem*>(e))
//        {
//            const ItemStack& stack = eDroppedItem->m_DroppedItem;
//
//            float i = Item::REGISTRY.getOrderId(stack.item());
//            float n = Item::REGISTRY.size();
//
//            ParticleRenderer::render(ItemTextures::ITEM_ATLAS, eDroppedItem->position(), 0.3f,
//                                       {i/n, 0},
//                                       {1/n, 1});
//
//            if (stack.amount() > 1)
//            {
//                // fixme: tmp draw count.
////                Gui::drawWorldpoint(eDroppedItem->position(), [&](glm::vec2 p) {
////                    Gui::drawString(p.x, p.y, std::to_string(stack.amount()));
////                });
//            }
//        }
//    }
//
//    glEnable(GL_DEPTH_TEST);
//
//    ParticleRenderer::renderAll();
//
//}




// void RenderEngine::RenderWorld()
//    GeometryRenderer::PrepareFrame();
//
//    if (Dbg::dbg_PauseWorldRender) return;
//
//    RenderCommand::CheckError("Begin World Render");
//
//    ParticleRenderer::updateAll(Ethertia::getDelta());
//
//    // Heavy
//    renderWorldGeometry(world);
//
//
//    float CurrTime = Ethertia::getPreciseTime();
//    static float s_LastTimeRenderShadowDepthMap = 0;
//    if ((s_LastTimeRenderShadowDepthMap + Settings::gInterval_ShadowDepthMap) <= CurrTime)
//    {
//        PROFILE("ShadowDepthMap");
//
//        s_LastTimeRenderShadowDepthMap = CurrTime;
//        ShadowRenderer::renderDepthMap(world->m_Entities, SunlightDir(world->getDayTime()), !Settings::g_ShadowMapping);
//    }
//
//    {
//        PROFILE("SSAO");
//
//        SSAORenderer::renderSSAO(GeometryRenderer::fboGbuffer->texColor[0], GeometryRenderer::fboGbuffer->texColor[1], !Settings::g_SSAO);
//    }
//
//
//    renderWorldCompose(world);
//
//
////    static Model* model = Loader::loadModel(100, {
////        {3, pos},
////        {2, uv},
////        {3, norm},
////        {3, joint_ids},
////        {3, joint_weights}});
////    AnimRenderer::Inst()->render();
//
//
//    RenderCommand::CheckError("End World Render");
//}


