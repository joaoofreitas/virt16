//
// Created by johnny on 22/09/24.
//

#include <cstring>
#include "virt16.h"

#include <fstream>
#include <iostream>


/*
## Opcode Translation Table
| OPCode | Instruction      | Description                                      | Example               |
|--------|------------------|--------------------------------------------------|-----------------------|
| 0x00   | LOAD X, #IMM     | Load immediate value into X                      | LOAD R1, #0x0001      |
| 0x01   | LOAD X, Y        | Load value from memory into X                    | LOAD R0, R1           |
| 0x02   | STORE X, Y       | Store value from Y into memory address in X      | STORE R0, R1          |
| 0x03   | MOV X, Y         | Move value from Y to X                           | MOV R0, R1            |
| 0x04   | INC X            | Increment X by 1                                 | INC R1                |
| 0x05   | DEC X            | Decrement X by 1                                 | DEC R1                |
| 0x06   | ADD X, Y, Z      | Add Y and Z and store in addr and addr+1 at X    | ADD R1, R2, R3        |
| 0x07   | SUB X, Y, Z      | Sub Z from Y and store in addr and addr+1 at X   | SUB R1, R2, R2        |
| 0x08   | AND X, Y, Z      | Bitwise AND Y and Z and store in X               | AND R1, R2, R3        |
| 0x09   | OR X, Y, Z       | Bitwise OR Y and Z and store in X                | OR R1, R2, R3         |
| 0x0A   | XOR X, Y, Z      | Bitwise XOR Y and Z and store in X               | XOR R1, R2, R3        |
| 0x0B   | NOT X, Y         | Bitwise NOT Y and store in X                     | NOT R1, R2            |
| 0x0C   | SHL X, Y, Z      | Shift Y left by Z bits and store in X            | SHL R1, R2, R3        |
| 0x0D   | SHR X, Y, Z      | Shift Y right by Z bits and store in X           | SHR R1, R2, R3        |
| 0x0E   | CMP X, Y         | Compare X and Y - Flags will pop up              | CMP R1, R2            |
| 0x0F   | JMP addr         | Jump to address                                  | JMP 0x0001            |
| 0x10   | JZ addr          | Jump if zero                                     | JZ 0x0001             |
| 0x11   | JE addr          | Jump if equal                                    | JE 0x0001             |
| 0x12   | JNE addr         | Jump if not equal                                | JNE 0x0001            |
| 0x13   | JG addr          | Jump if greater                                  | JG 0x0001             |
| 0x14   | JL addr          | Jump if less                                     | JL 0x0001             |
| 0x15   | CALL addr        | Call subroutine                                  | CALL 0x0001           |
| 0x16   | RET              | Return from subroutine                           | RET                   |
| 0x17   | PUSH X           | Push value from register onto stack              | PUSH R1               |
| 0x18   | POP X            | Pop value from stack into register               | POP R1                |
| 0x19   | HLT              | Halt the program                                 | HLT                   |
| 0x1A   | NOP              | No Operation                                     | NOP                   |
| 0x1B   | NOP              | No Operation                                     | NOP                   |
| 0x1C   | NOP              | No Operation                                     | NOP                   |
| 0x1D   | NOP              | No Operation                                     | NOP                   |
| 0x1E   | NOP              | No Operation                                     | NOP                   |
| 0x1F   | NOP              | No Operation                                     | NOP                   |

TOTAL: 32 Instructions (0x00 - 0x1F) 5 bits
*/

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


namespace Virt16 {
    virt16::virt16() {
        std::memset(memory, 0, sizeof(memory));
        std::memset(registers, 0, sizeof(registers));
        pc = 0;

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

        pc = 0;

        z = false;
        g = false;
        l = false;
        e = false;
    }

    unsigned short virt16::getMemory(const unsigned int addr) const {
        return this->memory[addr];
    }

    unsigned short virt16::getRegister(const Registers reg) const {
        return this->registers[reg];
    }

    bool virt16::getFlag(Flags flag) const {
        switch (flag) {
            case Z: return z;
            case G: return g;
            case L: return l;
            case E: return e;
            default: return false; // or throw an exception
        }
    }

