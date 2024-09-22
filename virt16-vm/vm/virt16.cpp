//
// Created by johnny on 22/09/24.
//

#include <cstring>
#include "virt16.h"

namespace Virt16 {
    virt16::virt16() {
        std::memset(memory, 0, sizeof(memory));
        std::memset(registers, 0, sizeof(registers));
        std::memset(peripherals, 0, sizeof(peripherals));

        time = 0;
        pc = 0;
        sp = 0;
        disp = 0;

        z = false;
        g = false;
        l = false;
        e = false;

        running = false;
    }

    virt16::~virt16() = default;

    void virt16::reset() {
        // Clear memory
        std::memset(memory, 0, sizeof(memory));
        std::memset(registers, 0, sizeof(registers));
        std::memset(peripherals, 0, sizeof(peripherals));

        time = 0;
        pc = 0;

        z = false;
        g = false;
        l = false;
        e = false;
    }

    unsigned short virt16::getMemory(int index) const {
        if (index >= 0 && index < MEMORY_SIZE) {
            return memory[index];
        }
        return 0; // or throw an exception
    }

    unsigned short virt16::getRegister(int index) const {
        if (index >= 0 && index < 24) {
            return registers[index];
        }
        return 0; // or throw an exception
    }

    bool virt16::getFlag(int flag) const {
        switch (flag) {
            case Z: return z;
            case G: return g;
            case L: return l;
            case E: return e;
            default: return false; // or throw an exception
        }
    }

    // Setters
    void virt16::setMemory(int index, unsigned short value) {
        if (index >= 0 && index < MEMORY_SIZE) {
            memory[index] = value;
        }
        // else throw an exception
    }

    void virt16::setRegister(int index, unsigned short value) {
        if (index >= 0 && index < 24) {
            registers[index] = value;
        }
        // else throw an exception
    }

    void virt16::setFlag(int flag, bool value) {
        switch (flag) {
            case Z: z = value;
                break;
            case G: g = value;
                break;
            case L: l = value;
                break;
            case E: e = value;
                break;
            default: break; // or throw an exception
        }
    }

    void virt16::step() {
        // Implement the step function
    }

    void virt16::load_program(const char *program) {
        // Implement the load_program function
    }
} // Virt16
