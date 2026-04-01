#ifndef VIRT16_H
#define VIRT16_H

#include <map>
#include <string>

#define MEMORY_SIZE 65536

namespace Virt16 {

    enum Registers {
        R0, R1, R2, R3, R4, R5, R6, R7,
        R8, R9, R10, R11, R12, R13, R14, R15,
        SP, DISP, TIME, A, P1, P2, P3, P4
    };

    static const char* register_names[] = {
        "R0",  "R1",  "R2",  "R3",  "R4",  "R5",  "R6",  "R7",
        "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
        "SP",  "DISP", "TIME", "A",  "P1",  "P2",  "P3",  "P4"
    };

    static const std::map<std::string, int> register_map = {
        {"R0", R0}, {"R1", R1}, {"R2",  R2},  {"R3",  R3},
        {"R4", R4}, {"R5", R5}, {"R6",  R6},  {"R7",  R7},
        {"R8", R8}, {"R9", R9}, {"R10", R10}, {"R11", R11},
        {"R12", R12}, {"R13", R13}, {"R14", R14}, {"R15", R15},
        {"SP", SP}, {"DISP", DISP}, {"TIME", TIME}, {"A", A},
        {"P1", P1}, {"P2",  P2},  {"P3",  P3},  {"P4",  P4}
    };

    enum Flags { Z, G, L, E, C };

    class virt16 {
    private:
        unsigned short memory[MEMORY_SIZE]{};
        unsigned short registers[24]{};
        unsigned short pc;
        bool z, g, l, e, c;
        bool running;

    public:
        virt16();
        ~virt16();

        /// Resets the VM: zeroes all memory, registers, PC, and flags.
        void reset();

        /// Reads a 16-bit word from the given memory address.
        /// @param addr word address (0x0000–0xFFFF)
        /// @return value stored at that address
        [[nodiscard]] unsigned short getMemory(unsigned int addr) const;

        /// Reads the current value of a register.
        /// @param reg register to read
        /// @return 16-bit register value
        [[nodiscard]] unsigned short getRegister(Registers reg) const;

        /// Reads a CPU flag.
        /// @param flag flag to query (Z, G, L, E, C)
        /// @return current boolean value of that flag
        [[nodiscard]] bool getFlag(Flags flag) const;

        /// Reads the VRAM base address from the DISP register.
        /// @return current display base address
        [[nodiscard]] unsigned short getDisp() const;

        /// Reads the program counter.
        /// @return current PC value
        [[nodiscard]] unsigned short getPC() const;

        /// Writes a 16-bit word to the given memory address.
        /// @param addr word address (0x0000–0xFFFF)
        /// @param value value to store
        void setMemory(unsigned int addr, unsigned short value);

        /// Writes a value into a register.
        /// @param reg register to write
        /// @param value 16-bit value to store
        void setRegister(Registers reg, unsigned short value);

        /// Sets a CPU flag.
        /// @param flag flag to set (Z, G, L, E, C)
        /// @param value boolean value to assign
        void setFlag(Flags flag, bool value);

        /// Sets the VRAM base address via the DISP register.
        /// @param value new display base address
        void setDisp(unsigned short value);

        /// Fetches and executes the 32-bit instruction at PC, then advances PC by 2.
        void step();

        /// Loads a flat binary program from disk into VM memory starting at address 0.
        /// @param program path to the .bin file
        void load_program(const char* program) noexcept;

        /// Runs the VM in a blocking loop until HLT or stop() is called.
        void run();

        /// Signals the VM to stop after the current step completes.
        void stop();
    };

} // namespace Virt16

#endif // VIRT16_H