    unsigned short virt16::getDisp() const {
        return this->getRegister(DISP);
    }

    unsigned short virt16::getPC() const {
        return this->pc;
    }

    // Setters
    void virt16::setMemory(const unsigned int addr, const unsigned short value) {
        this->memory[addr] = value;
    }

    void virt16::setRegister(const Registers reg, unsigned short value) {
        this->registers[reg] = value;
    }

    void virt16::setFlag(const Flags flag, const bool value) {
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

    void virt16::setDisp(const unsigned short value) {
        this->setRegister(DISP, value);
    }

    void virt16::step() {
        // Implement the step function (big endian)
        const unsigned short instr_l = this->getMemory(this->pc);
        const unsigned short instr_h = this->getMemory(this->pc + 1);


        Registers X, Y, Z;
        unsigned short addr;
        unsigned short imm;
        // OPCODE 5 bits (0x00 - 0x1F)
        // ADDR 16 bits
        // IMM 16 bits
        // REG 5 bits
        // Max 32 instructions (0x00 - 0x1F)
        // Instruction is 32 bits, first 16 bits is instr_l and last 16 bits is instr_h
        const unsigned int instr = (instr_l << 16) | instr_h;
        // Opcode is the first 5 bits
        switch (unsigned char opcode = (instr & 0xf8000000) >> 27) {
            case (LOAD_IMM):
                // OPCODE (5 bits) | REG (5 bits) | Empty (6 bits) | IMM (16 bits) - 32 bits Big Endian (MSB) Read Left to Right
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> 22);
                imm = (instr & 0x0000FFFF);
                this->setRegister(X, imm);
                break;
            case (LOAD_ADDR):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                this->setRegister(X, this->getMemory(Y));
                break;
            case (STORE_ADDR):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                this->setMemory(this->getRegister(X), this->getRegister(Y));
            break;
            case (MOV):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
            Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
            this->setRegister(X, this->getRegister(Y));
            break;
            case (INC):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
            this->setRegister(X, this->getRegister(X) + 1);
            break;
            case (DEC):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
            this->setRegister(X, this->getRegister(X) - 1);
            break;
            case (ADD): {
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                Z = static_cast<Registers>((instr & 0b00000000000000011111000000000000) >> (32 - 5 - 5 - 5 - 5));

                if (const unsigned int sum = this->getRegister(Y) + this->getRegister(Z); sum > 0xFFFF) {
                    const unsigned short sum_msb = (sum & 0xFFFF0000) >> 16;
                    const unsigned short sum_lsb = sum & 0x0000FFFF;

                    this->setMemory(this->getRegister(X), sum_msb);
                    this->setMemory(this->getRegister(X)+1, sum_lsb);
                    this->setFlag(C, true);
                } else {
                    this->setMemory(this->getRegister(X), sum);
                }

                break;
            }
            case (SUB): {
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                Z = static_cast<Registers>((instr & 0b00000000000000011111000000000000) >> (32 - 5 - 5 - 5 - 5));

                // Check if carry result is larger than 2 bytes
                if (const unsigned int diff = this->getRegister(Y) - this->getRegister(Z); diff > 0xFFFF) {
                    const unsigned short diff_msb = (diff & 0xFFFF0000) >> 16;
                    const unsigned short diff_lsb = diff & 0x0000FFFF;

                    this->setMemory(this->getRegister(X), diff_msb);
                    this->setMemory(this->getRegister(X)+1, diff_lsb);
                    this->setFlag(Flags::C, true);
                } else {
                    this->setMemory(this->getRegister(X), diff);
                }

                break;
            }
            case (AND):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                Z = static_cast<Registers>((instr & 0b00000000000000011111000000000000) >> (32 - 5 - 5 - 5 - 5));

                this->setRegister(X, this->getRegister(Y) & this->getRegister(Z));
                break;

            case (OR):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                Z = static_cast<Registers>((instr & 0b00000000000000011111000000000000) >> (32 - 5 - 5 - 5 - 5));

                this->setRegister(X, this->getRegister(Y) | this->getRegister(Z));
                break;

            case (XOR):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                Z = static_cast<Registers>((instr & 0b00000000000000011111000000000000) >> (32 - 5 - 5 - 5 - 5));

                this->setRegister(X, this->getRegister(Y) ^ this->getRegister(Z));
                break;

            case (NOT):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                this->setRegister(X, ~this->getRegister(Y));
                break;

            case (SHL):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                Z = static_cast<Registers>((instr & 0b00000000000000011111000000000000) >> (32 - 5 - 5 - 5 - 5));

                this->setRegister(X, this->getRegister(Y) << this->getRegister(Z));
                break;
            case (SHR):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));
                Z = static_cast<Registers>((instr & 0b00000000000000011111000000000000) >> (32 - 5 - 5 - 5 - 5));

