# Architecture of the Virt16

The Virt16 is a virtual 16 bit computer that is designed to be simple and easy to understand.

## Architecture
Word size: 16 bits (2 bytes)
Memory size: 32KB (32 768 bytes)

Van Neumann architecture so the program and data share the same memory.

## Memory
2 bytes / 32 768 bytes = 16384 Words
Address range: 0x0000 - 0x3FFF

## Display (In Memory DMA)
- `0x3000 - 0x3FFF` Display Memory (4KB)

## Registers
(2 State Registers)
- `PC` Program Counter (RESERVED)
- `SP` Stack Pointer   (RESERVED)

(1 Time Register)
- `TIME` Time Register

(1 Accumulator)
- `A` Accumulator

(16 General Purpose Registers)
- `R1` General Purpose Register
- `R2` General Purpose Register
- `R3` General Purpose Register
- `R4` General Purpose Register
- `R5` General Purpose Register
- `R6` General Purpose Register
- `R7` General Purpose Register
- `R8` General Purpose Register
- `R9` General Purpose Register
- `R10` General Purpose Register
- `R11` General Purpose Register
- `R12` General Purpose Register
- `R13` General Purpose Register
- `R14` General Purpose Register
- `R15` General Purpose Register


(4 Peripheral Registers)
- `P1` Peripheral Register 1 (16 bits)
- `P2` Peripheral Register 2 (16 bits)
- `P3` Peripheral Register 3 (16 bits)
- `P4` Peripheral Register 4 (16 bits)

(1 Video Address Registers)
- `DISP` Display Register (Contains Address of Current Display Memory)

(1 Flag Register)

TOTAL: 24 Registers
(0x0 - 0x17)

## Flags
- `Z` Zero Flag (True if A is zero)
- `G` Greater Than Flag
- `L` Less Than Flag
- `E` Equal Flag

## Instruction Set (32 Instructions)
   OPCODE for 32 instructions is 5 bits (0x00 - 0x1F)
   REGISTER for 24 registers is 5 bits (0x00 - 0x17)
   IMMEDIATE for 16 bit immediate value is 16 bits (0x00 - 0xFF)

   INSTRUCTION SIZES = 5 (OPCODE) + 5 (REGISTER) + 5 (REGISTER) + 5 (REGISTER) = 20 bits (ADD/SUB/MUL/DIV/MOD/AND/OR/XOR/SHL/SHR/CMP)
   INSTRUCTION SIZES = 5 (OPCODE) + 5 (REGISTER) + 16 (IMMEDIATE) = 26 bits (LOAD)
   INSTRUCTION SIZES = 5 (OPCODE) + 5 (REGISTER)  (INC/DEC/NOT/PUSH/POP/JMP/JZ/JE/JNE/JG/JL/RET) = 10 bits
   INSTRUCTION SIZES = 5 (OPCODE)  (HLT/NOP/RET) = 5 bits

   Depending on OPCODE the rest of the bits will be ignored.
   1 Instruction = 2 words (32 bits)

- `LOAD X, #imm` Load immediate value into A
- `LOAD X, addr` Load value from memory into A
- `STORE addr, X` Store value from register into memory
- `STORE addr, #imm` Store immediate value into memory
- `MOV X, Y` Move value from Y to X
- `INC X` Increment X by 1
- `DEC X` Decrement X by 1
- `ADD addr, Y, Z` Add Y and Z and store in addr and addr + 1
- `SUB addr, Y, Z` Subtract Y from Z and store in addr and addr + 1
- `MUL addr, Y, Z` Multiply Y and Z and store in addr and addr + 1
- `MOD addr, Y, Z` Modulus Y by Z and store in X (Remainder)
- `AND X, Y, Z` Bitwise AND Y and Z and store in X
- `OR X, Y, Z` Bitwise OR Y and Z and store in X
- `XOR X, Y, Z` Bitwise XOR Y and Z and store in X
- `NOT X, Y` Bitwise NOT Y and store in X
- `SHL X, Y, Z` Shift Y left by Z bits and store in X
- `SHR X, Y, Z` Shift Y right by Z bits and store in X
- `CMP X, Y, Z` Compare Y and Z and store in X
- `JMP addr` Jump to address
- `JZ addr` Jump if zero
- `JE addr` Jump if equal
- `JNE addr` Jump if not equal
- `JG addr` Jump if greater
- `JL addr` Jump if less
- `CALL addr` Call subroutine
- `RET` Return from subroutine
- `PUSH X` Push value from register onto stack
- `POP X` Pop value from stack into register
- `HLT` Halt the program
- `NOP` No Operation
- `NOP` No Operation

