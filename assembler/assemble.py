'''
| OPCode | Instruction      | Description                                      | Example               |
|--------|------------------|--------------------------------------------------|-----------------------|
| 0x00   | LOAD X, #IMM     | Load immediate value into X                      | LOAD R1, #0x0001      |
| 0x01   | LOAD X, addr     | Load value from memory into X                    | LOAD R1, 0x0001       |
| 0x02   | STORE X, Y       | Store value in Y into memory address in X        | STORE R1, R2          |
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
import os
# Helpers folder contains cleaners.py with functions
from helpers.parser import *
from helpers.store import *
from helpers.parse_instructions import *

source_code_filename = "program.asm"
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
        elif opcode == 'STORE':
            return parse_store(opcode, args)
        elif opcode == 'MOV':
            return parse_mov(opcode, args)
        elif opcode == 'INC':
            return parse_inc(opcode, args)
        elif opcode == 'DEC':
            return parse_dec(opcode, args)
        elif opcode == 'ADD':
            return parse_add(opcode, args)
        elif opcode == 'SUB':
            return parse_sub(opcode, args)
        elif opcode == 'AND':
            return parse_and(opcode, args)
        elif opcode == 'OR':
            return parse_or(opcode, args)
        elif opcode == 'XOR':
            return parse_xor(opcode, args)
        elif opcode == 'NOT':
            return parse_not(opcode, args)
        elif opcode == 'SHL':
            return parse_shl(opcode, args)
        elif opcode == 'SHR':
            return parse_shr(opcode, args)
        elif opcode == 'CMP':
            return parse_cmp(opcode, args)
        elif opcode == 'JMP':
            return parse_jmp(opcode, args)
        elif opcode == 'JZ':
            return parse_jz(opcode, args)
        elif opcode == 'JE':
            return parse_je(opcode, args)
        elif opcode == 'JNE':
            return parse_jne(opcode, args)
        elif opcode == 'JG':
            return parse_jg(opcode, args)
        elif opcode == 'JL':
            return parse_jl(opcode, args)
        elif opcode == 'CALL':
            return parse_call(opcode, args)
        elif opcode == 'RET':
            return parse_ret(opcode, args)
        elif opcode == 'PUSH':
            return parse_push(opcode, args)
        elif opcode == 'POP':
            return parse_pop(opcode, args)
        elif opcode == 'HLT':
            return parse_hlt(opcode, args)
        elif opcode == 'NOP':
            return parse_nop(opcode, args)
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

    # Remove extension from the filename
    source_code_filename = sys.argv[1]
    source_code_filename = source_code_filename.split('.')[0]


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
    
    # Now we take all the routines and assign addresses
    # .main is always at index 0
    # we iterate through the routines and assign addresses
    # address = current_address + len(routine[name])
    
    for routine in routines:
        if routine == '.main':
            allocated_routines[routine] = current_address
            current_address += len(routines[routine])
            
    for routine in routines:
        if routine == '.main':
            continue
        allocated_routines[routine] = current_address
        current_address += len(routines[routine])

    for alloc_r in allocated_routines:
        # Correct address since instructions are split in the real machine
        allocated_routines[alloc_r] = allocated_routines[alloc_r] * 2

    
    # Now we assemble the instructions and store them in the program
    for alloc_r in allocated_routines:
        # Assemble the instructions
        for instruction in routines[alloc_r]:
            program.append(assemble_instruction(instruction))

        
    # Create build directory if it doesn't exist
    if not os.path.exists('./build'):
        os.makedirs('./build')


    # Write the program in binary to a simple file.
    with open(f"./build/{source_code_filename}.bin", 'wb') as f:
        for instruction in program:
            # Write the instruction as 32 bits (4 bytes), keep zeros at the beginning if needed
            # f.write(instruction.to_bytes(4, byteorder='big', signed=False))
            upper = (instruction & 0xFFFF0000) >> 16
            lower = instruction & 0x0000FFFF
            print(hex(upper), hex(lower))
            f.write(upper.to_bytes(2, byteorder='little', signed=False)) # It is little because write will write in little endian which will turn in Big endian
            f.write(lower.to_bytes(2, byteorder='little', signed=False))

        for address, val in places.items():
            addr = int(address, 16)
            f.seek(addr)
            if isinstance(val, str):
                # Convert string to ASCII and write it to the file at the specified address
                ascii_values = [ord(c) for c in val]
                for ascii_val in ascii_values:
                    f.write(ascii_val.to_bytes(2, byteorder='little', signed=False))
            else:
                # Write array of hex values
                for hex_val in val:
                    value = int(hex_val, 16)
                    # Write as 16-bit values
                    f.write(value.to_bytes(2, byteorder='little', signed=False))
    
    # Write Data of the build to a file
    with open(f"./build/{source_code_filename}.data", 'w') as f:
        f.write("Routines:\n")
        for routine in routines:
            f.write(routine + "\n")
            for instruction in routines[routine]:
                f.write(instruction + "\n")
        f.write("\nAllocated Routines:\n")
        for routine in allocated_routines:
            f.write(routine + ": " + hex(allocated_routines[routine]) + "\n")
        f.write("\nMemory Contents:\n")
        for addr in memory:
            f.write(addr + ": " + str(memory[addr]) + "\n")
        f.write("\nMacros:\n")
        for macro in macros:
            f.write(macro + ": " + str(macros[macro]) + "\n")
        f.write("\nDefinitions:\n")
        for definition in definitions:
            f.write(definition + ": " + str(definitions[definition]) + "\n")
        f.write("\nPlaces:\n")
        for place in places:
            f.write(place + ": " + str(places[place]) + "\n")
        f.write("\nProgram Bin:\n")
        for addr, instruction in enumerate(program):
            f.write(hex(addr) + ": " + bin(instruction) + "\n")
        f.write("\nProgram Hex:\n")
        for addr, instruction in enumerate(program):
            f.write(hex(addr) + ": " +hex(instruction) + "\n")

    # For Virtual Machine Followup
    with open(f"./build/{source_code_filename}.debug", 'w') as f:
        # Write Data for Debug in VM
        for routine in allocated_routines:
            for instruction in routines[routine]:
                f.write(instruction + "\n")

    print("=========================================")
    print("\nRoutines:")
    for routine in routines:
        print(routine, routines[routine])

    print("\nAllocated Routines:")
    for routine in allocated_routines:
        print(routine + ": " + hex(allocated_routines[routine])) 

    print("\nMemory Contents:")
    for addr in memory:
        print(addr, memory[addr])

    print("\nMacros:")
    for macro in macros:
        print(macro, macros[macro])

    print("\nDefinitions:")
    for definition in definitions:
        print(definition, definitions[definition])
    
    print("\nPlaces:")
    for place in places:
        print(place, places[place])
    
    print("Program:")
    for instruction in program:
        print(bin(instruction))
