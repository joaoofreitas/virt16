# Virt16 Assembler

Multi-pass Python assembler for the Virt16 architecture. No dependencies beyond the standard library.

## Building a program

```sh
cd assembler
python3 assemble.py <source.asm>
```

Output is written to `assembler/build/`:

| File | Contents |
|------|----------|
| `<name>.bin` | Flat binary — load this into the VM |
| `<name>.debug` | One instruction per line, used by the VM's debug panel |
| `<name>.data` | Human-readable dump of routines, addresses, macros, and the encoded program |

## Example programs

```sh
python3 assemble.py hello.asm   # console "Hello World!" demo
python3 assemble.py test.asm    # instruction set test suite
```

## Assembly language reference for the Virt16

### Syntax

```asm
; this is a comment
.routine_name:
    INSTRUCTION ARG1, ARG2
```

All labels must start with `.` and end with `:`. `.main` is always placed at address `0x0000`.

### Instructions

See the root `README.md` for the full opcode table. Quick reference:

```asm
LOAD R1, #0x0042    ; R1 = 0x42 (immediate)
LOAD R1, R2         ; R1 = memory[R2] (indirect via register)
STORE R1, R2        ; memory[R1] = R2
MOV R1, R2          ; R1 = R2
INC R1              ; R1++
DEC R1              ; R1--
ADD R1, R2, R3      ; R1 = R2 + R3
SUB R1, R2, R3      ; R1 = R2 - R3
CMP R1, R2          ; set E/G/L flags
JMP .label          ; unconditional jump
JG  .label          ; jump if greater
JL  .label          ; jump if less
JE  .label          ; jump if equal
JNE .label          ; jump if not equal
JZ  .label          ; jump if zero
CALL .label         ; call subroutine
RET                 ; return
PUSH R1             ; push R1 onto stack
POP  R1             ; pop into R1
HLT                 ; halt
NOP                 ; no-op
```

### Preprocessor

```asm
@define NAME value
    LOAD R1, %NAME  ; %NAME is replaced with value at assembly time

@macro MYMACRO [arg1, arg2]
    LOAD R0, arg1
    ADD R0, R0, arg2
@endmacro
    @MYMACRO #0x01, R3  ; expands inline

.PLACE 0x2900 "Hello"           ; write ASCII string to address
.PLACE 0x3100 [0x0000, 0x7e00]  ; write array of 16-bit words to address
```