## Assembler

## Opcode Translation Table
| OPCode | Instruction      | Description                                      | Example               |
|--------|------------------|--------------------------------------------------|-----------------------|
| 0x00   | LOAD X, #IMM     | Load immediate value into X                      | LOAD R1, #0x0001      |
| 0x01   | LOAD X, addr     | Load value from memory into X                    | LOAD R1, 0x0001       |
| 0x02   | STORE addr, X    | Store value from X into memory                   | STORE 0x0001, R1      |
| 0x03   | MOV X, Y         | Move value from Y to X                           | MOV R1, R2            |
| 0x04   | INC X            | Increment X by 1                                 | INC R1                |
| 0x05   | DEC X            | Decrement X by 1                                 | DEC R1                |
| 0x06   | ADD addr, X, Y   | Add X and Y and store in addr and addr + 1       | ADD 0x0001, R1, R2    |
| 0x07   | SUB addr, X, Y   | Subtract X from Y and store in addr and addr + 1 | SUB 0x0001, R1, R2    |
| 0x08   | MUL addr, X, Y   | Multiply X and Y and store in addr and addr + 1  | MUL 0x0001, R1, R2    |
| 0x09   | MOD addr, X, Y   | Modulus X by Y and store in X (Remainder)        | MOD R1, R2, R3        |
| 0x0A   | AND X, Y, Z      | Bitwise AND Y and Z and store in X               | AND R1, R2, R3        |
| 0x0B   | OR X, Y, Z       | Bitwise OR Y and Z and store in X                | OR R1, R2, R3         |
| 0x0C   | XOR X, Y, Z      | Bitwise XOR Y and Z and store in X               | XOR R1, R2, R3        |
| 0x0D   | NOT X, Y         | Bitwise NOT Y and store in X                     | NOT R1, R2            |
| 0x0E   | SHL X, Y, Z      | Shift Y left by Z bits and store in X            | SHL R1, R2, R3        |
| 0x0F   | SHR X, Y, Z      | Shift Y right by Z bits and store in X           | SHR R1, R2, R3        |
| 0x10   | CMP X, Y, Z      | Compare Y and Z and store in X                   | CMP R1, R2, R3        |
| 0x11   | JMP addr         | Jump to address                                  | JMP 0x0001            |
| 0x12   | JZ addr          | Jump if zero                                     | JZ 0x0001             |
| 0x13   | JE addr          | Jump if equal                                    | JE 0x0001             |
| 0x14   | JNE addr         | Jump if not equal                                | JNE 0x0001            |
| 0x15   | JG addr          | Jump if greater                                  | JG 0x0001             |
| 0x16   | JL addr          | Jump if less                                     | JL 0x0001             |
| 0x17   | CALL addr        | Call subroutine                                  | CALL 0x0001           |
| 0x18   | RET              | Return from subroutine                           | RET                   |
| 0x19   | PUSH X           | Push value from register onto stack              | PUSH R1               |
| 0x1A   | POP X            | Pop value from stack into register               | POP R1                |
| 0x1B   | HLT              | Halt the program                                 | HLT                   |
| 0x1C   | NOP              | No Operation                                     | NOP                   |
| 0x1D   | NOP              | No Operation                                     | NOP                   |
| 0x1E   | NOP              | No Operation                                     | NOP                   |
| 0x1F   | NOP              | No Operation                                     | NOP                   |

TOTAL: 32 Instructions (0x00 - 0x1F) 5 bits

Preprocessor Directives
.PLACE STRING addr "Hello, World!"; 
.PLACE ARRAY addr 0x00, 0x01, 0x02, 0x03, 0x04;

Comments are allowed with `;` at the end of the line.

@macro name, arg1, arg2, arg3
    LOAD R1, arg1
    LOAD R2, arg2
    ADD R3, R1, R2
    STORE arg3, R3

@define name, value
    LOAD R1, name ; Same as LOAD R1, value 


## Virtual Machine


Display. Must be squared. 32x32 each address is a pixel color, 16 bits.
32x32 = 1024 pixels
VRAM = 1024 * 2 = 2048 bytes
