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
| 0x01   | LOAD X, addr     | Load value from memory into X                    | LOAD R1, 0x0001       |
| 0x02   | STORE addr, X    | Store value from X into memory                   | STORE 0x0001, R1      |
| 0x03   | STORE addr, #IMM | Store immediate value into memory                | STORE 0x0001, #0x0001 |
| 0x04   | MOV X, Y         | Move value from Y to X                           | MOV R1, R2            |
| 0x05   | INC X            | Increment X by 1                                 | INC R1                |
| 0x06   | DEC X            | Decrement X by 1                                 | DEC R1                |
| 0x07   | ADD addr, X, Y   | Add X and Y and store in addr and addr + 1       | ADD 0x0001, R1, R2    |
| 0x08   | SUB addr, X, Y   | Subtract X from Y and store in addr and addr + 1 | SUB 0x0001, R1, R2    |
| 0x09   | MUL addr, X, Y   | Multiply X and Y and store in addr and addr + 1  | MUL 0x0001, R1, R2    |
| 0x0A   | MOD addr, X, Y   | Modulus X by Y and store in X (Remainder)        | MOD R1, R2, R3        |
| 0x0B   | AND X, Y, Z      | Bitwise AND Y and Z and store in X               | AND R1, R2, R3        |
| 0x0C   | OR X, Y, Z       | Bitwise OR Y and Z and store in X                | OR R1, R2, R3         |
| 0x0D   | XOR X, Y, Z      | Bitwise XOR Y and Z and store in X               | XOR R1, R2, R3        |
| 0x0E   | NOT X, Y         | Bitwise NOT Y and store in X                     | NOT R1, R2            |
| 0x0F   | SHL X, Y, Z      | Shift Y left by Z bits and store in X            | SHL R1, R2, R3        |
| 0x10   | SHR X, Y, Z      | Shift Y right by Z bits and store in X           | SHR R1, R2, R3        |
| 0x11   | CMP X, Y, Z      | Compare Y and Z and store in X                   | CMP R1, R2, R3        |
| 0x12   | JMP addr         | Jump to address                                  | JMP 0x0001            |
| 0x13   | JZ addr          | Jump if zero                                     | JZ 0x0001             |
| 0x14   | JE addr          | Jump if equal                                    | JE 0x0001             |
| 0x15   | JNE addr         | Jump if not equal                                | JNE 0x0001            |
| 0x16   | JG addr          | Jump if greater                                  | JG 0x0001             |
| 0x17   | JL addr          | Jump if less                                     | JL 0x0001             |
| 0x18   | CALL addr        | Call subroutine                                  | CALL 0x0001           |
| 0x19   | RET              | Return from subroutine                           | RET                   |
| 0x1A   | PUSH X           | Push value from register onto stack              | PUSH R1               |
| 0x1B   | POP X            | Pop value from stack into register               | POP R1                |
| 0x1C   | HLT              | Halt the program                                 | HLT                   |
| 0x1D   | NOP              | No Operation                                     | NOP                   |
| 0x1E   | NOP              | No Operation                                     | NOP                   |
| 0x1F   | NOP              | No Operation                                     | NOP                   |

TOTAL: 32 Instructions (0x00 - 0x1F) 5 bits
*/

#define LOAD_IMM 0x00
#define LOAD_ADDR 0x01
#define STORE_ADDR 0x02
#define STORE_IMM 0x03
#define MOV 0x04
#define INC 0x05
#define DEC 0x06
#define ADD 0x07
#define SUB 0x08
#define MUL 0x09
#define MOD 0x0A
#define AND 0x0B
#define OR 0x0C
#define XOR 0x0D
#define NOT 0x0E
#define SHL 0x0F
#define SHR 0x10
#define CMP 0x11
#define JMP 0x12
#define JZ 0x13
#define JE 0x14
#define JNE 0x15
#define JG 0x16
#define JL 0x17
#define CALL 0x18
#define RET 0x19
#define PUSH 0x1A
#define POP 0x1B
#define HLT 0x1C
#define NOP 0x1D

