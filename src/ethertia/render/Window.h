//
// Created by Dreamtowards on 2022/3/31.
//

#pragma once

//#define GL
#define VULKAN

#ifdef VULKAN
#define GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_NONE 1
#endif

#include <stdexcept>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>



class Window
{
public:
    static void Init(int _w, int _h, const char* _title);
    static void Destroy();
    static double PreciseTime();

    static void PollEvents();
    static GLFWwindow* Handle();

    /// reposition window to monitor center.
    static void Centralize();
    void SetTitle(const char* s);

    static bool IsCloseRequested();
    static bool IsFramebufferResized();

    static void SetFullscreen(GLFWmonitor* monitor = glfwGetPrimaryMonitor());
    static void UnsetFullscreen(int _w = 1280, int _h = 720);
    static bool IsFullscreen();
    static void ToggleFullscreen();


    static glm::vec2 MousePos();
    static glm::vec2 MouseDelta();
    static glm::vec2 MouseWheel();
    static float MouseWheelSum() { return MouseWheel().x + MouseWheel().y; }

    static void SetMouseGrabbed(bool grabbed);
    static void SetStickyKeys(bool s);

    static glm::vec2 Size();
//    static glm::vec2 Pos();




    static bool isKeyDown(int key);
    static bool isMouseDown(int button);

    static bool isMouseLeftDown() { return isMouseDown(GLFW_MOUSE_BUTTON_LEFT); }
    static bool isMouseRightDown() { return isMouseDown(GLFW_MOUSE_BUTTON_RIGHT); }
    static bool isMouseMiddleDown() { return isMouseDown(GLFW_MOUSE_BUTTON_MIDDLE); }

    static bool isShiftKeyDown() { return isKeyDown(GLFW_KEY_LEFT_SHIFT) || isKeyDown(GLFW_KEY_RIGHT_SHIFT); }
    static bool isAltKeyDown() { return isKeyDown(GLFW_KEY_LEFT_ALT) || isKeyDown(GLFW_KEY_RIGHT_ALT); }
    static bool isCtrlKeyDown() { return isKeyDown(GLFW_KEY_LEFT_CONTROL) || isKeyDown(GLFW_KEY_RIGHT_CONTROL); }

    static const char* GetClipboard();
    static void SetClipboard(const char* str);




//    void maximize() {
//        glfwMaximizeWindow(m_WindowHandle);
//    }
//    void restoreMaximize() {
//        glfwRestoreWindow(m_WindowHandle);
//    }



//    void SwapBuffers()
//    {
//        glfwSwapBuffers(m_WindowHandle);
//    }
//
//    void setVSync(bool vsync) {
//        glfwSwapInterval(vsync ? 1 : 0);
//    }


};
