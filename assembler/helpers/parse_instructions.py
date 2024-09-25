instructions = {
    'LOAD': 0x00,
    'STORE': 0x02,
    'MOV': 0x03,
    'INC': 0x04,
    'DEC': 0x05,
    'ADD': 0x06,
    'SUB': 0x07,
    'AND': 0x08,
    'OR': 0x09,
    'XOR': 0x0A,
    'NOT': 0x0B,
    'SHL': 0x0C,
    'SHR': 0x0D,
    'CMP': 0x0E,
    'JMP': 0x0F,
    'JZ': 0x10,
    'JE': 0x11,
    'JNE': 0x12,
    'JG': 0x13,
    'JL': 0x14,
    'CALL': 0x15,
    'RET': 0x16,
    'PUSH': 0x17,
    'POP': 0x18,
    'HLT': 0x19,
    'NOP': 0x1A,
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

allocated_routines = {}

def parse_load(opcode : str, args) -> int:
            if len(args) != 2:
                print(f"Error: Invalid number of arguments for LOAD instruction")
                return 0
            else:
                dest, src = args
                dest = dest.strip(',').strip()
                src = src.strip(',').strip()
                # Case 1: LOAD R1, R2 (Register-to-Register)
                if dest in registers and src in registers:
                    # Opcode (5 bits), dest register (5 bits), src register (5 bits), rest are 0 until 32 bits
                    op = (instructions[opcode] << 27) & 0xFFFFFFFF  # Shift the opcode to the first 5 bits, ensuring 32-bit
                    dest_reg = (registers[dest] << 22) & 0xFFFFFFFF  # Destination register next 5 bits
                    src_reg = (registers[src] << 17) & 0xFFFFFFFF    # Source register next 5 bits
                    return op | dest_reg | src_reg  # Combine opcode, dest, and source into a 32-bit result
                # Case 2: LOAD R1, #IMM (Register-to-Immediate)
                elif dest in registers and src.startswith('#'):
                    try:
                        # Check if its in hexa or decimal
                        if src.startswith('#0x'):
                            imm_value = int(src[1:], 16)
                        elif src.startswith('#0b'):
                            imm_value = int(src[1:], 2)
                        else:
                            imm_value = int(src[1:])

                        if imm_value < 0 or imm_value > 0xFFFF:  # Ensure it's within 16-bit range
                            print(f"Error: Immediate value {imm_value} out of range (must be 0 to 65535)")
                            return 0
                        # Opcode (5 bits), dest register (5 bits), immediate value (16 bits), rest is 0
                        op = (instructions[opcode] << 27) & 0xFFFFFFFF  # Opcode 5 bits
                        dest_reg = (registers[dest] << 22) & 0xFFFFFFFF  # Destination register next 5 bits
                        imm_value = imm_value & 0xFFFF  # Mask the immediate value to 16 bits
                        return op | dest_reg | imm_value  # Combine opcode, dest, and immediate into a 32-bit result
                    except ValueError:
                        print(f"Error: Invalid immediate value {src}")
                        return 0
                else:
                    print(f"Error: Invalid arguments for LOAD instruction")
                    return 0

def parse_store(opcode : str, args) -> int:
    if len(args) != 2:
        print(f"Error: Invalid number of arguments for STORE instruction")
        return 0
    else:
        src, dest = args
        src = src.strip(',').strip()
        dest = dest.strip(',').strip()
        # Store reg1, reg2, stores content of reg2 in the address in reg1
        if src in registers and dest in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            src_reg = (registers[src] << 22) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 17) & 0xFFFFFFFF
            return op | src_reg | dest_reg
        else:
            print(f"Error: Invalid arguments for STORE instruction")
            return 0


def parse_mov(opcode : str, args) -> int:
    if len(args) != 2:
        print(f"Error: Invalid number of arguments for MOV instruction")
        return 0
    else:
        dest, src = args
        dest = dest.strip(',').strip()
        src = src.strip(',').strip()
        if dest in registers and src in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            src_reg = (registers[src] << 17) & 0xFFFFFFFF
            return op | dest_reg | src_reg
        else:
            print(f"Error: Invalid arguments for MOV instruction")
            return 0

