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
program = []
current_address = 0x0000
memory = {}  # Define memory as a dictionary to store placed data

def parse_line(line):
    tokens = line.strip().split()

    if not tokens:
        return None

    # Handle instructions
    if tokens[0] in instructions:
        opcode = instructions[tokens[0]]
        args = tokens[1:]
        return ('instruction', opcode, args)

    # Handle labels
    elif tokens[0].endswith(':'):
        label = tokens[0][:-1]
        labels[label] = current_address
        return ('label', label)

    # Handle directives
    elif tokens[0] in directives:
        directive = tokens[0]
        args = tokens[1:]
        return ('directive', directive, args)

    else:
        return ('error', f"Invalid instruction {tokens[0]}")

def handle_define(args):
    if len(args) != 2:
        print(f"Error: Invalid define syntax")
        return
    name = args[0]
    value = args[1]
    if value.startswith('0x'):
        value = int(value, 16)
    else:
        value = int(value)
    definitions[name] = value
    print(f"Defined {name} = {hex(value)}")

def handle_place(args):
    if len(args) != 2:
        print(f"Error: Invalid .PLACE syntax")
        return
    address = args[0]
    if address.startswith('0x'):
        address = int(address, 16)
    else:
        address = int(address)
    text = args[1].strip('"')
    for i in range(0, len(text), 2):
        high_char = text[i]
        low_char = text[i+1] if i+1 < len(text) else '\x00'
        high_byte = ord(high_char)
        low_byte = ord(low_char)
        word = (high_byte << 8) | low_byte
        memory[address + (i // 2)] = word
    print(f"Placed '{text}' at {hex(address)}")

def expand_macro(macro_name, args):
    if macro_name in macros:
        macro_params, macro_body = macros[macro_name]
        if len(args) != len(macro_params):
            print(f"Error: Macro {macro_name} expects {len(macro_params)} arguments, got {len(args)}")
            return []
        param_map = dict(zip(macro_params, args))
        expanded_lines = []
        for line in macro_body:
            for param, value in param_map.items():
                line = line.replace(param, value)
            expanded_lines.append(line)
        return expanded_lines
    else:
        print(f"Error: Undefined macro {macro_name}")
        return []

def parse_macro(line, lines_iter):
    match = re.match(r'@macro\s+(\w+)\s*\[(.*?)\]\s*\((.*)', line)
    if match:
        name = match.group(1)
        params = [p.strip() for p in match.group(2).split(',') if p.strip()]
        body_line = match.group(3)
        body = [body_line]
        while True:
            try:
                next_line = next(lines_iter).strip()
                if next_line == ')':
                    break
                body.append(next_line)
            except StopIteration:
                print("Error: Macro not properly terminated with ')'")
                break
        macros[name] = (params, body)
        print(f"Defined macro '{name}' with params {params}")
        return True
    else:
        print(f"Error: Invalid macro definition")
        return False

def second_pass(program):
    # We will substitute the definitions and macros with proper instructions
    # if line starts with @SOMETHING then it is a macro
    # if line starts with .SOMETHING then it is a definition

    assembled_program = []
    for item in program:
        if item[0].startswith('@'):
            #Check if its a known macro
            if item[0] in macros:
                macro = macros[item[0]]
                #Replace the macro with the instructions
                for line in macro:
                    assembled_program.append(parse_line(line))
            else:
                print(f"Error: Unknown macro {item[0]}")
        elif item[0].startswith('.'):
            #Check if its a known definition
            if item[0] in definitions:
                definition = definitions[item[0]]
                #Replace the definition with the value
                assembled_program.append(parse_line(definition))
            else:
                print(f"Error: Unknown definition {item[0]}")

def third_pass(program):
    assembled_program = []
    for item in program:
        if item[0] == 'instruction':
            opcode = item[1]
            args = item[2]
            # Handle arguments, replace labels and definitions
            processed_args = []
            for arg in args:
                # Remove commas
                arg = arg.strip(',')
                if arg in definitions:
                    processed_args.append(definitions[arg])
                elif arg in registers:
                    processed_args.append(registers[arg])
                elif arg.startswith('#'):  # Immediate value
                    value = arg[1:]
                    if value.startswith('0x'):
                        value = int(value, 16)
                    else:
                        value = int(value)
                    processed_args.append(value)
                elif arg.startswith('%'):  # Special registers (e.g., %DISP)
                    processed_args.append(arg)
                elif arg in labels:
                    processed_args.append(labels[arg])
                else:
                    print(f"Error: Unknown argument '{arg}'")
                    processed_args.append(0)
            assembled_program.append((opcode, processed_args))
        elif item[0] == 'label':
            pass  # Labels are already handled
        elif item[0] == 'directive':
            pass  # Directives are handled during the first pass
        else:
            print(item[1])
    return assembled_program

def assemble(lines):
    global current_address
    output_program = []
    lines_iter = iter(lines)
    for line in lines_iter:
        line = line.split(';')[0].strip()  # Remove comments and whitespace
        if not line:
            continue

        if line.startswith('@macro'):
            parse_macro(line, lines_iter)
        elif line.startswith('@define'):
            tokens = line.split()
            args = tokens[1:]
            handle_define(args)
        elif line.startswith('.PLACE'):
            tokens = re.findall(r'0x[0-9A-Fa-f]+|"[^"]+"|\S+', line)
            handle_place(tokens[1:])
        else:
            parsed = parse_line(line)
            if parsed:
                if parsed[0] == 'directive':
                    directive = parsed[1]
                    args = parsed[2]
                    if directive == '@macro':
                        parse_macro(line, lines_iter)
                    elif directive == '@define':
                        handle_define(args)
                    elif directive == '.PLACE':
                        handle_place(args)
                elif parsed[0] == 'label':
                    label = parsed[1]
                    labels[label] = current_address
                elif parsed[0] == 'instruction':
                    instruction_name = line.split()[0]
                    if instruction_name in macros:
                        args = line.split()[1:]
                        expanded_lines = expand_macro(instruction_name, args)
                        for expanded_line in expanded_lines:
                            output_program.append(parse_line(expanded_line))
                            current_address += 1
                    else:
                        output_program.append(parsed)
                        current_address += 1
                else:
                    print(parsed[1])  # Error message
    # Second pass to resolve labels and definitions
    second = second_pass(output_program)
    final_program = third_pass(second)


    return final_program

if __name__ == '__main__':
    # Take source file as input
    if len(sys.argv) < 2:
        print("Usage: python assembler.py <source.asm>")
        sys.exit(1)

    with open(sys.argv[1], 'r') as f:
        lines = f.readlines()

    assembled_program = assemble(lines)
    print("\nAssembled Program:")
    for line in assembled_program:
        print(line)

    print("\nMemory Contents:")
    for addr in sorted(memory.keys()):
        print(f"0x{addr:04X}: 0x{memory[addr]:04X}")
