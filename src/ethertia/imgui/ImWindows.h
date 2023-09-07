

#pragma once

#include <vector>


class ImWindows
{
public:

    static void ShowDockspaceAndMainMenubar();

    struct Inspector
	{
		inline static void* Inspecting = nullptr;

		static void ShowInspector();
	};

	struct Settings
	{

	    static void ShowSettings();
	};

};