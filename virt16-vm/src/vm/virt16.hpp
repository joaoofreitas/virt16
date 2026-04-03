#pragma once

#include <map>
#include <string>

#define MEMORY_SIZE 65536

namespace Virt16
{

enum Registers
{
    R0,
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
    SP,
    DISP,
    TIME,
    A,
    P1,
    P2,
    P3,
    P4,
    TVEC,
    KVEC,
    TPER
};

static const char* register_names[] = {"R0",   "R1",  "R2",  "R3",  "R4",  "R5",  "R6",   "R7",   "R8",
                                       "R9",   "R10", "R11", "R12", "R13", "R14", "R15",  "SP",   "DISP",
                                       "TIME", "A",   "P1",  "P2",  "P3",  "P4",  "TVEC", "KVEC", "TPER"};

static const std::map<std::string, int> register_map = {
    {"R0", R0},   {"R1", R1},   {"R2", R2}, {"R3", R3},     {"R4", R4},     {"R5", R5},    {"R6", R6},
    {"R7", R7},   {"R8", R8},   {"R9", R9}, {"R10", R10},   {"R11", R11},   {"R12", R12},  {"R13", R13},
    {"R14", R14}, {"R15", R15}, {"SP", SP}, {"DISP", DISP}, {"TIME", TIME}, {"A", A},      {"P1", P1},
    {"P2", P2},   {"P3", P3},   {"P4", P4}, {"TVEC", TVEC}, {"KVEC", KVEC}, {"TPER", TPER}};

enum Flags
{
    Z,
    G,
    L,
    E,
    C
};

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