def parse_inc(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for INC instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            return op | dest_reg
        else:
            print(f"Error: Invalid arguments for INC instruction")
            return 0

def parse_dec(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for DEC instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            return op | dest_reg
        else:
            print(f"Error: Invalid arguments for DEC instruction")
            return 0

def parse_add(opcode : str, args) -> int:
    if len(args) != 3:
        print(f"Error: Invalid number of arguments for ADD instruction")
        return 0
    else:
        dest, op1, op2 = args
        dest = dest.strip(',').strip()
        op1 = op1.strip(',').strip()
        op2 = op2.strip(',').strip()
        
        if dest in registers and op1 in registers and op2 in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            op1_reg = (registers[op1] << 17) & 0xFFFFFFFF
            op2_reg = (registers[op2] << 12) & 0xFFFFFFFF
            return op | dest_reg | op1_reg | op2_reg
        else:
            print(f"Error: Invalid arguments for ADD instruction")
            return 0

def parse_sub(opcode : str, args) -> int:
    if len(args) != 3:
        print(f"Error: Invalid number of arguments for SUB instruction")
        return 0 
    else:
        dest, op1, op2 = args
        dest = dest.strip(',').strip()
        op1 = op1.strip(',').strip()
        op2 = op2.strip(',').strip()
        
        if dest in registers and op1 in registers and op2 in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            op1_reg = (registers[op1] << 17) & 0xFFFFFFFF
            op2_reg = (registers[op2] << 12) & 0xFFFFFFFF
            return op | dest_reg | op1_reg | op2_reg
        else:
            print(f"Error: Invalid arguments for SUB instruction")
            return 0


def parse_and(opcode : str, args) -> int:
    if len(args) != 3:
        print(f"Error: Invalid number of arguments for AND instruction")
        return 0
    else:
        dest, op1, op2 = args
        dest = dest.strip(',').strip()
        op1 = op1.strip(',').strip()
        op2 = op2.strip(',').strip()
        
        if dest in registers and op1 in registers and op2 in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            op1_reg = (registers[op1] << 17) & 0xFFFFFFFF
            op2_reg = (registers[op2] << 12) & 0xFFFFFFFF
            return op | dest_reg | op1_reg | op2_reg
        else:
            print(f"Error: Invalid arguments for AND instruction")
            return 0

def parse_or(opcode : str, args) -> int:
    if len(args) != 3:
        print(f"Error: Invalid number of arguments for OR instruction")
        return 0
    else:
        dest, op1, op2 = args
        dest = dest.strip(',').strip()
        op1 = op1.strip(',').strip()
        op2 = op2.strip(',').strip()
        
        if dest in registers and op1 in registers and op2 in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            op1_reg = (registers[op1] << 17) & 0xFFFFFFFF
            op2_reg = (registers[op2] << 12) & 0xFFFFFFFF
            return op | dest_reg | op1_reg | op2_reg
        else:
            print(f"Error: Invalid arguments for OR instruction")
            return 0

def parse_xor(opcode : str, args) -> int:
    if len(args) != 3:
        print(f"Error: Invalid number of arguments for XOR instruction")
        return 0
    else:
        dest, op1, op2 = args
        dest = dest.strip(',').strip()
        op1 = op1.strip(',').strip()
        op2 = op2.strip(',').strip()
        
        if dest in registers and op1 in registers and op2 in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            op1_reg = (registers[op1] << 17) & 0xFFFFFFFF
            op2_reg = (registers[op2] << 12) & 0xFFFFFFFF
            return op | dest_reg | op1_reg | op2_reg
        else:
            print(f"Error: Invalid arguments for XOR instruction")
            return 0

def parse_not(opcode : str, args) -> int:
    if len(args) != 2:
        print(f"Error: Invalid number of arguments for NOT instruction")
        return 0
    else:
        dest, src = args
        dest = dest.strip(',').strip()
        src = src.strip(',').strip()
        if dest in registers and src in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            src_reg = (registers[src] << 17) & 0xFFFFFFFF
            return op | dest_reg | src_reg
        else:
            print(f"Error: Invalid arguments for NOT instruction")
            return 0

def parse_shl(opcode : str, args) -> int:
    if len(args) != 3:
        print(f"Error: Invalid number of arguments for SHL instruction")
        return 0
    else:
        dest, src, shift = args
        dest = dest.strip(',').strip()
        src = src.strip(',').strip()
        shift = shift.strip(',').strip()
        if dest in registers and src in registers and shift in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            src_reg = (registers[src] << 17) & 0xFFFFFFFF
            shift_reg = (registers[shift] << 12) & 0xFFFFFFFF
            return op | dest_reg | src_reg | shift_reg
        else:
            print(f"Error: Invalid arguments for SHL instruction")
            return 0

def parse_shr(opcode : str, args) -> int:
    if len(args) != 3:
        print(f"Error: Invalid number of arguments for SHR instruction")
        return 0
    else:
        dest, src, shift = args
        dest = dest.strip(',').strip()
        src = src.strip(',').strip()
        shift = shift.strip(',').strip()
        if dest in registers and src in registers and shift in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            src_reg = (registers[src] << 17) & 0xFFFFFFFF
            shift_reg = (registers[shift] << 12) & 0xFFFFFFFF
            return op | dest_reg | src_reg | shift_reg
        else:
            print(f"Error: Invalid arguments for SHR instruction")
            return 0

def parse_cmp(opcode : str, args) -> int:
    if len(args) != 2:
        print(f"Error: Invalid number of arguments for CMP instruction")
        return 0
    else:
        op1, op2 = args
        op1 = op1.strip(',').strip()
        op2 = op2.strip(',').strip()
        if op1 in registers and op2 in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            op1_reg = (registers[op1] << 22) & 0xFFFFFFFF
            op2_reg = (registers[op2] << 17) & 0xFFFFFFFF
            return op | op1_reg | op2_reg
        else:
            print(f"Error: Invalid arguments for CMP instruction")
            return 0

def parse_jmp(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for JMP instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in allocated_routines:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_addr = (allocated_routines[dest])
            return op | dest_addr
        else:
            print(f"Error: Invalid arguments for JMP instruction")
            return 0


def parse_jz(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for JZ instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in allocated_routines:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_addr = (allocated_routines[dest])
            return op | dest_addr
        else:
            print(f"Error: Invalid arguments for JZ instruction")
            return 0

def parse_je(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for JE instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in allocated_routines: 
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_addr = (allocated_routines[dest])
            return op | dest_addr
        else:
            print(f"Error: Invalid arguments for JE instruction")
            return 0

def parse_jne(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for JNE instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in allocated_routines:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_addr = (allocated_routines[dest])
            return op | dest_addr
        else:
            print(f"Error: Invalid arguments for JNE instruction")
            return 0

def parse_jg(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for JG instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in allocated_routines:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_addr = (allocated_routines[dest])
            return op | dest_addr
        else:
            print(f"Error: Invalid arguments for JG instruction")
            return 0

def parse_jl(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for JL instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in allocated_routines:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_addr = (allocated_routines[dest])
            return op | dest_addr
        else:
            print(f"Error: Invalid arguments for JL instruction")
            return 0

def parse_call(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for CALL instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in allocated_routines:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            # Dest address is the last 16 bits of the 32-bit instruction
            dest_addr = (allocated_routines[dest]) & 0xFFFFFFFF
            return op | dest_addr
        else:
            print(f"Error: Invalid arguments for CALL instruction")
            return 0

def parse_ret(opcode : str, args) -> int:
    if len(args) != 0:
        print(f"Error: Invalid number of arguments for RET instruction")
        return 0
    else:
        op = (instructions[opcode] << 27) & 0xFFFFFFFF
        return op

def parse_push(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for PUSH instruction")
        return 0
    else:
        src = args[0].strip(',').strip()
        if src in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            src_reg = (registers[src] << 22) & 0xFFFFFFFF
            return op | src_reg
        else:
            print(f"Error: Invalid arguments for PUSH instruction")
            return 0

def parse_pop(opcode : str, args) -> int:
    if len(args) != 1:
        print(f"Error: Invalid number of arguments for POP instruction")
        return 0
    else:
        dest = args[0].strip(',').strip()
        if dest in registers:
            op = (instructions[opcode] << 27) & 0xFFFFFFFF
            dest_reg = (registers[dest] << 22) & 0xFFFFFFFF
            return op | dest_reg
        else:
            print(f"Error: Invalid arguments for POP instruction")
            return 0

def parse_hlt(opcode : str, args) -> int:
    if len(args) != 0:
        print(f"Error: Invalid number of arguments for HLT instruction")
        return 0
    else:
        op = (instructions[opcode] << 27) & 0xFFFFFFFF
        return op

def parse_nop(opcode : str, args) -> int:
    if len(args) != 0:
        print(f"Error: Invalid number of arguments for NOP instruction")
        return 0
    else:
        op = (instructions[opcode] << 27) & 0xFFFFFFFF
        return op
