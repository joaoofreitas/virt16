# Architecture of the Virt16

The Virt16 is a virtual 16-bit computer that is designed to be simple and easy to understand.

## Architecture
- **Word size**: 16 bits (2 bytes)
- **Memory size**: 32KB (32,768 bytes)
- **Architecture type**: Von Neumann architecture (program and data share the same memory)

## Memory
- **Words**: 16,384 (2 bytes each)
- **Address range**: 0x0000 - 0x3FFF

## Display (In Memory DMA)
- **Display Memory**: 0x3000 - 0x3FFF (4KB)

## Registers
- **State Registers**:
  - `PC`: Program Counter (RESERVED)
  - `SP`: Stack Pointer (RESERVED)
- **Time Register**:
  - `TIME`: Time Register
- **Accumulator**:
  - `A`: Accumulator
- **General Purpose Registers**:
  - `R1` - `R15`: General Purpose Registers
- **Peripheral Registers**:
  - `P1` - `P4`: Peripheral Registers (16 bits each)
- **Video Address Register**:
  - `DISP`: Display Register (Contains Address of Current Display Memory)
- **Flag Register**:
  - `Z`: Zero Flag (True if A is zero)
  - `G`: Greater Than Flag
  - `L`: Less Than Flag
  - `E`: Equal Flag

## Instruction Set
- **OPCODE**: 5 bits (0x00 - 0x1F)
- **REGISTER**: 5 bits (0x00 - 0x17)
- **IMMEDIATE**: 16 bits (0x00 - 0xFF)
- **Instruction Sizes**:
  - 20 bits: ADD/SUB/MUL/DIV/MOD/AND/OR/XOR/SHL/SHR/CMP
  - 26 bits: LOAD
  - 10 bits: INC/DEC/NOT/PUSH/POP/JMP/JZ/JE/JNE/JG/JL/RET
  - 5 bits: HLT/NOP/RET
  - 1 Instruction = 2 words (32 bits)

### Instructions
- `LOAD X, #imm`: Load immediate value into A
- `LOAD X, addr`: Load value from memory into A
- `STORE addr, X`: Store value from register into memory
- `STORE addr, #imm`: Store immediate value into memory
- `MOV X, Y`: Move value from Y to X
- `INC X`: Increment X by 1
- `DEC X`: Decrement X by 1
- `ADD addr, Y, Z`: Add Y and Z and store in addr and addr + 1
- `SUB addr, Y, Z`: Subtract Y from Z and store in addr and addr + 1
- `AND X, Y, Z`: Bitwise AND Y and Z and store in X
- `OR X, Y, Z`: Bitwise OR Y and Z and store in X
- `XOR X, Y, Z`: Bitwise XOR Y and Z and store in X
- `NOT X, Y`: Bitwise NOT Y and store in X
- `SHL X, Y, Z`: Shift Y left by Z bits and store in X
- `SHR X, Y, Z`: Shift Y right by Z bits and store in X
- `CMP X, Y, Z`: Compare Y and Z and store in X
- `JMP addr`: Jump to address
- `JZ addr`: Jump if zero
- `JE addr`: Jump if equal
- `JNE addr`: Jump if not equal
- `JG addr`: Jump if greater
- `JL addr`: Jump if less
- `CALL addr`: Call subroutine
- `RET`: Return from subroutine
- `PUSH X`: Push value from register onto stack
- `POP X`: Pop value from stack into register
- `HLT`: Halt the program
- `NOP`: No Operation

## Assembler
### Opcode Translation Table
| OPCode | Instruction      | Description                                      | Example               |
|--------|------------------|--------------------------------------------------|-----------------------|
| 0x00   | LOAD X, #IMM     | Load immediate value into X                      | LOAD R1, #0x0001      |
| 0x01   | LOAD X, addr     | Load value from memory into X                    | LOAD R1, 0x0001       |
| 0x02   | STORE addr, X    | Store value from X into memory                   | STORE 0x0001, R1      |
| 0x03   | MOV X, Y         | Move value from Y to X                           | MOV R1, R2            |
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

Preprocessor Directives
- `.PLACE addr "Hello, World!"`
- `.PLACE addr [0x00, 0x01, 0x02, 0x03, 0x04]`

Comments are allowed with `;` at the end of the line.

Example Macro:
```assembly
@macro name, arg1, arg2, arg3
    LOAD R1, arg1
    LOAD R2, arg2
    ADD R3, R1, R2
    STORE arg3, R3
@endmacro

@define name, value 
    LOAD R1, name ; Same as LOAD R1, value
```

### Virtual Machine

#### Graphics
- **Display**: 32x32 each address is a pixel color, 16 bits.
- **Total Pixels**: 32x32 = 1024 pixels
- **VRAM**: 2048 bytes (1024 pixels * 2 bytes per pixel)

#### Console
- **Grid**: 16x16 grid of text
- **Font Address**: 0x3100 (Font should be loaded here)
- **Console Interaction Memory**: Starts at 0x2900

### TODO

#### Assembler
- [ ] Fix @macros for multiple argument substitution
- [ ] Implement file imports
- [ ] Implement a basic standard library for ADD, SUB, MUL, DIV, MOD, and memory manipulation macros

#### VM
- [ ] Write tests for every single instruction to ensure proper functionality

#### Future Architecture Enhancements
- [ ] Add JC instruction to jump if the carry flag is set and handle math properly
- [ ] Add Overflow Flag and instructions to handle signed and unsigned values in the future



