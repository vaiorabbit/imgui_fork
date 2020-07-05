// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

class MemoryAllocator
{
public:
    static void* Allocate(size_t sz, void* /*user_data*/) {
        allocated_size.fetch_add(sz);
        return std::malloc(sz);
    }

    static void Free(void* ptr, void* /*user_data*/) {
        size_t sz = malloc_size(ptr); // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/malloc_size.3.html [TODO] __APPLE__ only.
        allocated_size.fetch_sub(sz);
        return std::free(ptr);
    }

    static size_t GetAllocatedSize() {
        return allocated_size.load();
    }

private:
    static std::atomic_size_t allocated_size;
};
std::atomic_size_t MemoryAllocator::allocated_size{0};

// Main code
int main(int argc, char* argv[])
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    ImGui::SetAllocatorFunctions(MemoryAllocator::Allocate, MemoryAllocator::Free);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
#ifdef IMGUI_USE_WCHAR32
    // Enable IMGUI_USE_WCHAR32 if you want to display "𠮟" (the modern form of "叱") correctly.
    builder.AddText(u8"𠮟"); // codepoint 0x20b9f(==134047, exceeds the range of ImWchar16), encoded as F0 A0 AE 9F in UTF-8
#endif
    ImVector<ImWchar> out_ranges;
    builder.BuildRanges(&out_ranges);
#if __APPLE__
    ImFont* font = io.Fonts->AddFontFromFileTTF("../../data/NotoSansCJKjp/NotoSansMonoCJKjp-Regular.otf", 20.0f, nullptr, out_ranges.Data);
#elif _MSC_VER
    ImFont* font = io.Fonts->AddFontFromFileTTF("../data/NotoSansCJKjp/NotoSansMonoCJKjp-Regular.otf", 20.0f, nullptr, out_ranges.Data);
#endif
    IM_ASSERT(font != NULL);

    // Japanese text (2999 kanjis included)
#if __APPLE__
    std::ifstream text_stream_regular("../../kanji/regular_use.txt");
    std::ifstream text_stream_name_1("../../kanji/personal_name_1.txt");
    std::ifstream text_stream_name_2("../../kanji/personal_name_2.txt");
#elif _MSC_VER
    std::ifstream text_stream_regular("../kanji/regular_use.txt");
    std::ifstream text_stream_name_1("../kanji/personal_name_1.txt");
    std::ifstream text_stream_name_2("../kanji/personal_name_2.txt");
#endif

    std::vector<std::string> text_regular, text_name_1, text_name_2;
    {
        std::string line;
        while (std::getline(text_stream_regular, line)) {
            text_regular.emplace_back(line);
        }
        while (std::getline(text_stream_name_1, line)) {
            text_name_1.emplace_back(line);
        }
        while (std::getline(text_stream_name_2, line)) {
            text_name_2.emplace_back(line);
        }
    }

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) {
            // ImGui::ShowDemoWindow(&show_demo_window);
            ImGui::ShowStyleEditor();
        }
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.

        ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 620), ImGuiCond_FirstUseEver);
        ImGui::Begin(u8"2136 常用漢字 (https://en.wikipedia.org/wiki/List_of_j%C5%8Dy%C5%8D_kanji)");
        for (auto& line : text_regular) {
            ImGui::TextWrapped(line.c_str());
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(610, 5), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(630, 175), ImGuiCond_FirstUseEver);
        ImGui::Begin(u8"651 人名用漢字(not part of 常用漢字) (https://en.wikipedia.org/wiki/Jinmeiy%C5%8D_kanji)");
        for (auto& line : text_name_1) {
            ImGui::TextWrapped(line.c_str());
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(610, 190), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(630, 175), ImGuiCond_FirstUseEver);
        ImGui::Begin(u8"212 人名用漢字(Traditional variants of 常用漢字) (https://en.wikipedia.org/wiki/Jinmeiy%C5%8D_kanji)");
        for (auto& line : text_name_2) {
            ImGui::TextWrapped(line.c_str());
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(610, 370), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 120), ImGuiCond_FirstUseEver);
        ImGui::Begin(u8"𠮟 (modern form) and 叱 (traditional form)");
        ImGui::TextWrapped(
#ifdef IMGUI_USE_WCHAR32
            u8"IMGUI_USE_WCHAR32 : defined"
#else
            u8"IMGUI_USE_WCHAR32 : undefined"
#endif
            );
        ImGui::TextWrapped(u8"𠮟 (codepoint 0x20b9f(==134047), encoded as F0 A0 AE 9F in UTF-8)");
        ImGui::TextWrapped(u8"叱 (codepoint 0x53f1(==21489), encoded as E5 8F B1 in UTF-8)");
        ImGui::End();

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        //
        // GetGlyphRangesJapanese[Current]
        //   (Debug, IMGUI_USE_WCHAR32 undefined)   -> GetAllocatedSize=25529865
        //   (Debug, IMGUI_USE_WCHAR32 defined)     -> GetAllocatedSize=26347677
        //   (Release, IMGUI_USE_WCHAR32 undefined) -> GetAllocatedSize=25500926
        //   (Release, IMGUI_USE_WCHAR32 defined)   -> GetAllocatedSize=26347410
        //
        // GetGlyphRangesJapanese[New]
        //   (Debug, IMGUI_USE_WCHAR32 undefined)   -> GetAllocatedSize=24653983
        //   (Debug, IMGUI_USE_WCHAR32 defined)     -> GetAllocatedSize=25494463
        //   (Release, IMGUI_USE_WCHAR32 undefined) -> GetAllocatedSize=24653716
        //   (Release, IMGUI_USE_WCHAR32 defined)   -> GetAllocatedSize=25474059
        //
        // GetGlyphRangesChineseFull
        //   (Debug, IMGUI_USE_WCHAR32 undefined)   -> GetAllocatedSize=82231985
        //   (Debug, IMGUI_USE_WCHAR32 defined)     -> GetAllocatedSize=83040773
        //   (Release, IMGUI_USE_WCHAR32 undefined) -> GetAllocatedSize=82228814
        //   (Release, IMGUI_USE_WCHAR32 defined)   -> GetAllocatedSize=83039361
        //
        // printf("GetAllocatedSize=%ld\n", MemoryAllocator::GetAllocatedSize());
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
