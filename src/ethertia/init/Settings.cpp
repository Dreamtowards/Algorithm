//
// Created by Dreamtowards on 2023/3/8.
//



#include <ethertia/init/Settings.h>

#include <ethertia/util/Loader.h>
#include <ethertia/world/gen/NoiseGen.h>
#include <ethertia/render/RenderEngine.h>
#include <ethertia/init/MaterialTextures.h>
#include <ethertia/Ethertia.h>
#include <ethertia/render/Window.h>

#include <nlohmann/json.hpp>


template<typename T>
static void TryLoad(nlohmann::json& j, std::string_view name, T* p) 
{
    if (j.contains(name))
    {
        *p = j[name];
    }
}

void Settings::LoadSettings() {  
    BENCHMARK_TIMER;
    Log::info("Load Settings.\1");
    if (!Loader::fileExists(SETTINGS_FILE))
        return;

    using nlohmann::json;
    json settings = json::parse((char*)Loader::loadFile(SETTINGS_FILE).data());

    TryLoad(settings, "view_distance",  &s_ViewDistance);
    TryLoad(settings, "fov",            &Ethertia::getCamera().fov);
    TryLoad(settings, "display_width",  &displayWidth);
    TryLoad(settings, "display_height", &displayHeight);
    TryLoad(settings, "vsync",          &s_Vsync);

    TryLoad(settings, "assets",         &Settings::ASSETS);
    TryLoad(settings, "mods",           &MODS);


    std::string simdLevel;
    TryLoad(settings, "simd_level", &simdLevel);
    NoiseGen::g_SIMDLevel = NoiseGen::FastSIMD_ofLevelName(simdLevel);

    TryLoad(settings, "mtl_resolution", &MaterialTextures::TEX_RESOLUTION);
    
    TryLoad(settings, "graphics.ssao", &g_SSAO);
    TryLoad(settings, "graphics.shadow", &g_ShadowMapping);
    
    TryLoad(settings, "window.viewport", &w_Viewport);
    TryLoad(settings, "window.settings", &w_Settings);
    TryLoad(settings, "window.toolbar", &w_Toolbar);
    TryLoad(settings, "window.console", &w_Console);
    
    TryLoad(settings, "window.entity_list", &w_EntityList);
    TryLoad(settings, "window.entity_insp", &w_EntityInsp);
    TryLoad(settings, "window.shader_insp", &w_ShaderInsp);


}


void Settings::SaveSettings() 
{ 
    BENCHMARK_TIMER;
    Log::info("Save Settings.\1");

    using nlohmann::json;
    json settings = json::object({
        {"view_distance",  s_ViewDistance},
        {"fov",            Ethertia::getCamera().fov},
        {"assets",         Settings::ASSETS},
        {"display_width",  Window::Size().x},
        {"display_height", Window::Size().y},
        {"vsync",          s_Vsync},
        {"fullscreen",     false},
        {"mtl_resolution", MaterialTextures::TEX_RESOLUTION},
        {"simd_level",     NoiseGen::FastSIMD_LevelName(NoiseGen::g_SIMDLevel)},
        {"mods",           MODS},
        {"graphics.ssao",  Settings::g_SSAO},
        {"graphics.shadow",Settings::g_ShadowMapping},
        {"window.viewport",w_Viewport},
        {"window.settings",w_Settings},
        {"window.toolbar", w_Toolbar},
        {"window.console", w_Console},
        {"window.entity_list", w_EntityList},
        {"window.entity_insp", w_EntityInsp},
        {"window.shader_insp", w_ShaderInsp}
    });

    std::ofstream file(SETTINGS_FILE);
    file << settings.dump(2);
}