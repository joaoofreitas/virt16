cmake_minimum_required(VERSION 3.29)
project(virt16_vm)

set(CMAKE_CXX_STANDARD 20)

# Manually specify source files
set(SRC_FILES
    main.cpp
    # Add other source files here
)

# Manually specify imgui source files
set(IMGUI_DIR imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    # Add other imgui source files here
)
set(IMGUI_BACKENDS_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Include directories
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# Find and link libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(virt16_vm ${SRC_FILES} ${IMGUI_SOURCES} ${IMGUI_BACKENDS_SOURCES})

# Link libraries
target_link_libraries(virt16_vm PRIVATE OpenGL::GL glfw dl X11)