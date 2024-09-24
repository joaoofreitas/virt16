'''
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

TOTAL: 32 Instructions (0x00 - 0x1F) 5 bits

Preprocessor Directives
.PLACE STRING addr "Hello, World!"; 
.PLACE ARRAY addr 0x00, 0x01, 0x02, 0x03, 0x04;

#IMM  ; Immediate value
%addr ; Content of the address
%reg  ; Content of the register

Comments are allowed with `;` at the end of the line.

@macro name [arg1, arg2, arg3]
    LOAD R1, arg1
    LOAD R2, arg2
    ADD R3, R1, R2
    STORE arg3, R3

@define name, value
    LOAD R1, name ; Same as LOAD R1, value 

RoutineName:        ; Label of the routine
    LOAD R1, #0x0001
    LOAD R2, #0x0002
    ADD R3, R1, R2
    STORE 0x0001, R3
    RET
'''

import sys
# Helpers folder contains cleaners.py with functions
from helpers.parser import *
from helpers.store import *
from helpers.parse_instructions import *

program = []
current_address = 0x0000

def assemble_instruction(instruction):
    # Split the instruction into parts
    parts = instruction.split()
    opcode = parts[0]  # The first part is the opcode (e.g., 'LOAD')
    args = []
    
    if len(parts) > 1:
        args = parts[1:]  # Remaining parts are the arguments
    
    # Check if the instruction is valid
    if opcode in instructions:
        if opcode == 'LOAD':
            return parse_load(opcode, args)
        else:
            print(f"Error: Unsupported instruction {opcode}")
            return 0
    else:
        print(f"Error: Unknown opcode {opcode}")
        return 0


if __name__ == '__main__':
    # Take source file as input
    if len(sys.argv) < 2:
        print("Usage: python assembler.py <source.asm>")
        sys.exit(1)

    with open(sys.argv[1], 'r') as f:
        lines = f.readlines()
    print("Original Program:")
    for line in lines:
        print(line, end='')
    print("=========================================")
    lines = remove_comments(lines)
    print("After removing comments:")
    for line in lines:
        print(line, end='')
    print("=========================================")
    lines = remove_empty_lines(lines)
    print("After removing empty lines:")
    for line in lines:
        print(line, end='')
    print("=========================================")

    lines = store_macros(lines)
    print("Macro Storage:")
    for line in lines:
        print(line, end='')

    print("=========================================")
    lines = store_definitions(lines)
    print("Definitions Storage:")
    for line in lines:
        print(line, end='')

    print("=========================================")
    lines = store_place(lines)
    print("Placed Data Storage:")
    for line in lines:
        print(line, end='')
    print("=========================================")#

    lines = substitute_macros_and_defs(lines)
    print("After substituting macros and definitions:")
    for line in lines:
        print(line, end='')
    print("=========================================")

    encapsulate_routine(lines)
    #assembled_program = assemble(lines)
    #print("\nAssembled Program:")
    #for line in assembled_program:
    #    print(line)

    for routine in routines:
        print(routine)
        for instruction in routines[routine]:
            print(instruction)
            # Print the assembled instruction in 32-bit binary
            print(f"{assemble_instruction(instruction):032b}")
            

    print("=========================================")
    print("\nRoutines:")
    for routine in routines:
        print(routine, routines[routine])

    print("\nMemory Contents:")
    for addr in memory:
        print(addr, memory[addr])


    print("\nMacros:")
    for macro in macros:
        print(macro, macros[macro])

    print("\nDefinitions:")
    for definition in definitions:
        print(definition, definitions[definition])

