

#pragma once

#include <vector>
#include <functional>


class ImWindows
{
public:
	using FuncPtr = void (*)(bool*);
	inline static std::vector<FuncPtr> Windows;

	static void Show(FuncPtr w);

	static bool Has(FuncPtr w);

	static void Erase(FuncPtr w);

	static void ShowWindows();


	class Inspector
	{
	public:
		inline static bool
			ShowInspector = true, 
			ShowEntityList = false;
	};

	class Debug
	{
	public:
		inline static bool
			ShowImGuiDemoWindow = false;
	};

};