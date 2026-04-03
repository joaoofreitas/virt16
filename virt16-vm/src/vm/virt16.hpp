#pragma once

namespace Virt16
{

inline constexpr int MEMORY_SIZE = 65536;

/// General-purpose and special register indices used to address the registers array.
enum Registers
{
    R0,   // general purpose
    R1,   // general purpose
    R2,   // general purpose
    R3,   // general purpose
    R4,   // general purpose
    R5,   // general purpose
    R6,   // general purpose
    R7,   // general purpose
    R8,   // general purpose
    R9,   // general purpose
    R10,  // general purpose
    R11,  // general purpose
    R12,  // general purpose
    R13,  // general purpose
    R14,  // general purpose
    R15,  // general purpose
    SP,   // stack pointer
    DISP, // display base address in VRAM
    TIME, // timer counter, auto-incremented each step
    A,    // accumulator
    P1,   // peripheral data register 1 (keyboard input)
    P2,   // peripheral data register 2
    P3,   // peripheral data register 3
    P4,   // peripheral data register 4
    TVEC, // timer interrupt service routine address
    KVEC, // keyboard interrupt service routine address
    TPER  // timer period: TIME resets and fires when TIME == TPER; 0 = disabled
};

/// Human-readable names for each register, indexed by Registers enum value.
inline constexpr const char* register_names[] = {"R0",   "R1",  "R2",  "R3",  "R4",  "R5",  "R6",   "R7",   "R8",
                                                 "R9",   "R10", "R11", "R12", "R13", "R14", "R15",  "SP",   "DISP",
                                                 "TIME", "A",   "P1",  "P2",  "P3",  "P4",  "TVEC", "KVEC", "TPER"};

class virt16
{
  public:
    unsigned short memory[MEMORY_SIZE]{};
    unsigned short registers[27]{};
    unsigned short pc = 0;

    // CPU flags — z is auto-set by arithmetic ops (INC, DEC, ADD, SUB)
    bool z = false; // zero
    bool g = false; // greater
    bool l = false; // less
    bool e = false; // equal
    bool c = false; // carry / borrow
    bool i = false; // interrupts enabled

    bool timer_pending = false;
    bool keyboard_pending = false;

    virt16() = default;
    ~virt16() = default;

    /// Resets the VM: zeroes all memory, registers, PC, and flags.
    void reset();

    /// Fetches and executes the 32-bit instruction at PC, then advances PC by 2.
    void step();

    /// Loads a flat binary program from disk into VM memory starting at address 0.
    /// @param program path to the .bin file
    void load_program(const char* program) noexcept;

    /// Starts continuous execution mode without blocking the caller.
    void start();

    /// Executes up to max_steps while in continuous execution mode.
    /// @param max_steps maximum instructions to execute this call
    void run_for_steps(unsigned int max_steps);

    /// Returns whether continuous execution mode is active.
    /// @return true if the VM is currently running
    bool is_running() const;

    /// Signals the VM to stop after the current step completes.
    void stop();

  private:
    bool running = false;
};

} // namespace Virt16
