//
// Created by Dreamtowards on 2022/4/22.
//

#pragma once


class RenderEngine
{
public:
    RenderEngine() = delete;

    static void Init();
    static void Destroy();

    static void Render();


    //inline static vkx::Image* TEX_WHITE = nullptr;
    //inline static vkx::Image* TEX_UVMAP = nullptr;
    //
    //inline static VkImageView g_ComposeView = nullptr;
};

