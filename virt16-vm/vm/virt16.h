//
// Created by johnny on 22/09/24.
//

#ifndef VIRT16_H
#define VIRT16_H

#define MEMORY_SIZE 16384

namespace Virt16 {
    enum Registers {
        R1,
        R2,
        R3,
        R4,
        R5,
        R6,
        R7,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
    };

    enum Peripherals {
        P1,
        P2,
        P3,
        P4
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
        unsigned short registers[16]{};
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
