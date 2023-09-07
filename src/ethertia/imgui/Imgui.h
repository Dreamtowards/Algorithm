
#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <vulkan/vulkan.hpp>

class Imgui
{
public:

	static void Init();

	static void Destroy();


	static void NewFrame();
	
	static void Render(vk::CommandBuffer cmdbuf);




    using DrawFuncPtr = void (*)(bool*);
    inline static std::vector<DrawFuncPtr> Windows;

    static void ShowWindows();

    static void Show(DrawFuncPtr w);

    static bool Has(DrawFuncPtr w);

    static void Close(DrawFuncPtr w);

    static void ToggleDrawCheckbox(const char* label, DrawFuncPtr w);
};