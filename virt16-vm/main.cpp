#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <string>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

#define WIDTH 1280
#define HEIGHT 720
#define SIZE ImVec2(WIDTH, HEIGHT)

// Main code
int main(int, char **) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Virt16 - Virtual Machine", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    io.Fonts->Clear();
    io.Fonts->Build();

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    uint16_t memory[16384] = {0};
    static uint16_t goto_address = 0;
    char address_input[4 + 1] = {0};

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        ImGui::Begin("Virt16 - Virtual Machine", NULL,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        // Make it full size
        ImGui::SetWindowSize(SIZE);
        ImGui::SetWindowPos(ImVec2(0, 0));

        if (ImGui::BeginTabBar("MainTabBar")) {
            if (ImGui::BeginTabItem("Memory Viewer")) {
                // Label on left
                ImGui::InputText("##AddressInput", address_input, sizeof(address_input),
                                 ImGuiInputTextFlags_CharsHexadecimal);
                ImGui::SameLine();
                if (ImGui::Button("Visit")) {
                    goto_address = std::strtol(address_input, nullptr, 16);
                }

                // Create a table with 16 columns
                if (ImGui::BeginTable("Memory Dump", 17, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    // Create header row
                    ImGui::TableSetupColumn("Address");
                    for (int col = 0; col < 16; col++) {
                        ImGui::TableSetupColumn(std::to_string(col).c_str());
                    }
                    ImGui::TableHeadersRow();

                    // Populate the table
                    for (int row = 0; row < 16384 / 16; row++) {
                        ImGui::TableNextRow();
                        if (goto_address > 0 && row == goto_address / 16) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(255, 0, 0, 255));
                            ImGui::SetScrollHereY();
                            goto_address = 0;
                        }


                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("0x%04X", row * 16); // Address column

                        for (int col = 0; col < 16; col++) {
                            ImGui::TableSetColumnIndex(col + 1);
                            char buf[9];
                            snprintf(buf, sizeof(buf), "%04X", memory[row * 16 + col]);

                            // Align cell at the center
                            ImGui::SetCursorPosX(
                                ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(buf).x) / 2);

                            if (ImGui::InputText(("##" + std::to_string(row) + std::to_string(col)).c_str(), buf,
                                                 sizeof(buf),
                                                 ImGuiInputTextFlags_CharsHexadecimal |
                                                 ImGuiInputTextFlags_CharsUppercase)) {
                                memory[row * 16 + col] = std::stoul(buf, nullptr, 16);
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Display")) {
                // Canvas content
                ImGui::Text("Display");

                ImDrawList *draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos(); // Top-left of the canvas
                ImVec2 canvas_size = ImVec2(256, 256); // Size of the canvas
                // Align center
                canvas_pos.x += (ImGui::GetWindowWidth() - canvas_size.x) * 0.5f;
                canvas_pos.y += (ImGui::GetWindowHeight() - canvas_size.y) * 0.5f;



                // Draw a 32x32 grid of colored rectangles
                for (int y = 0; y < 32; y++) {
                    for (int x = 0; x < 32; x++) {
                        ImU32 color = IM_COL32(60, 60, 60, 60); // Set your desired color here
                        ImVec2 p_min = ImVec2(canvas_pos.x + x * 8, canvas_pos.y + y * 8);
                        ImVec2 p_max = ImVec2(p_min.x + 8, p_min.y + 8);
                        draw_list->AddRectFilled(p_min, p_max, color);
                    }
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
