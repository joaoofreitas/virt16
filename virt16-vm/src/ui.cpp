#include "ui.hpp"

#include "imgui.h"
#include "imgui_memory_editor.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

static constexpr int DISPLAY_PIXELS = 32; // graphics canvas is 32x32 pixels
static constexpr int UPSCALE = 16;        // each graphics pixel is 16x16 screen pixels
static constexpr int CONSOLE_CHARS = 16;  // console grid is 16x16 characters
static constexpr int CHAR_PIXELS = 8;     // each character glyph is 8x8 pixels
static constexpr int UPSCALE_CONSOLE = 4; // each glyph pixel is 4x4 screen pixels

/// Reads a .debug file matching the given .bin path and stores the lines in out.
/// @param bin_path path to the loaded .bin file
/// @param out vector to populate with debug lines
static void load_debug_info(const char* bin_path, std::vector<std::string>& out)
{
    std::string path(bin_path);
    path = path.substr(0, path.find_last_of('.')) + ".debug";

    out.clear();
    std::ifstream file(path);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line))
        out.push_back(line);
}

/// Executes a shell command and returns stdout trimmed of trailing whitespace.
/// @param command shell command to execute
/// @return command stdout (empty string on failure)
static std::string run_command_capture(const char* command)
{
    FILE* pipe = popen(command, "r");
    if (!pipe)
        return "";

    std::string result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe))
        result += buffer;
    pclose(pipe);

    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
        result.pop_back();

    return result;
}

/// Opens a platform-specific file picker and returns a selected ROM path.
/// macOS: osascript chooser. Linux: zenity, then kdialog fallback.
/// @return absolute file path or empty string if cancelled/unavailable
static std::string browse_for_rom_path()
{
#if defined(__APPLE__)
    return run_command_capture(
        "osascript -e 'POSIX path of (choose file with prompt \"Select a ROM (.bin)\")' 2>/dev/null");
#elif defined(__linux__)
    std::string path = run_command_capture("zenity --file-selection --title='Select ROM (.bin)' 2>/dev/null");
    if (!path.empty())
        return path;
    return run_command_capture("kdialog --getopenfilename 2>/dev/null");
#else
    return "";
#endif
}

void render_load_rom_tab(Virt16::virt16* vm, AppState& state)
{
    static char file_path[256] = "";

    ImGui::SeparatorText("ROM Loader");
    ImGui::TextDisabled("Select a .bin file to load into VM memory.");
    ImGui::Spacing();

    ImGui::SetNextItemWidth(520.0f);
    ImGui::InputText("ROM Path", file_path, sizeof(file_path));
    ImGui::SameLine();
    if (ImGui::Button("Browse..."))
    {
        const std::string selected = browse_for_rom_path();
        if (!selected.empty())
        {
            std::snprintf(file_path, sizeof(file_path), "%s", selected.c_str());
        }
    }

    if (ImGui::Button("Load ROM", ImVec2(160.0f, 0.0f)) && file_path[0] != '\0')
    {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open())
        {
            state.rom_load_ok = false;
            state.rom_status = "Failed to open ROM file.";
            return;
        }

        vm->load_program(file_path);
        load_debug_info(file_path, state.debug_info);
        state.rom_load_ok = true;
        state.rom_status = "ROM loaded successfully.";
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(120.0f, 0.0f)))
    {
        file_path[0] = '\0';
        state.rom_status.clear();
    }

    ImGui::Spacing();
    if (!state.rom_status.empty())
    {
        const ImVec4 ok_col(0.35f, 0.90f, 0.45f, 1.0f);
        const ImVec4 err_col(0.95f, 0.40f, 0.40f, 1.0f);
        ImGui::TextColored(state.rom_load_ok ? ok_col : err_col, "%s", state.rom_status.c_str());
    }
    else if (file_path[0] == '\0')
    {
        ImGui::TextDisabled("No file selected");
    }
}

