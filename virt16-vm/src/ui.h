#pragma once

#include <string>
#include <vector>

#include "vm/virt16.h"

/// Mutable UI state shared across tabs and persisted between frames.
struct AppState {
    std::vector<std::string> debug_info;  // one entry per assembled instruction, used by the debug panel
    bool graphics_mode = false;           // false = console (text), true = graphics (pixel)
};

/// Renders the "Load ROM" tab: file path input, load button, and status text.
/// @param vm the running VM instance
/// @param state shared UI state; debug_info is populated on a successful load
void render_load_rom_tab(Virt16::virt16* vm, AppState& state);

/// Renders the "Memory Viewer" tab: a 16-column editable hex dump of all VM memory.
/// @param vm the running VM instance
void render_memory_viewer_tab(Virt16::virt16* vm);

/// Renders the "Monitor" tab: register panel, display canvas, and debug output.
/// @param vm the running VM instance
/// @param state shared UI state; graphics_mode is toggled here
void render_monitor_tab(Virt16::virt16* vm, AppState& state);
