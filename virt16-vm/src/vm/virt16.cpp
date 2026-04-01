#include "virt16.h"

#include <cstring>
#include <fstream>
#include <iostream>

// Opcode constants — 5-bit values packed into the top of each 32-bit instruction.
#define LOAD_IMM 0x00
#define LOAD_ADDR 0x01
#define STORE_ADDR 0x02
#define MOV 0x03
#define INC 0x04
#define DEC 0x05
#define ADD 0x06
#define SUB 0x07
#define AND 0x08
#define OR 0x09
#define XOR 0x0A
#define NOT 0x0B
#define SHL 0x0C
#define SHR 0x0D
#define CMP 0x0E
#define JMP 0x0F
#define JZ 0x10
#define JE 0x11
#define JNE 0x12
#define JG 0x13
#define JL 0x14
#define CALL 0x15
#define RET 0x16
#define PUSH 0x17
#define POP 0x18
#define HLT 0x19
#define NOP 0x1A

// Instruction field extraction helpers.
// Each instruction is 32 bits: [OPCODE:5][X:5][Y:5][Z:5][IMM/ADDR:16] (big-endian, MSB first).
#define FIELD_X(instr) static_cast<Registers>(((instr) & 0b00000111110000000000000000000000) >> 22)
#define FIELD_Y(instr) static_cast<Registers>(((instr) & 0b00000000001111100000000000000000) >> 17)
#define FIELD_Z(instr) static_cast<Registers>(((instr) & 0b00000000000000011111000000000000) >> 12)
#define FIELD_IMM(instr) static_cast<unsigned short>((instr) & 0x0000FFFF)

