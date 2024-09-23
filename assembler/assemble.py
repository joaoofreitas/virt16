'''
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

# We will be writing a simple assembler for the above instructions.

# First, we need to define the instructions and their opcodes.
# We will use a dictionary to store the instructions and their opcodes.
# The key will be the instruction and the value will be the opcode.
instructions = {
    'LOAD': 0x00,
    'STORE': 0x02,
    'MOV': 0x03,
    'INC': 0x04,
    'DEC': 0x05,
    'ADD': 0x06,
    'SUB': 0x07,
    'MUL': 0x08,
    'MOD': 0x09,
    'AND': 0x0A,
    'OR': 0x0B,
    'XOR': 0x0C,
    'NOT': 0x0D,
    'SHL': 0x0E,
    'SHR': 0x0F,
    'CMP': 0x10,
    'JMP': 0x11,
    'JZ': 0x12,
    'JE': 0x13,
    'JNE': 0x14,
    'JG': 0x15,
    'JL': 0x16,
    'CALL': 0x17,
    'RET': 0x18,
    'PUSH': 0x19,
    'POP': 0x1A,
    'HLT': 0x1B,
    'NOP': 0x1C,
}

# Next, we need to define the registers and their numbers.

registers = {
    'R0': 0x00,
    'R1': 0x01,
    'R2': 0x02,
    'R3': 0x03,
    'R4': 0x04,
    'R5': 0x05,
    'R6': 0x06,
    'R7': 0x07,
    'R8': 0x08,
    'R9': 0x09,
    'R10': 0x0A,
    'R11': 0x0B,
    'R12': 0x0C,
    'R13': 0x0D,
    'R14': 0x0E,
    'R15': 0x0F,

    'SP': 0x10,
    'DISP': 0x11,
    'TIME': 0x12,
    'A': 0x13,
    'P1': 0x14,
    'P2': 0x15, 
    'P3': 0x16,
    'P4': 0x17,
}

# Now, we need to define the preprocessor directives.
# We will use a dictionary to store the directives and their opcodes.
# The key will be the directive and the value will be the opcode.
directives = {
    '.PLACE': 0x20,
    '@macro': 0x21,
    '@define': 0x22,
}

import sys
import re

macros = {}
definitions = {}
labels = {}
memory = {}
routines = {}

program = []
current_address = 0x0000

def remove_comments(lines):
    return [re.sub(r';.*', '', line) for line in lines]

def remove_empty_lines(lines):
    return [line for line in lines if line.strip()]

def store_macros(lines):
    global macros
    for line in lines:
        if line.startswith('@macro'):
            parts = line.split()
            name = parts[1]
            # args go inside [ ] and are separated by commas
            args = parts[2].strip('[]').split(',')
            # remove args empty strings
            args = [arg for arg in args if arg]
            # body is everything after the first line until @endmacro
            body = []
            for line in lines[lines.index(line)+1:]:
                if line.startswith('@endmacro'):
                    break
                body.append(line)
                # Remove line from lines
                lines.remove(line)

            # Remove whitespace and \n from the args and body
            args = [arg.strip() for arg in args]
            body = [line.strip() for line in body]
            macros[name] = (args, body)
    
    # Remove all @macro and @endmacro lines
    lines = [line for line in lines if not line.startswith('@macro') and not line.startswith('@endmacro')]

    return lines



def store_definitions(lines):
    global definitions
    for line in lines:
        if line.startswith('@define'):
            parts = line.split()
            name = parts[1]
            value = parts[2]
            definitions[name] = value
    # Remove all @define lines
    lines = [line for line in lines if not line.startswith('@define')]
    return lines

def store_place(lines):
    global memory
    for line in lines:
        if line.startswith('.PLACE'):
            parts = line.split()
            addr = parts[1]
            data = parts[2]

            memory[addr] = data
    # Remove all .PLACE lines
    lines = [line for line in lines if not line.startswith('.PLACE')]
    return lines

def substitute_macros_and_defs(lines):
    # if line is a macro (starts with @), replace it with the body
    for i, line in enumerate(lines):
        # line starts with @
        matches = re.findall(r'@\w+', line)
        for match in matches:
            name = match[1:]
            if name in macros:
                # Check if the macro is defined
                args, body = macros[name]
                b_copy = body.copy()
                l = line.strip().split()
                if (len(l) - 1) != len(args):
                    print(f"Error: Incorrect number of arguments for macro {name}")
                    break

                # Replace args with values
                for j, arg in enumerate(args):
                    for k, word in enumerate(b_copy):
                        b_copy[k] = re.sub(arg, l[j+1], word)

                # Insert body into lines
                lines[i] = '    ' + b_copy[0] + '\n'
                for line in b_copy[1:]:
                    lines.insert(i+1, '    ' + line + '\n')

            else:
                print(f"Error: Macro {match} not defined")
        


    # Definitions start with %NAME and are replaced with the value
    for i, line in enumerate(lines):
        matches = re.findall(r'%\w+', line)
        for match in matches:
            name = match[1:]
            if name in definitions:
                lines[i] = re.sub(match, definitions[name], lines[i])
            else:
                print(f"Error: Definition {match} not defined")

    return lines



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

    #assembled_program = assemble(lines)
    #print("\nAssembled Program:")
    #for line in assembled_program:
    #    print(line)

    print("\nMemory Contents:")
    for addr in memory:
        print(addr, memory[addr])


    print("\nMacros:")
    for macro in macros:
        print(macro, macros[macro])

    print("\nDefinitions:")
    for definition in definitions:
        print(definition, definitions[definition])

