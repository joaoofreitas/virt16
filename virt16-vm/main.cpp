#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <complex>
#include <string>
#include <sstream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <iostream>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "vm/virt16.h"


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
#define ORIGINAL_DISPLAY_SIZE ImVec2(32, 32)
#define UPSCALE 16
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

    // VM instance
    auto *vm = new Virt16::virt16();
    vm->setDisp(0x3000);

    // UI Defs
    static uint16_t goto_address = 0;
    char address_input[4 + 1] = {0};

    // Main loop
#ifdef __EMSCRIPTEN__
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        ImGui::Begin("Virt16 - Virtual Machine", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        // Make it full size
        ImGui::SetWindowSize(SIZE);
        ImGui::SetWindowPos(ImVec2(0, 0));

        if (ImGui::BeginTabBar("MainTabBar")) {
            if (ImGui::BeginTabItem("Load ROM")) {
                // Add button for file picking and present the selected file and load button
                static char file_path[256] = "";

                // Align center of the window
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (ImGui::GetContentRegionAvail().y - 100) / 2);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 180);

                ImGui::InputText("ROM File Path", file_path, sizeof(file_path));
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 180);
                if (ImGui::Button("Load ROM")) {
                    if (strlen(file_path) > 0) {
                        try {
                            vm->load_program(file_path);
                        } catch ([[maybe_unused]] std::runtime_error &e) {
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 50) / 2);
                            ImGui::Text("Error loading file");
                        } catch (...) {
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 50) / 2);
                            ImGui::Text("Unknown error loading file");
                        }
                    }
                }
                if (file_path[0] == '\0') {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 180);
                    ImGui::Text("No file selected");
                }

                ImGui::SameLine();
                ImGui::Text("%s", file_path);

                ImGui::EndTabItem();
            }

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
                        std::stringstream ss;
                        ss << std::hex << col;
                        ImGui::TableSetupColumn(ss.str().c_str());
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
                            snprintf(buf, sizeof(buf), "%04X", vm->getMemory(row * 16 + col));

                            // Align cell at the center
                            ImGui::SetCursorPosX(
                                ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(buf).x) / 2);
                            const int addr = row * 16 + col;
                            char label[7];
                            snprintf(label, sizeof(label), "##%04X", addr);
                            if (ImGui::InputText(label,
                                                 buf,
                                                 sizeof(buf),
                                                 ImGuiInputTextFlags_CharsHexadecimal |
                                                 ImGuiInputTextFlags_CharsUppercase)) {
                                vm->setMemory(row * 16 + col, std::stoul(buf, nullptr, 16));
                            }
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Monitor")) {
                // Canvas content

                // Panel on right side
                ImGui::BeginChild("Register Settings", ImVec2(200, 0), true);
                // Button to step, reset, run, reset
                if (ImGui::Button("Step")) {
                    // Step
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset")) {
                    // Reset
                }
                ImGui::SameLine();
                if (ImGui::Button("Run")) {
                    // Run
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop")) {
                    // Stop
                }

                // Add a separator
                ImGui::Separator();
                ImGui::Text("Register Status");


                // Create Register Table
                if (ImGui::BeginTable("Registers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    // Create header row
                    ImGui::TableSetupColumn("Register");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();

                    // Populate the table
                    for (int row = 0; row <= 15; row++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("R%d", row);
                        ImGui::TableSetColumnIndex(1);
                        char buf[9];
                        snprintf(buf, sizeof(buf), "%04X", vm->getRegister(row));

                        char reg_labels[8];
                        snprintf(reg_labels, sizeof(reg_labels), "##R%d", row);
                        if (ImGui::InputText(reg_labels, buf, sizeof(buf),
                                             ImGuiInputTextFlags_CharsHexadecimal |
                                             ImGuiInputTextFlags_CharsUppercase)) {
                            vm->setRegister(row, std::stoul(buf, nullptr, 16));
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::Separator();
                // Display Flags
                ImGui::Text("Flags");
                ImGui::Text("Z: %d", vm->getFlag(Virt16::Z));
                ImGui::SameLine();
                ImGui::Text("G: %d", vm->getFlag(Virt16::G));
                ImGui::SameLine();
                ImGui::Text("L: %d", vm->getFlag(Virt16::L));
                ImGui::SameLine();
                ImGui::Text("E: %d", vm->getFlag(Virt16::E));

                ImGui::Separator();


                ImGui::EndChild();

                // Display on the right side of the Monitor tab
                ImGui::SameLine();
                ImGui::BeginChild("Display", ImVec2(512, 0), true);
                // Create canvas in the center of the Display Panel
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
                // Center canvas position based on the size of the canvas
                canvas_pos.x += (ImGui::GetContentRegionAvail().x - (32 * UPSCALE)) / 2;
                canvas_pos.y += (ImGui::GetContentRegionAvail().y - (32 * UPSCALE)) / 2;


                ImVec2 canvas_size = ImVec2(32 * UPSCALE, 32 * UPSCALE);
                ImGui::InvisibleButton("canvas", canvas_size);
                ImDrawList *draw_list = ImGui::GetWindowDrawList();
                // Draw a 32x32 grid of colored rectangles
                for (int y = 0; y < 32; y++) {
                    for (int x = 0; x < 32; x++) {
                        //ImU32 color = IM_COL32(60, 60, 60, 60); // Set your desired color here
                        // Color will be vm->getMemory(vm->GetDisp() + y * 32 + x)
                        ImU32 color = IM_COL32(0xff, 0xff, 0xff, 0xff);
                        const unsigned short c = vm->getMemory(vm->getDisp() + y * 32 + x);
                        const unsigned char r = (c & 0xF000) >> 8;
                        const unsigned char g = (c & 0x0F00) >> 4;
                        const unsigned char b = (c & 0x00F0 >> 2);
                        //const unsigned char a = (c & 0x000F);
                        color = IM_COL32(r*17, g*17, b*0x17, 0xff);

                        ImVec2 p_min = ImVec2(canvas_pos.x + static_cast<float>(x) * UPSCALE,
                                              canvas_pos.y + static_cast<float>(y) * UPSCALE);
                        ImVec2 p_max = ImVec2(p_min.x + UPSCALE, p_min.y + UPSCALE);
                        draw_list->AddRectFilled(p_min, p_max, color);
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("Exclusive Registers and Peripherals", ImVec2(0, 0), true);
                ImGui::BeginChild("Exclusive Registers", ImVec2(300, 150), true);
                //ImGui::Text("Time: 0x%04X", vm->getRegister(Virt16::TIME));
                //ImGui::Text("PC: 0x%04X", vm->getRegister(Virt16::PC));
                //ImGui::Text("SP: 0x%04X", vm->getRegister(Virt16::SP));
                ImGui::Text("Disp: 0x%04X", vm->getDisp());
                ImGui::Text(
                    "TODO: Implement Getters and Setters \n for exclusive register and display \n here in a table");


                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("Peripherals", ImVec2(0, 150), true);
                // Draw Hex Keyboard
                ImGui::Text("Hex Keyboard (P0)");
                // We draw 16 buttons 4x4 And when one is pressed, we set the corresponding bit in the peripheral register P0
                for (int i = 0; i < 16; i++) {
                    if (i % 4 != 0) {
                        ImGui::SameLine();
                    }
                    char label[3];
                    snprintf(label, sizeof(label), "%X", i);
                    if (ImGui::Button(label)) {
                        // Set the bit in the peripheral register
                        // To implement set peripheral
                    }
                }
                ImGui::Text("(Action Not implemented!)");

                ImGui::EndChild();
                ImGui::EndChild();


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
