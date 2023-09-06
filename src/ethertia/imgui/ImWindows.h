

#pragma once

#include <vector>


class ImWindows
{
public:
	using FuncPtr = void (*)(bool*);
	inline static std::vector<FuncPtr> Windows;

	static void Show(FuncPtr w);

	static bool Has(FuncPtr w);

	static void Close(FuncPtr w);

	static void ShowWindows();

	// ToggleWindow
	static void Checkbox(const char* label, FuncPtr w);


	struct Inspector
	{
		inline static void* Inspecting = nullptr;

		static void ShowInspector();
	};

	struct Settings
	{

	};

};