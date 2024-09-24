//
// Created by johnny on 22/09/24.
//

#ifndef VIRT16_H
#define VIRT16_H

#define MEMORY_SIZE 65536
#include <map>
#include <string>

namespace Virt16 {
    enum Registers {
        R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
        SP, DISP, TIME, A, P1, P2, P3, P4
    };

    // Array with register names
    static const char *register_names[] = {
            "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
            "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
            "SP", "DISP", "TIME", "A", "P1", "P2", "P3", "P4"
    };

    // Map of register names to their index
    static const std::map<std::string, int> register_map = {
            {"R0",   R0},
            {"R1",   R1},
            {"R2",   R2},
            {"R3",   R3},
            {"R4",   R4},
            {"R5",   R5},
            {"R6",   R6},
            {"R7",   R7},
            {"R8",   R8},
            {"R9",   R9},
            {"R10",  R10},
            {"R11",  R11},
            {"R12",  R12},
            {"R13",  R13},
            {"R14",  R14},
            {"R15",  R15},
            {"SP",   SP},
            {"DISP", DISP},
            {"TIME", TIME},
            {"A",    A},
            {"P1",   P1},
            {"P2",   P2},
            {"P3",   P3},
            {"P4",   P4}
    };

    enum Flags {
        Z, G, L, E
    };

    class virt16 {
    private:
        unsigned short memory[MEMORY_SIZE]{};
        unsigned short registers[24]{};

        unsigned short pc;

        bool z;
        bool g;
        bool l;
        bool e;

        bool running;

    public:
        virt16();

        void reset();

        [[nodiscard]] unsigned short getMemory(int addr) const;

        [[nodiscard]] unsigned short getRegister(Registers reg) const;

        [[nodiscard]] bool getFlag(Flags flag) const;

        [[nodiscard]] unsigned short getDisp() const;

        void setMemory(int addr, unsigned short value);

        void setRegister(Registers reg, unsigned short value);

        void setFlag(int flag, bool value);

        void setDisp(unsigned short value);

        void step();

        void load_program(const char *program) noexcept;

        ~virt16();
    };
} // Virt16

#endif //VIRT16_H
