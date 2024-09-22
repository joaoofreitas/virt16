# Makefile flags
CC = g++
SRCDIR = ./src
BUILD_DIR = ./build
PROJECT_NAME = vm
# Add flags if needed
CFLAGS =
LDFLAGS = -lGL -lglfw -ldl -lX11 -lGL
IMGUI_DIR = ./src/imgui

.PHONY: build run clean

default:
	$(CC) $(SRCDIR)/*.cpp $(CFLAGS) -o $(BUILD_DIR)/$(PROJECT_NAME) \
	$(IMGUI_DIR)/imgui*.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp \
	-I $(IMGUI_DIR) -I $(IMGUI_DIR)/backends \
	-I $(LDFLAGS) \
	&& $(BUILD_DIR)/$(PROJECT_NAME)
init:
	mkdir build src
	touch $(SRCDIR)/main.cpp
build:
	$(CC) $(SRCDIR)/*.cpp $(CFLAGS) -o $(BUILD_DIR)/$(PROJECT_NAME) \
	$(IMGUI_DIR)/imgui*.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \ $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp \
	-I $(IMGUI_DIR) -I $(IMGUI_DIR)/backends \
	-I $(LDFLAGS)

run:
	$(BUILD_DIR)/$(PROJECT_NAME)
clean:
	rm $(BUILD_DIR)/*