namespace Virt16 {
    virt16::virt16() {
        std::memset(memory, 0, sizeof(memory));
        std::memset(registers, 0, sizeof(registers));
        std::memset(peripherals, 0, sizeof(peripherals));

        time = 0;
        pc = 0;
        sp = 0;
        disp = 0;

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
        std::memset(peripherals, 0, sizeof(peripherals));

        time = 0;
        pc = 0;

        z = false;
        g = false;
        l = false;
        e = false;
    }

    unsigned short virt16::getMemory(int index) const {
        if (index >= 0 && index < MEMORY_SIZE) {
            return memory[index];
        }
        return 0; // or throw an exception
    }

    unsigned short virt16::getRegister(int index) const {
        if (index >= 0 && index < 16) {
            return this->registers[index];
        }
        return 0; // or throw an exception
    }

    bool virt16::getFlag(int flag) const {
        switch (flag) {
            case Z: return z;
            case G: return g;
            case L: return l;
            case E: return e;
            default: return false; // or throw an exception
        }
    }

    unsigned short virt16::getDisp() const {
        return disp;
    }

    // Setters
    void virt16::setMemory(int index, unsigned short value) {
        if (index >= 0 && index < MEMORY_SIZE) {
            memory[index] = value;
        }
        // else throw an exception
    }

    void virt16::setRegister(int index, unsigned short value) {
        if (index >= 0 && index < 16) {
            this->registers[index] = value;
        }
        // else throw an exception
    }

    void virt16::setFlag(const int flag, const bool value) {
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
        disp = value;
    }

    void virt16::step() {
        // Implement the step function
        const unsigned short instr_l = this->getMemory(this->pc);
        const unsigned short instr_h = this->getMemory(this->pc + 1);


        unsigned char X, Y, Z, addr;
        unsigned short imm;
        // OPCODE 5 bits (0x00 - 0x1F)
        // ADDR 16 bits
        // IMM 16 bits
        // REG 5 bits
        // Max 32 instructions (0x00 - 0x1F)
        const unsigned int instr = (instr_h << 8) | instr_l;
        // Opcode is the first 5 bits
        switch (unsigned char opcode = (instr_l & 0b11111000) >> 3) {
            case (LOAD_IMM):
                // X is the 5 bits after the opcode IMM is 16 bits after
                X = (instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5);
                imm = (instr & 0b00000000001111111111111111000000) >> (32 - 5 - 5 - 16);
                this->setRegister(X, imm);
                break;
            case (LOAD_ADDR):
                X = (instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5);
                addr = (instr & 0b00000000001111111111111111000000) >> (32 - 5 - 5 - 16);
                this->setRegister(X, this->getMemory(addr));
                break;
            case (STORE_ADDR):
                addr = (instr & 0b00000111111111111111100000000000) >> (32 - 16);
                X = (instr & 0b00000000000000000000011111000000) >> (32 - 16 - 5);
                this->setMemory(addr, this->getRegister(X));
                break;
            case (NOP):
                break;
            case (MOV):
                X = (instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5);
                Y = (instr & 0b00000000001111100000000000000000) >> (32 - 5 - 5 - 5);
                this->setRegister(X, this->getRegister(Y));
                break;
            case (INC):
                X = (instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5);
                this->setRegister(X, this->getRegister(X) + 1);
                break;
            case (DEC):
                X = (instr & 0b00000111110000000000000000000000) >> (32 - 5 - 5);
                this->setRegister(X, this->getRegister(X) - 1);
                break;
            // More to add... ALU
            case (HLT):
                this->running = false;
                break;
            case (JMP):
                addr = (instr & 0b00000111111111111111100000000000) >> (32 - 16);
                this->pc = addr;
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
        std::cout << "Loading program: " << program << std::endl;
        while (file && address < MEMORY_SIZE) {
            unsigned int data;
            // Read 2 bytes at a time
            file.read(reinterpret_cast<char *>(&data), 2);
            if (file) {
                this->setMemory(address++, data);
            }
        }
        file.close();
    }
} // Virt16