void render_memory_viewer_tab(Virt16::virt16* vm)
{
    static MemoryEditor mem_edit;
    mem_edit.Cols = 32;
    mem_edit.OptShowOptions = true;
    mem_edit.OptShowDataPreview = true;
    mem_edit.OptShowAscii = true;
    mem_edit.ReadOnly = false;
    mem_edit.PreviewEndianness = 1;
    mem_edit.PreviewDataType = ImGuiDataType_U16;
    mem_edit.HighlightColor = IM_COL32(246, 190, 0, 50); // Dark Yellow
    mem_edit.DrawContents(vm->memory, sizeof(vm->memory));
}

/// Renders the control buttons (Step / Reset / Run / Stop) for the VM.
/// @param vm the running VM instance
static void render_control_buttons(Virt16::virt16* vm, AppState& state)
{
    ImGui::SeparatorText("Controls");
    if (ImGui::Button("Step", ImVec2(84.0f, 0.0f)))
        vm->step();
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(84.0f, 0.0f)))
    {
        vm->reset();
        state.auto_run = false;
    }
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.50f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.60f, 0.36f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.17f, 0.43f, 0.26f, 1.0f));
    if (ImGui::Button("Run", ImVec2(84.0f, 0.0f)))
    {
        vm->start();
        state.auto_run = true;
    }
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.58f, 0.22f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.27f, 0.27f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.19f, 0.19f, 1.0f));
    if (ImGui::Button("Stop", ImVec2(84.0f, 0.0f)))
    {
        vm->stop();
        state.auto_run = false;
    }
    ImGui::PopStyleColor(3);

    ImGui::TextColored(state.auto_run ? ImVec4(0.40f, 0.90f, 0.55f, 1.0f) : ImVec4(0.75f, 0.75f, 0.80f, 1.0f),
                       "Status: %s",
                       state.auto_run ? "Running" : "Idle");
}

