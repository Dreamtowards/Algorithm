

#include "ImWindows.h"

#include <ethertia/imgui/Imgui.h>
#include <ethertia/Ethertia.h>



#pragma region Main Dockspace & Menubar


static void _MenuSystem()
{
    if (ImGui::BeginMenu("Servers"))
    {
        if (ImGui::MenuItem("Connect to server..")) {
        }
        ImGui::Separator();
        ImGui::TextDisabled("Servers:");

        if (ImGui::SmallButton("+")) {
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add server");

        ImGui::EndMenu();
    }
    //ImGui::Separator();
    //
    //if (ImGui::BeginMenu("Open World"))
    //{
    //    if (ImGui::MenuItem("New World..")) {
    //        w_NewWorld = true;
    //    }
    //    if (ImGui::MenuItem("Open World..")) {
    //        const char* filename = Loader::openFolderDialog("Open World..", "./saves/");  //std::filesystem::current_path().append("/saves/").string().c_str());
    //        if (filename) {
    //            Log::info("Open world: ", filename);
    //            Ethertia::loadWorld(filename);
    //        }
    //    }
    //
    //    //ImGui::SeparatorText("Saves");
    //    ImGui::Separator();
    //    ImGui::TextDisabled("Saves:");
    //
    //    if (Loader::fileExists("saves/"))
    //    {
    //        for (const auto& savedir : std::filesystem::directory_iterator("saves/"))
    //        {
    //            //            std::string size_str = Strings::size_str(Loader::calcDirectorySize(savedir.path()));
    //
    //            float epoch = std::chrono::duration_cast<std::chrono::seconds>(savedir.last_write_time().time_since_epoch()).count();
    //            if (epoch < 0)  epoch = 0;  // Error on Windows.
    //            std::string time_str = Strings::time_fmt(epoch);
    //
    //            auto filename = savedir.path().filename();
    //            if (ImGui::MenuItem((const char*)filename.c_str(), time_str.c_str()))
    //            {
    //                Ethertia::loadWorld(savedir.path().string());
    //            }
    //        }
    //    }
    //
    //    ImGui::EndMenu();
    //}
    //
    //bool worldvalid = Ethertia::getWorld();
    //if (ImGui::MenuItem("Edit World..", nullptr, false, worldvalid))
    //{
    //    Settings::w_Settings = true;
    //    g_CurrSettingsPanel = SettingsPanel::CurrentWorld;
    //}
    //if (ImGui::MenuItem("Save World", nullptr, false, worldvalid)) {}
    //
    //if (ImGui::MenuItem("Close World", nullptr, false, worldvalid)) {
    //    Ethertia::unloadWorld();
    //}
    //
    //ImGui::Separator();
    //
    //if (ImGui::MenuItem("Settings.."))
    //{
    //    Settings::w_Settings = true;
    //}
    //
    //if (ImGui::MenuItem("Mods", "0 mods loaded")) {
    //    Settings::w_Settings = true;
    //    g_CurrSettingsPanel = SettingsPanel::Mods;
    //}
    //if (ImGui::MenuItem("Resource Packs")) {
    //    Settings::w_Settings = true;
    //    g_CurrSettingsPanel = SettingsPanel::ResourcePacks;
    //}
    //if (ImGui::MenuItem("Controls")) {
    //    Settings::w_Settings = true;
    //    g_CurrSettingsPanel = SettingsPanel::Controls;
    //}
    //
    //
    //
    //if (ImGui::MenuItem("About")) {
    //    Settings::w_Settings = true;
    //    g_CurrSettingsPanel = SettingsPanel::Credits;
    //}
    //
    ImGui::Separator();

    if (ImGui::MenuItem("Terminate")) {
        Ethertia::Shutdown();
    }
}

template<typename T>
static T* ptr(const T& ref)
{
    return (T*) &ref;
}

static void ShowMainMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("System"))
        {
            _MenuSystem();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("World"))
        {

            //ImGui::Checkbox("AllEntityAABB", &dbg_AllEntityAABB);
            //ImGui::Checkbox("AllChunkAABB", &dbg_AllChunkAABB);
            ////            ImGui::Checkbox("DbgHitEntityAABB", &GuiDebugV::dbg_CursorPt);
            ////            ImGui::Checkbox("DbgNearChunkBoundAABB", &GuiDebugV::dbgChunkBoundAABB);
            ////            ImGui::Checkbox("DbgCursorNearCellsInfo", &GuiDebugV::dbgCursorRangeInfo);
            //
            //ImGui::Checkbox("MeshingChunks AABB", &Dbg::dbg_MeshingChunksAABB);
            //ImGui::Checkbox("Chunk Mesh Counter", &Dbg::dbg_ChunkMeshedCounter);
            //
            //ImGui::Checkbox("NoChunkSave", &Dbg::dbg_NoChunkSave);
            //ImGui::Checkbox("NoChunkLoad", &Dbg::dbg_NoChunkLoad);
            //
            //ImGui::Checkbox("PauseThread ChunkMeshing", &Dbg::dbg_PauseThread_ChunkMeshing);
            //ImGui::Checkbox("PauseThread ChunkLoad/Gen/Save", &Dbg::dbg_PauseThread_ChunkLoadGenSave);
            //
            //if (ImGui::Button("Remesh All Chunks")) {
            //    for (auto it : Ethertia::getWorld()->getLoadedChunks()) {
            //        it.second->requestRemodel();
            //    }
            //}
            //
            //if (ImGui::BeginMenu("Gamemode"))
            //{
            //    EntityPlayer& p = *Ethertia::getPlayer();
            //    int gm = Ethertia::getPlayer()->getGamemode();
            //
            //    if (ImGui::MenuItem("Survival", nullptr, gm == Gamemode::SURVIVAL))  p.switchGamemode(Gamemode::SURVIVAL);
            //    if (ImGui::MenuItem("Creative", nullptr, gm == Gamemode::CREATIVE))  p.switchGamemode(Gamemode::CREATIVE);
            //    if (ImGui::MenuItem("Spectator", nullptr, gm == Gamemode::SPECTATOR)) p.switchGamemode(Gamemode::SPECTATOR);
            //
            //    ImGui::EndMenu();
            //}
            //ImGui::SliderFloat("Breaking Terrain Interval in CreativeMode", &Settings::gInterval_BreakingTerrain_CreativeMode, 0, 0.5f);
            //
            //ImGui::Separator();
            //
            //ImGui::Checkbox("Text Info", &Dbg::dbg_TextInfo);
            //ImGui::Checkbox("View Gizmo", &Dbg::dbg_ViewGizmo);
            //ImGui::Checkbox("View Basis", &Dbg::dbg_ViewBasis);
            //ImGui::Checkbox("World Basis", &Dbg::dbg_WorldBasis);
            //ImGui::SliderInt("World GridSize", &Dbg::dbg_WorldHintGrids, 0, 500);
            //
            //ImGui::Separator();
            //
            //ImGui::Checkbox("Hit Tracking", &Ethertia::getHitCursor().keepTracking);
            //ImGui::SliderFloat("BrushSize", &Ethertia::getHitCursor().brushSize, 0, 16);
            //
            //Camera& cam = Ethertia::getCamera();
            //ImGui::SliderFloat("Camera Smoothness", &cam.m_Smoothness, 0, 5);
            //ImGui::SliderFloat("Camera Roll", &cam.eulerAngles.z, -3.14, 3.14);

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Rendering"))
        {
            //ImGui::SliderFloat("FOV", &Ethertia::getCamera().fov, 0, 180);
            //ImGui::SliderFloat("ViewDistance", &Settings::s_ViewDistance, 0, 16);
            //ImGui::Checkbox("Vsync", &Settings::s_Vsync);
            //
            //ImGui::Checkbox("SSAO", &Settings::g_SSAO);
            //ImGui::Checkbox("Shadow Mapping", &Settings::g_ShadowMapping);
            //ImGui::SliderFloat("Shadow Depth Map Re-Render Interval", &Settings::gInterval_ShadowDepthMap, 0, 10);
            //
            //ImGui::Separator();
            //
            //ImGui::Checkbox("PauseWorldRender", &Dbg::dbg_PauseWorldRender);
            //ImGui::Checkbox("GBuffers", &dbg_Gbuffer);
            //ImGui::Checkbox("Border/Norm", &Dbg::dbg_EntityGeo);
            //ImGui::Checkbox("HitEntityGeo", &Dbg::dbg_HitPointEntityGeo);
            //
            //ImGui::Checkbox("NoVegetable", &Dbg::dbg_NoVegetable);

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Audio"))
        {
            //    if (ImGui::Button("Click Sound")) {
            //        Log::info("PlaySoundClick");
            //        Sounds::AS_GUI->UnqueueAllBuffers();
            //        Sounds::AS_GUI->QueueBuffer(Sounds::GUI_CLICK->m_BufferId);
            //        Sounds::AS_GUI->play();
            //    }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {

            if (ImGui::BeginMenu("Debug"))
            {
                bool show = ImWindows::Has(ImGui::ShowDemoWindow);
                if (ImGui::Checkbox("ImGui DemoWindow", &show)) {
                    if (show) {
                        ImWindows::Show(ImGui::ShowDemoWindow);
                    } else {
                        ImWindows::Erase(ImGui::ShowDemoWindow);
                    }
                }

                ImGui::EndMenu();
            }
            //ImGui::Checkbox("Viewport", &Settings::w_Viewport);
            //ImGui::Checkbox("Full Viewport", &Settings::w_Viewport_Full);
            //ImGui::Checkbox("Entity List", &Settings::w_EntityList);
            //ImGui::Checkbox("Entity Inspect", &Settings::w_EntityInsp);
            //ImGui::Checkbox("Shader Inspect", &Settings::w_ShaderInsp);
            //ImGui::Checkbox("Console", &Settings::w_Console);
            //ImGui::Checkbox("Profiler", &Settings::w_Profiler);
            //
            //ImGui::Separator();
            //ImGui::Checkbox("Settings", &Settings::w_Settings);
            //ImGui::Checkbox("Toolbar", &Settings::w_Toolbar);
            //
            //ImGui::Checkbox("NodeEditor", &w_NodeEditor);
            //ImGui::Checkbox("TitleScreen", &w_TitleScreen);
            //ImGui::Checkbox("Singleplayer", &w_Singleplayer);
            //
            //
            //ImGui::Separator();
            //
            //static bool ShowHUD = true;
            //if (ImGui::MenuItem("HUD", "F1", &ShowHUD)) {
            //
            //}
            //if (ImGui::MenuItem("Save Screenshot", KeyBindings::KEY_SCREENSHOT.keyName())) {
            //    Controls::saveScreenshot();
            //}
            //if (ImGui::MenuItem("Fullscreen", KeyBindings::KEY_FULLSCREEN.keyName(), Ethertia::getWindow().isFullscreen())) {
            //    Window::ToggleFullscreen();
            //}
            //if (ImGui::MenuItem("Controlling Game", KeyBindings::KEY_ESC.keyName(), Ethertia::isIngame())) {
            //    Ethertia::isIngame() = !Ethertia::isIngame();
            //}

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

}

#pragma endregion



static void ShowDockspaceAndMainMenubar()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

    ImVec4 Blu = { 0.188, 0.478, 0.776, 1.0 };
    ImVec4 Pur = { 0.373, 0.157, 0.467, 1.0 };
    ImVec4 Org = { 0.741, 0.345, 0.133, 1.0 };
    ImVec4 Dark = { 0.176f, 0.176f, 0.176f, 0.700f };
    ImVec4 _col = Dark;  // Dbg::dbg_PauseWorldRender ? Org : Ethertia::isIngame() ? _Dar : Pur;
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, _col);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    ImGui::DockSpace(ImGui::GetID("MainDockSpace"), { 0, 0 },
        ImGuiDockNodeFlags_PassthruCentralNode);  //ImGuiDockNodeFlags_AutoHideTabBar

    ShowMainMenuBar();


    //// Draw Holding ItemStack. tmp.
    //if (!s_HoldingItemStack.empty())
    //{
    //    ImGui::SetCursorScreenPos(ImGui::GetMousePos() - ImVec2(20, 20));
    //    ItemImage(s_HoldingItemStack.item(), 40, ImGui::GetForegroundDrawList());
    //}

    ImGui::End();
}




namespace ImInspector
{
    void* Inspecting = nullptr;

    void ShowInspector()
    {
        ImGui::Begin("Inspector");



        ImGui::End();
    }

    void InspCamera()
    {

    }

};

#include <stdx/stdx.h>

#include <ethertia/util/Log.h>

void ImWindows::Show(FuncPtr w)
{
    if (Has(w))
    {
        Log::info("Failed to ShowWindow, Existed Already");
        return;
    }
    ImWindows::Windows.push_back(w);
}

bool ImWindows::Has(FuncPtr w)
{
    return stdx::exists(ImWindows::Windows, w);  //auto& ls = ImWindows::Windows;std::find(ls.begin(), ls.end(), w) != ls.end();
}

void ImWindows::Erase(FuncPtr w)
{
    stdx::erase(ImWindows::Windows, w);
}

void ImWindows::ShowWindows()
{
    ShowDockspaceAndMainMenubar();

    auto& windows = ImWindows::Windows;
    for (int i = windows.size()-1; i >= 0; --i)
    {
        bool show = true;

        windows[i](&show);

        if (!show) 
        {
            stdx::erase(windows, i);
        }
    }
}