                this->setRegister(X, this->getRegister(Y) >> this->getRegister(Z));
                break;

            case (CMP):
            {
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                Y = static_cast<Registers>((instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5));

                const unsigned short x_val = this->getRegister(X);
                const unsigned short y_val = this->getRegister(Y);

                if (x_val == y_val) {
                    this->setFlag(E, true);
                }
                if (x_val > y_val) {
                    this->setFlag(G, true);
                }
                if (x_val < y_val) {
                    this->setFlag(L, true);
                }
                break;
            }
            case (JMP):
                addr = (instr & 0xFFFF);
                this->pc = addr;
                break;
            case (JZ):
                addr = (instr & 0xFFFF);
                if (this->getFlag(Virt16::Z)) {
                    this->pc = addr;
                }
                break;
            case (JE):
                addr = (instr & 0xFFFF);
                if (this->getFlag(Virt16::E)) {
                    this->pc = addr;
                }
                break;
            case (JNE):
                addr = (instr & 0xFFFF);
                if (!this->getFlag(Virt16::E)) {
                    this->pc = addr;
                }
                break;
            case (JG):
                addr = (instr & 0xFFFF);
                if (this->getFlag(Virt16::G)) {
                    this->pc = addr;
                }
                break;
            case (JL):
                addr = (instr & 0xFFFF);
                if (this->getFlag(Virt16::L)) {
                    this->pc = addr;
                }
                break;
            case (CALL):
                addr = (instr & 0xFFFF);
                this->setRegister(SP, this->getRegister(SP) - 1);
                this->setMemory(this->getRegister(SP), this->pc);
                this->pc = addr - 2;
                break;
            case (RET):
                this->pc = this->getMemory(this->getRegister(SP));
                this->setRegister(SP, this->getRegister(SP) + 1);
                break;
            case (PUSH):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                this->setRegister(SP, this->getRegister(SP) - 1);
                this->setMemory(this->getRegister(SP), this->getRegister(X));
                break;
            case (POP):
                X = static_cast<Registers>((instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5));
                this->setRegister(X, this->getMemory(this->getRegister(SP)));
                this->setRegister(SP, this->getRegister(SP) + 1);
                break;
            case (HLT):
                this->running = false;
                this->pc -= 2;
                break;
            case (NOP):
                break;
            default:
                // Invalid opcode
                std::cout << "Invalid opcode: " << opcode << std::endl;
                break;
        }
        this->pc += 2;
    }

    void virt16::load_program(const char *program) noexcept {
        // Open the file in binary mode
        std::ifstream file(program, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << program << std::endl;
            //throw std::runtime_error("Failed to open file");
        }

        // Read the file contents into the memory array
        unsigned short address = 0;
        unsigned short data;
        std::cout << "Loading program: " << program << std::endl;
        while (file) {
            // Read 2 bytes at a time
            file.read(reinterpret_cast<char *>(&data), 2);
            if (file) {
                this->setMemory(address++, data);
            }
        }
        file.close();
    }

    void virt16::run() {
        this->running = true;
        while (this->running) {
            this->step();
        }
    }

    void virt16::stop() {
        this->running = false;
    }
} // Virt16