namespace Virt16
{

virt16::virt16()
{
    std::memset(memory, 0, sizeof(memory));
    std::memset(registers, 0, sizeof(registers));
    pc = 0;
    z = g = l = e = c = false;
    running = false;
}

virt16::~virt16() = default;

void virt16::reset()
{
    std::memset(memory, 0, sizeof(memory));
    std::memset(registers, 0, sizeof(registers));
    pc = 0;
    z = g = l = e = c = false;
}

unsigned short virt16::getMemory(const unsigned int addr) const
{
    return memory[addr];
}

unsigned short virt16::getRegister(const Registers reg) const
{
    return registers[reg];
}

bool virt16::getFlag(const Flags flag) const
{
    switch (flag)
    {
    case Z:
        return z;
    case G:
        return g;
    case L:
        return l;
    case E:
        return e;
    case C:
        return c;
    default:
        return false;
    }
}

unsigned short virt16::getDisp() const
{
    return getRegister(DISP);
}

unsigned short virt16::getPC() const
{
    return pc;
}

void virt16::setMemory(const unsigned int addr, const unsigned short value)
{
    memory[addr] = value;
}

void virt16::setRegister(const Registers reg, const unsigned short value)
{
    registers[reg] = value;
}

void virt16::setFlag(const Flags flag, const bool value)
{
    switch (flag)
    {
    case Z:
        z = value;
        break;
    case G:
        g = value;
        break;
    case L:
        l = value;
        break;
    case E:
        e = value;
        break;
    case C:
        c = value;
        break;
    default:
        break;
    }
}

void virt16::setDisp(const unsigned short value)
{
    setRegister(DISP, value);
}

void virt16::step()
{
    // Fetch: two consecutive 16-bit words form one 32-bit instruction (big-endian).
    const unsigned int instr =
        (static_cast<unsigned int>(getMemory(pc)) << 16) | static_cast<unsigned int>(getMemory(pc + 1));

    const unsigned char opcode = (instr & 0xF8000000) >> 27;

    Registers X, Y, Z;
    unsigned short addr, imm;

    switch (opcode)
    {
    case LOAD_IMM:
        // LOAD X, #imm — load 16-bit immediate into register X
        X = FIELD_X(instr);
        imm = FIELD_IMM(instr);
        setRegister(X, imm);
        break;

    case LOAD_ADDR:
        // LOAD X, Y — load the word at the address stored in Y into X
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        setRegister(X, getMemory(getRegister(Y)));
        break;

    case STORE_ADDR:
        // STORE X, Y — store value of Y into the memory address stored in X
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        setMemory(getRegister(X), getRegister(Y));
        break;

    case MOV:
        // MOV X, Y — copy register Y into X
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        setRegister(X, getRegister(Y));
        break;

    case INC:
        X = FIELD_X(instr);
        setRegister(X, getRegister(X) + 1);
        break;

    case DEC:
        X = FIELD_X(instr);
        setRegister(X, getRegister(X) - 1);
        break;

    case ADD:
    {
        // ADD X, Y, Z — add Y+Z; if result overflows 16 bits store both halves at [X] and [X+1]
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        const unsigned int sum = getRegister(Y) + getRegister(Z);
        if (sum > 0xFFFF)
        {
            setMemory(getRegister(X), static_cast<unsigned short>((sum & 0xFFFF0000) >> 16));
            setMemory(getRegister(X) + 1, static_cast<unsigned short>(sum & 0x0000FFFF));
            setFlag(C, true);
        }
        else
        {
            setMemory(getRegister(X), static_cast<unsigned short>(sum));
        }
        break;
    }

    case SUB:
    {
        // SUB X, Y, Z — subtract Z from Y; overflow handling mirrors ADD
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        const unsigned int diff = getRegister(Y) - getRegister(Z);
        if (diff > 0xFFFF)
        {
            setMemory(getRegister(X), static_cast<unsigned short>((diff & 0xFFFF0000) >> 16));
            setMemory(getRegister(X) + 1, static_cast<unsigned short>(diff & 0x0000FFFF));
            setFlag(C, true);
        }
        else
        {
            setMemory(getRegister(X), static_cast<unsigned short>(diff));
        }
        break;
    }

    case AND:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        setRegister(X, getRegister(Y) & getRegister(Z));
        break;

    case OR:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        setRegister(X, getRegister(Y) | getRegister(Z));
        break;

    case XOR:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        setRegister(X, getRegister(Y) ^ getRegister(Z));
        break;

    case NOT:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        setRegister(X, ~getRegister(Y));
        break;

    case SHL:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        setRegister(X, getRegister(Y) << getRegister(Z));
        break;

    case SHR:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        setRegister(X, getRegister(Y) >> getRegister(Z));
        break;

    case CMP:
    {
        // CMP X, Y — set G/L/E flags based on comparison; does not write to a register
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        const unsigned short xv = getRegister(X);
        const unsigned short yv = getRegister(Y);
        setFlag(E, xv == yv);
        setFlag(G, xv > yv);
        setFlag(L, xv < yv);
        break;
    }

    case JMP:
        addr = FIELD_IMM(instr);
        pc = addr;
        break;

    case JZ:
        addr = FIELD_IMM(instr);
        if (getFlag(Flags::Z))
            pc = addr;
        break;

    case JE:
        addr = FIELD_IMM(instr);
        if (getFlag(Flags::E))
            pc = addr;
        break;

    case JNE:
        addr = FIELD_IMM(instr);
        if (!getFlag(Flags::E))
            pc = addr;
        break;

    case JG:
        addr = FIELD_IMM(instr);
        if (getFlag(Flags::G))
            pc = addr;
        break;

    case JL:
        addr = FIELD_IMM(instr);
        if (getFlag(Flags::L))
            pc = addr;
        break;

    case CALL:
        // Push return address onto stack, then jump
        addr = FIELD_IMM(instr);
        setRegister(SP, getRegister(SP) - 1);
        setMemory(getRegister(SP), pc);
        pc = addr - 2; // -2 so the +2 at end of step() lands on addr
        break;

    case RET:
        pc = getMemory(getRegister(SP));
        setRegister(SP, getRegister(SP) + 1);
        break;

    case PUSH:
        X = FIELD_X(instr);
        setRegister(SP, getRegister(SP) - 1);
        setMemory(getRegister(SP), getRegister(X));
        break;

    case POP:
        X = FIELD_X(instr);
        setRegister(X, getMemory(getRegister(SP)));
        setRegister(SP, getRegister(SP) + 1);
        break;

    case HLT:
        running = false;
        pc -= 2; // cancel the +2 below so PC stays on the HLT
        break;

    case NOP:
        break;

    default:
        std::cerr << "Invalid opcode: 0x" << std::hex << static_cast<int>(opcode) << "\n";
        break;
    }

    pc += 2;
}

void virt16::load_program(const char* program) noexcept
{
    std::ifstream file(program, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << program << "\n";
        return;
    }

    unsigned short address = 0;
    unsigned short word;
    while (file.read(reinterpret_cast<char*>(&word), sizeof(word)))
    {
        setMemory(address++, word);
    }
}

void virt16::run()
{
    running = true;
    while (running)
        step();
}

void virt16::stop()
{
    running = false;
}

} // namespace Virt16
