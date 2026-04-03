#include "virt16.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>

namespace
{

// 5-bit opcode values packed into bits 31–27 of each 32-bit instruction.
enum Opcode : unsigned char
{
    LOAD_IMM = 0x00,
    LOAD_ADDR = 0x01,
    STORE_ADDR = 0x02,
    MOV = 0x03,
    INC = 0x04,
    DEC = 0x05,
    ADD = 0x06,
    SUB = 0x07,
    AND = 0x08,
    OR = 0x09,
    XOR = 0x0A,
    NOT = 0x0B,
    SHL = 0x0C,
    SHR = 0x0D,
    CMP = 0x0E,
    JMP = 0x0F,
    JZ = 0x10,
    JE = 0x11,
    JNE = 0x12,
    JG = 0x13,
    JL = 0x14,
    CALL = 0x15,
    RET = 0x16,
    PUSH = 0x17,
    POP = 0x18,
    HLT = 0x19,
    NOP = 0x1A,
    JC = 0x1B,
    EI = 0x1C,
    DI = 0x1D,
    RETI = 0x1E
};

// Instruction layout: [OPCODE:5][X:5][Y:5][Z:5][IMM/ADDR:16] — big-endian, MSB first.

[[nodiscard]] inline Virt16::Registers field_x(unsigned int instr)
{
    return static_cast<Virt16::Registers>((instr & 0x07C00000u) >> 22);
}

[[nodiscard]] inline Virt16::Registers field_y(unsigned int instr)
{
    return static_cast<Virt16::Registers>((instr & 0x003E0000u) >> 17);
}

[[nodiscard]] inline Virt16::Registers field_z(unsigned int instr)
{
    return static_cast<Virt16::Registers>((instr & 0x0001F000u) >> 12);
}

[[nodiscard]] inline unsigned short field_imm(unsigned int instr)
{
    return static_cast<unsigned short>(instr & 0x0000FFFFu);
}

} // anonymous namespace

