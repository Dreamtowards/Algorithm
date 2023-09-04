
cmake_minimum_required(VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


set(CMAKE_CXX_STANDARD 20)

set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")

project(Ethertia)

message("Ethertia CMake G ${CMAKE_GENERATOR}, arch ${CMAKE_SYSTEM_PROCESSOR}, os ${CMAKE_SYSTEM_NAME}")


include_directories(src)

include_directories(lib/glfw-3.3.8/include)
include_directories(lib/imgui-docking-1.89.9)
include_directories(lib/openal-soft/include)
include_directories(lib/tinyfd-2.9.3)

include_directories(lib/glm-0.9.9.8)
include_directories(lib/stb)
include_directories(lib/tinyobjloader-1.0.6)
include_directories(lib/fastnoise2-0.10.0a/include)
include_directories(lib/tinyobjloader-1.0.6/include)
include_directories(lib/json11-1.0.0/include)


add_executable(Ethertia
    src/vkx/vkx.cpp
    
    lib/imgui-docking-1.89.9/imgui.cpp
    lib/imgui-docking-1.89.9/imgui_draw.cpp
    lib/imgui-docking-1.89.9/imgui_widgets.cpp
    lib/imgui-docking-1.89.9/imgui_tables.cpp
    lib/imgui-docking-1.89.9/imgui_demo.cpp
    lib/imgui-docking-1.89.9/backends/imgui_impl_glfw.cpp
    lib/imgui-docking-1.89.9/backends/imgui_impl_vulkan.cpp

    lib/tinyfd-2.9.3/tinyfiledialogs.c

    src/ethertia/imgui/Imgui.cpp
    src/ethertia/util/BenchmarkTimer.cpp
    src/ethertia/util/Loader.cpp
    src/ethertia/util/BitmapImage.cpp
    src/ethertia/render/Window.cpp
    src/ethertia/render/RenderEngine.cpp
    src/ethertia/render/VertexData.cpp
    src/ethertia/Ethertia.cpp)


target_link_libraries(Ethertia 
    "${LIB_DIR}/glfw-3.3.8/bin/windows-x64-msvc2022/glfw3.lib")

target_include_directories(Ethertia PRIVATE
    "C:/Softwares/VulkanSDK/1.3.250.0/Include/")
    
target_link_directories(Ethertia PRIVATE
    "C:/Softwares/VulkanSDK/1.3.250.0/Lib/")
    
target_link_libraries(Ethertia 
    "vulkan-1")

    
if (MSVC)
    target_compile_options(Ethertia PRIVATE 
        /wd4267  # msvc ignore C4267: implicit conversion possible loss of data e.g. uint32_t count = (size_t)ls.size();
        /wd4244
    )
endif()