/// Renders an editable table of all 27 general-purpose and special registers.
/// @param vm the running VM instance
static void render_register_table(Virt16::virt16* vm)
{
    ImGui::SeparatorText("Registers");

    if (!ImGui::BeginTable("Registers", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        return;

    ImGui::TableSetupColumn("Register");
    ImGui::TableSetupColumn("Value");
    ImGui::TableHeadersRow();

    for (int i = Virt16::R0; i <= Virt16::TPER; ++i)
    {
        const auto reg = static_cast<Virt16::Registers>(i);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", Virt16::register_names[i]);

        ImGui::TableSetColumnIndex(1);
        char buf[5];
        std::snprintf(buf, sizeof(buf), "%04X", vm->registers[reg]);

        char label[8];
        std::snprintf(label, sizeof(label), "##%s", Virt16::register_names[i]);
        if (ImGui::InputText(
                label, buf, sizeof(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
        {
            vm->registers[reg] = static_cast<unsigned short>(std::stoul(buf, nullptr, 16));
        }
    }

    ImGui::EndTable();
}

/// Renders the CPU flags (Z, G, L, E, C, I) as read-only labels.
/// @param vm the running VM instance
static void render_flags(Virt16::virt16* vm)
{
    ImGui::SeparatorText("Flags");
    ImGui::Text("Z:%d", vm->z);
    ImGui::SameLine();
    ImGui::Text("G:%d", vm->g);
    ImGui::SameLine();
    ImGui::Text("L:%d", vm->l);
    ImGui::SameLine();
    ImGui::Text("E:%d", vm->e);
    ImGui::SameLine();
    ImGui::Text("C:%d", vm->c);
    ImGui::SameLine();
    ImGui::Text("I:%d", vm->i);
}

/// Renders the left register panel (buttons, register table, flags).
/// @param vm the running VM instance
static void render_register_panel(Virt16::virt16* vm, AppState& state)
{
    ImGui::BeginChild("RegisterPanel", ImVec2(200, 0), ImGuiChildFlags_Borders);
    render_control_buttons(vm, state);
    render_register_table(vm);
    render_flags(vm);
    ImGui::EndChild();
}

/// Draws the 32x32 graphics display canvas, reading pixel colors from VRAM.
/// Each word encodes a 12-bit RGB color: 0xRGBx (nibbles, high-to-low).
/// @param vm the running VM instance
/// @param canvas_pos top-left screen position of the canvas
static void render_graphics_canvas(Virt16::virt16* vm, ImVec2 canvas_pos)
{
    const ImVec2 canvas_size(DISPLAY_PIXELS * UPSCALE, DISPLAY_PIXELS * UPSCALE);
    ImGui::InvisibleButton("canvas", canvas_size);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int y = 0; y < DISPLAY_PIXELS; ++y)
    {
        for (int x = 0; x < DISPLAY_PIXELS; ++x)
        {
            const unsigned short c = vm->memory[vm->registers[Virt16::DISP] + y * DISPLAY_PIXELS + x];
            const unsigned char r = (c & 0xF000) >> 8;
            const unsigned char g = (c & 0x0F00) >> 4;
            const unsigned char b = (c & 0x00F0) >> 0;
            const ImU32 color = IM_COL32(r * 17, g * 17, b * 17, 0xFF);

            const ImVec2 p_min(canvas_pos.x + x * UPSCALE, canvas_pos.y + y * UPSCALE);
            const ImVec2 p_max(p_min.x + UPSCALE, p_min.y + UPSCALE);
            dl->AddRectFilled(p_min, p_max, color);
        }
    }
}

/// Draws the 16x16 text console canvas by reading character codes and font data from VM memory.
/// Character buffer: 0x2900. Font base: 0x3100 (4 words per glyph, ASCII 32–127).
/// @param vm the running VM instance
/// @param canvas_pos top-left screen position of the canvas
static void render_console_canvas(Virt16::virt16* vm, ImVec2 canvas_pos)
{
    const int canvas_px = CONSOLE_CHARS * CHAR_PIXELS * UPSCALE_CONSOLE;
    const ImVec2 canvas_size(canvas_px, canvas_px);
    ImGui::InvisibleButton("canvas", canvas_size);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Black background
    dl->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_px, canvas_pos.y + canvas_px), IM_COL32(0, 0, 0, 0xFF));

    for (int cy = 0; cy < CONSOLE_CHARS; ++cy)
    {
        for (int cx = 0; cx < CONSOLE_CHARS; ++cx)
        {
            const unsigned short ch = vm->memory[0x2900 + cy * CONSOLE_CHARS + cx];
            if (ch < 32 || ch > 127)
                continue;

            // Each glyph occupies 4 words starting at font_addr.
            // Each word holds two rows of 8 pixels: high byte = row N, low byte = row N+1.
            const unsigned short font_addr = 0x3100 + (ch - 32) * 4;
            unsigned char rows[8];
            for (int w = 0; w < 4; ++w)
            {
                const unsigned short word = vm->memory[font_addr + w];
                rows[w * 2] = (word & 0xFF00) >> 8;
                rows[w * 2 + 1] = (word & 0x00FF);
            }

            const ImVec2 char_origin(canvas_pos.x + cx * CHAR_PIXELS * UPSCALE_CONSOLE,
                                     canvas_pos.y + cy * CHAR_PIXELS * UPSCALE_CONSOLE);

            for (int row = 0; row < CHAR_PIXELS; ++row)
            {
                for (int bit = 0; bit < CHAR_PIXELS; ++bit)
                {
                    const bool lit = (rows[row] >> (7 - bit)) & 1;
                    const ImU32 col = lit ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255);
                    const ImVec2 p_min(char_origin.x + bit * UPSCALE_CONSOLE, char_origin.y + row * UPSCALE_CONSOLE);
                    const ImVec2 p_max(p_min.x + UPSCALE_CONSOLE, p_min.y + UPSCALE_CONSOLE);
                    dl->AddRectFilled(p_min, p_max, col);
                }
            }
        }
    }
}

/// Renders the center display panel with a mode toggle and the active canvas.
/// @param vm the running VM instance
/// @param graphics_mode true for pixel graphics, false for text console
static void render_display_panel(Virt16::virt16* vm, bool& graphics_mode)
{
    ImGui::BeginChild("DisplayPanel", ImVec2(512, 0), ImGuiChildFlags_Borders);

    ImGui::SeparatorText("Display");
    ImGui::Text("Display Mode: %s", graphics_mode ? "Graphics" : "Console");
    ImGui::SameLine();
    if (ImGui::Button("Toggle", ImVec2(90.0f, 0.0f)))
        graphics_mode = !graphics_mode;

    // Center the canvas within the panel
    const float canvas_w = graphics_mode ? static_cast<float>(DISPLAY_PIXELS * UPSCALE)
                                         : static_cast<float>(CONSOLE_CHARS * CHAR_PIXELS * UPSCALE_CONSOLE);
    const float canvas_h = canvas_w;

    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    canvas_pos.x += (ImGui::GetContentRegionAvail().x - canvas_w) / 2.0f;
    canvas_pos.y += (ImGui::GetContentRegionAvail().y - canvas_h) / 2.0f;

    if (graphics_mode)
        render_graphics_canvas(vm, canvas_pos);
    else
        render_console_canvas(vm, canvas_pos);

    ImGui::EndChild();
}

/// Renders the hex keyboard (4×4 grid of buttons). Pressing a key stores the
/// key value in P1 and sets keyboard_pending to trigger the KVEC interrupt.
/// @param vm the running VM instance
static void render_hex_keyboard(Virt16::virt16* vm)
{
    ImGui::BeginChild("Peripherals", ImVec2(0, 175), ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Hex Keyboard (P1)");
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
    for (int i = 0; i < 16; ++i)
    {
        if (i % 4 != 0)
            ImGui::SameLine();
        char label[3];
        std::snprintf(label, sizeof(label), "%X", i);
        if (ImGui::Button(label, ImVec2(32.0f, 24.0f)))
        {
            vm->registers[Virt16::P1] = static_cast<unsigned short>(i);
            vm->keyboard_pending = true;
        }
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

/// Renders the debug info panel: highlights the current PC line in yellow.
/// @param vm the running VM instance
/// @param debug_info one assembled instruction string per index
static void render_debug_panel(Virt16::virt16* vm, const std::vector<std::string>& debug_info)
{
    ImGui::BeginChild("DebugInfo", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);
    ImGui::SeparatorText("Debug Trace");
    ImGui::Text("PC: 0x%04X", vm->pc);

    for (int i = 0; i < static_cast<int>(debug_info.size()); ++i)
    {
        const char* text = debug_info[i].c_str();
        float text_w = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorScreenPos(
            ImVec2(ImGui::GetCursorScreenPos().x + (ImGui::GetContentRegionAvail().x - text_w) / 2.0f,
                   ImGui::GetCursorScreenPos().y));

        if (vm->pc / 2 == i)
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", text);
        else
            ImGui::Text("%s", text);
    }

    ImGui::EndChild();
}

/// Renders the right panel: exclusive registers, hex keyboard, and debug output.
/// @param vm the running VM instance
/// @param debug_info assembled instruction strings for the debug view
static void render_right_panel(Virt16::virt16* vm, const std::vector<std::string>& debug_info)
{
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), ImGuiChildFlags_Borders);

    // Top row: peripherals
    render_hex_keyboard(vm);

    ImGui::Separator();
    render_debug_panel(vm, debug_info);

    ImGui::EndChild();
}

void render_monitor_tab(Virt16::virt16* vm, AppState& state)
{
    // Cooperative execution keeps the UI responsive even for long/infinite programs.
    if (state.auto_run)
    {
        vm->run_for_steps(5000);
        if (!vm->is_running())
            state.auto_run = false;
    }

    render_register_panel(vm, state);
    ImGui::SameLine();
    render_display_panel(vm, state.graphics_mode);
    ImGui::SameLine();
    render_right_panel(vm, state.debug_info);
}