namespace Virt16
{

void virt16::reset()
{
    std::memset(memory, 0, sizeof(memory));
    std::memset(registers, 0, sizeof(registers));
    pc = 0;
    z = g = l = e = c = i = false;
    timer_pending = keyboard_pending = false;
}

void virt16::step()
{
    // Fetch: two consecutive 16-bit words form one 32-bit instruction (big-endian).
    const unsigned int instr =
        (static_cast<unsigned int>(memory[pc]) << 16) | static_cast<unsigned int>(memory[pc + 1]);

    const unsigned char opcode = (instr & 0xF8000000u) >> 27;

    Registers X, Y, Z;
    unsigned short addr, imm;

    switch (opcode)
    {
    case LOAD_IMM:
        // LOAD X, #imm — load 16-bit immediate into register X
        X = field_x(instr);
        imm = field_imm(instr);
        registers[X] = imm;
        break;

    case LOAD_ADDR:
        // LOAD X, Y — load the word at the address stored in Y into X
        X = field_x(instr);
        Y = field_y(instr);
        registers[X] = memory[registers[Y]];
        break;

    case STORE_ADDR:
        // STORE X, Y — store value of Y into the memory address stored in X
        X = field_x(instr);
        Y = field_y(instr);
        memory[registers[X]] = registers[Y];
        break;

    case MOV:
        // MOV X, Y — copy register Y into X
        X = field_x(instr);
        Y = field_y(instr);
        registers[X] = registers[Y];
        break;

    case INC:
        X = field_x(instr);
        registers[X]++;
        z = registers[X] == 0;
        break;

    case DEC:
        X = field_x(instr);
        registers[X]--;
        z = registers[X] == 0;
        break;

    case ADD:
    {
        // ADD X, Y, Z — X = Y + Z; sets C on 16-bit overflow, Z if result is zero
        X = field_x(instr);
        Y = field_y(instr);
        Z = field_z(instr);
        const unsigned int sum = registers[Y] + registers[Z];
        const unsigned short result = static_cast<unsigned short>(sum & 0xFFFF);
        registers[X] = result;
        c = sum > 0xFFFF;
        z = result == 0;
        break;
    }

    case SUB:
    {
        // SUB X, Y, Z — X = Y - Z; sets C on borrow (underflow), Z if result is zero
        X = field_x(instr);
        Y = field_y(instr);
        Z = field_z(instr);
        const bool borrow = registers[Y] < registers[Z];
        const unsigned short result = static_cast<unsigned short>((registers[Y] - registers[Z]) & 0xFFFF);
        registers[X] = result;
        c = borrow;
        z = result == 0;
        break;
    }

    case AND:
        X = field_x(instr);
        Y = field_y(instr);
        Z = field_z(instr);
        registers[X] = registers[Y] & registers[Z];
        break;

    case OR:
        X = field_x(instr);
        Y = field_y(instr);
        Z = field_z(instr);
        registers[X] = registers[Y] | registers[Z];
        break;

    case XOR:
        X = field_x(instr);
        Y = field_y(instr);
        Z = field_z(instr);
        registers[X] = registers[Y] ^ registers[Z];
        break;

    case NOT:
        X = field_x(instr);
        Y = field_y(instr);
        registers[X] = ~registers[Y];
        break;

    case SHL:
        X = field_x(instr);
        Y = field_y(instr);
        Z = field_z(instr);
        registers[X] = registers[Y] << registers[Z];
        break;

    case SHR:
        X = field_x(instr);
        Y = field_y(instr);
        Z = field_z(instr);
        registers[X] = registers[Y] >> registers[Z];
        break;

    case CMP:
    {
        // CMP X, Y — clear all comparison flags, then set based on result
        X = field_x(instr);
        Y = field_y(instr);
        e = g = l = false;
        e = registers[X] == registers[Y];
        g = registers[X] > registers[Y];
        l = registers[X] < registers[Y];
        break;
    }

    case JMP:
        addr = field_imm(instr);
        pc = addr - 2; // -2 so the +2 at end of step() lands on addr
        break;

    case JZ:
        addr = field_imm(instr);
        if (z)
            pc = addr - 2;
        break;

    case JE:
        addr = field_imm(instr);
        if (e)
            pc = addr - 2;
        break;

    case JNE:
        addr = field_imm(instr);
        if (!e)
            pc = addr - 2;
        break;

    case JG:
        addr = field_imm(instr);
        if (g)
            pc = addr - 2;
        break;

    case JL:
        addr = field_imm(instr);
        if (l)
            pc = addr - 2;
        break;

    case CALL:
        // Push return address onto stack, then jump
        addr = field_imm(instr);
        registers[SP]--;
        memory[registers[SP]] = pc;
        pc = addr - 2; // -2 so the +2 at end of step() lands on addr
        break;

    case RET:
        pc = memory[registers[SP]];
        registers[SP]++;
        break;

    case PUSH:
        X = field_x(instr);
        registers[SP]--;
        memory[registers[SP]] = registers[X];
        break;

    case POP:
        X = field_x(instr);
        registers[X] = memory[registers[SP]];
        registers[SP]++;
        break;

    case HLT:
        running = false;
        pc -= 2; // cancel the +2 below so PC stays on the HLT
        break;

    case JC:
        addr = field_imm(instr);
        if (c)
            pc = addr - 2;
        break;

    case EI:
        i = true;
        break;

    case DI:
        i = false;
        break;

    case RETI:
        // Return from interrupt and re-enable interrupts
        pc = memory[registers[SP]];
        registers[SP]++;
        i = true;
        break;

    case NOP:
        break;

    default:
        fprintf(stderr, "invalid opcode: 0x%02X\n", static_cast<int>(opcode));
        this->stop();
        break;
    }

    pc += 2;

    // Advance the timer; fire a pending interrupt if the period has elapsed.
    registers[TIME]++;
    if (registers[TPER] != 0 && registers[TIME] == registers[TPER])
    {
        registers[TIME] = 0;
        timer_pending = true;
    }

    // Dispatch the highest-priority pending interrupt if interrupts are enabled.
    if (i)
    {
        if (timer_pending)
        {
            timer_pending = false;
            if (registers[TPER] != 0)
            {
                i = false;
                registers[SP]--;
                memory[registers[SP]] = pc - 2;
                pc = registers[TVEC];
            }
        }
        else if (keyboard_pending)
        {
            i = false;
            registers[SP]--;
            memory[registers[SP]] = pc - 2;
            pc = registers[KVEC];
            keyboard_pending = false;
        }
    }
}

void virt16::load_program(const char* program) noexcept
{
    std::ifstream file(program, std::ios::binary);
    if (!file.is_open())
    {
        fprintf(stderr, "failed to open file: %s\n", program);
        return;
    }

    unsigned short address = 0;
    unsigned short word;
    while (file.read(reinterpret_cast<char*>(&word), sizeof(word)))
        memory[address++] = word;
}

void virt16::start()
{
    running = true;
}

void virt16::run_for_steps(unsigned int max_steps)
{
    if (!running)
        return;

    for (unsigned int n = 0; n < max_steps && running; ++n)
        step();
}

bool virt16::is_running() const
{
    return running;
}

void virt16::stop()
{
    running = false;
}

} // namespace Virt16
