//
// Created by johnny on 22/09/24.
//

#ifndef VIRT16_H
#define VIRT16_H

#define MEMORY_SIZE 16384

namespace Virt16 {
    enum Registers {
        PC = 0,
        SP = 1,
        TIME = 2,
        A = 3,
        R1 = 4,
        R2 = 5,
        R3 = 6,
        R4 = 7,
        R5 = 8,
        R6 = 9,
        R7 = 10,
        R8 = 11,
        R9 = 12,
        R10 = 13,
        R11 = 14,
        R12 = 15,
        R13 = 16,
        R14 = 17,
        R15 = 18,
        P1 = 19,
        P2 = 20,
        P3 = 21,
        P4 = 22,
        DISP = 23,
        FLAGS = 24
    };

    enum Flags {
        Z = 0,
        G = 1,
        L = 2,
        E = 3
    };

    class virt16 {
    private:
        unsigned short memory[MEMORY_SIZE]{};
        unsigned short registers[24]{};
        unsigned short peripherals[4]{};

        unsigned short time;
        unsigned short pc;
        unsigned short sp;
        unsigned short disp;

        bool z;
        bool g;
        bool l;
        bool e;

        bool running;

    public:
        virt16();

        void reset();

        [[nodiscard]] unsigned short getMemory(int index) const;

        [[nodiscard]] unsigned short getRegister(int index) const;

        [[nodiscard]] bool getFlag(int flag) const;

        [[nodiscard]] unsigned short getDisp() const;

        void setMemory(int index, unsigned short value);

        void setRegister(int index, unsigned short value);

        void setFlag(int flag, bool value);

        void setDisp(unsigned short value);

        static void step();

        static void load_program(const char *program);

        ~virt16();
    };
} // Virt16

#endif //VIRT16_H
