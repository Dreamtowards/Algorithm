
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

};