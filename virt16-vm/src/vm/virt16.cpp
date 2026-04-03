#include "virt16.hpp"

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
#define JC 0x1B
#define EI 0x1C
#define DI 0x1D
#define RETI 0x1E

// Instruction field extraction helpers.
// Each instruction is 32 bits: [OPCODE:5][X:5][Y:5][Z:5][IMM/ADDR:16] (big-endian, MSB first).
#define FIELD_X(instr) static_cast<Registers>(((instr) & 0b00000111110000000000000000000000) >> 22)
#define FIELD_Y(instr) static_cast<Registers>(((instr) & 0b00000000001111100000000000000000) >> 17)
#define FIELD_Z(instr) static_cast<Registers>(((instr) & 0b00000000000000011111000000000000) >> 12)
#define FIELD_IMM(instr) static_cast<unsigned short>((instr) & 0x0000FFFF)

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

    const unsigned char opcode = (instr & 0xF8000000) >> 27;

    Registers X, Y, Z;
    unsigned short addr, imm;

    switch (opcode)
    {
    case LOAD_IMM:
        // LOAD X, #imm — load 16-bit immediate into register X
        X = FIELD_X(instr);
        imm = FIELD_IMM(instr);
        registers[X] = imm;
        break;

    case LOAD_ADDR:
        // LOAD X, Y — load the word at the address stored in Y into X
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        registers[X] = memory[registers[Y]];
        break;

    case STORE_ADDR:
        // STORE X, Y — store value of Y into the memory address stored in X
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        memory[registers[X]] = registers[Y];
        break;

    case MOV:
        // MOV X, Y — copy register Y into X
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        registers[X] = registers[Y];
        break;

    case INC:
        X = FIELD_X(instr);
        registers[X]++;
        z = registers[X] == 0;
        break;

    case DEC:
        X = FIELD_X(instr);
        registers[X]--;
        z = registers[X] == 0;
        break;

    case ADD:
    {
        // ADD X, Y, Z — result stored directly in register X; C set on 16-bit overflow
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        const unsigned int sum = registers[Y] + registers[Z];
        const unsigned short result = static_cast<unsigned short>(sum & 0xFFFF);
        registers[X] = result;
        c = sum > 0xFFFF;
        z = result == 0;
        break;
    }

    case SUB:
    {
        // SUB X, Y, Z — result stored directly in register X; C set on borrow (underflow)
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        const bool borrow = registers[Y] < registers[Z];
        const unsigned short result = static_cast<unsigned short>((registers[Y] - registers[Z]) & 0xFFFF);
        registers[X] = result;
        c = borrow;
        z = result == 0;
        break;
    }

    case AND:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        registers[X] = registers[Y] & registers[Z];
        break;

    case OR:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        registers[X] = registers[Y] | registers[Z];
        break;

    case XOR:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        registers[X] = registers[Y] ^ registers[Z];
        break;

    case NOT:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        registers[X] = ~registers[Y];
        break;

    case SHL:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        registers[X] = registers[Y] << registers[Z];
        break;

    case SHR:
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        Z = FIELD_Z(instr);
        registers[X] = registers[Y] >> registers[Z];
        break;

    case CMP:
    {
        // CMP X, Y — clear all comparison flags then set based on result
        X = FIELD_X(instr);
        Y = FIELD_Y(instr);
        e = g = l = false;
        e = registers[X] == registers[Y];
        g = registers[X] > registers[Y];
        l = registers[X] < registers[Y];
        break;
    }

    case JMP:
        addr = FIELD_IMM(instr);
        pc = addr - 2; // -2 so the +2 at end of step() lands on addr
        break;

    case JZ:
        addr = FIELD_IMM(instr);
        if (z)
            pc = addr - 2;
        break;

    case JE:
        addr = FIELD_IMM(instr);
        if (e)
            pc = addr - 2;
        break;

    case JNE:
        addr = FIELD_IMM(instr);
        if (!e)
            pc = addr - 2;
        break;

    case JG:
        addr = FIELD_IMM(instr);
        if (g)
            pc = addr - 2;
        break;

    case JL:
        addr = FIELD_IMM(instr);
        if (l)
            pc = addr - 2;
        break;

    case CALL:
        // Push return address onto stack, then jump
        addr = FIELD_IMM(instr);
        registers[SP]--;
        memory[registers[SP]] = pc;
        pc = addr - 2; // -2 so the +2 at end of step() lands on addr
        break;

    case RET:
        pc = memory[registers[SP]];
        registers[SP]++;
        break;

    case PUSH:
        X = FIELD_X(instr);
        registers[SP]--;
        memory[registers[SP]] = registers[X];
        break;

    case POP:
        X = FIELD_X(instr);
        registers[X] = memory[registers[SP]];
        registers[SP]++;
        break;

    case HLT:
        running = false;
        pc -= 2; // cancel the +2 below so PC stays on the HLT
        break;

    case JC:
        addr = FIELD_IMM(instr);
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
        pc = memory[registers[SP]];
        registers[SP]++;
        i = true;
        break;

    case NOP:
        break;

    default:
        std::cerr << "invalid opcode: 0x" << std::hex << static_cast<int>(opcode) << "\n";
        this->stop();
        break;
    }

    pc += 2;

    registers[TIME]++;
    if (registers[TPER] != 0 && registers[TIME] == registers[TPER])
    {
        registers[TIME] = 0;
        timer_pending = true;
    }

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
        std::cerr << "failed to open file: " << program << "\n";
        return;
    }

    unsigned short address = 0;
    unsigned short word;
    while (file.read(reinterpret_cast<char*>(&word), sizeof(word)))
    {
        memory[address++] = word;
    }
}

void virt16::start()
{
    running = true;
}

void virt16::run_for_steps(unsigned int max_steps)
{
    if (!running)
        return;

    for (unsigned int i = 0; i < max_steps && running; ++i)
    {
        step();
    }
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
