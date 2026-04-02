; test.asm — full instruction set test bench with console output
;
; Each test group prints its result to the 16x16 console:
;   Row 0: "LOAD MOV : PASS" or "LOAD MOV : FAIL"
;   Row 1: "ADD SUB  : PASS" or "ADD SUB  : FAIL"
;   Row 2: "BITWISE  : PASS" or "BITWISE  : FAIL"
;   Row 3: "JUMP FLW : PASS" or "JUMP FLW : FAIL"
;   Row 4: "STACK    : PASS" or "STACK    : FAIL"
;
; After running, mem[0x0300]:
;   0xBEEF = all tests passed
;   0xDEAD = at least one test failed
;
; Register conventions:
;   R8  — failure counter (0 means all pass)
;   R9  — write subroutine char scratch
;   R12 — write subroutine destination (console address)
;   R14 — result address (0x0300)
;   R15 — EXPECT scratch

; ── Font data ─────────────────────────────────────────────────────────────────
; 8x8 glyphs for ASCII 32–127; each glyph is 4 words (two 8-bit rows per word).
.PLACE 0x3100 [
0x0000, 0x0000, 0x0000, 0x0000, 0x3078, 0x7830, 0x3000, 0x3000,
0x6c6c, 0x6c00, 0x0000, 0x0000, 0x6c6c, 0xfe6c, 0xfe6c, 0x6c00,
0x307c, 0xc078, 0x0cf8, 0x3000, 0x00c6, 0xcc18, 0x3066, 0xc600,
0x386c, 0x3876, 0xdccc, 0x7600, 0x6060, 0xc000, 0x0000, 0x0000,
0x1830, 0x6060, 0x6030, 0x1800, 0x6030, 0x1818, 0x1830, 0x6000,
0x0066, 0x3cff, 0x3c66, 0x0000, 0x0030, 0x30fc, 0x3030, 0x0000,
0x0000, 0x0000, 0x0030, 0x3060, 0x0000, 0x00fc, 0x0000, 0x0000,
0x0000, 0x0000, 0x0030, 0x3000, 0x060c, 0x1830, 0x60c0, 0x8000,
0x7cc6, 0xcede, 0xf6e6, 0x7c00, 0x3070, 0x3030, 0x3030, 0xfc00,
0x78cc, 0x0c38, 0x60cc, 0xfc00, 0x78cc, 0x0c38, 0x0ccc, 0x7800,
0x1c3c, 0x6ccc, 0xfe0c, 0x1e00, 0xfcc0, 0xf80c, 0x0ccc, 0x7800,
0x3860, 0xc0f8, 0xcccc, 0x7800, 0xfccc, 0x0c18, 0x3030, 0x3000,
0x78cc, 0xcc78, 0xcccc, 0x7800, 0x78cc, 0xcc7c, 0x0c18, 0x7000,
0x0030, 0x3000, 0x0030, 0x3000, 0x0030, 0x3000, 0x3030, 0x6000,
0x1830, 0x60c0, 0x6030, 0x1800, 0x0000, 0xfc00, 0x00fc, 0x0000,
0x6030, 0x180c, 0x1830, 0x6000, 0x78cc, 0x0c18, 0x3000, 0x3000,
0x7cc6, 0xdede, 0xdec0, 0x7800, 0x3078, 0xcccc, 0xfccc, 0xcc00,
0xfc66, 0x667c, 0x6666, 0xfc00, 0x3c66, 0xc0c0, 0xc066, 0x3c00,
0xf86c, 0x6666, 0x666c, 0xf800, 0xfe62, 0x6878, 0x6862, 0xfe00,
0xfe62, 0x6878, 0x6860, 0xf000, 0x3c66, 0xc0c0, 0xce66, 0x3e00,
0xcccc, 0xccfc, 0xcccc, 0xcc00, 0x7830, 0x3030, 0x3030, 0x7800,
0x1e0c, 0x0c0c, 0xcccc, 0x7800, 0xe666, 0x6c78, 0x6c66, 0xe600,
0xf060, 0x6060, 0x6266, 0xfe00, 0xc6ee, 0xfefe, 0xd6c6, 0xc600,
0xc6e6, 0xf6de, 0xcec6, 0xc600, 0x386c, 0xc6c6, 0xc66c, 0x3800,
0xfc66, 0x667c, 0x6060, 0xf000, 0x78cc, 0xcccc, 0xdc78, 0x1c00,
0xfc66, 0x667c, 0x6c66, 0xe600, 0x78cc, 0xe070, 0x1ccc, 0x7800,
0xfcb4, 0x3030, 0x3030, 0x7800, 0xcccc, 0xcccc, 0xcccc, 0xfc00,
0xcccc, 0xcccc, 0xcc78, 0x3000, 0xc6c6, 0xc6d6, 0xfeee, 0xc600,
0xc6c6, 0x6c38, 0x386c, 0xc600, 0xcccc, 0xcc78, 0x3030, 0x7800,
0xfec6, 0x8c18, 0x3266, 0xfe00, 0x7860, 0x6060, 0x6060, 0x7800,
0xc060, 0x3018, 0x0c06, 0x0200, 0x7818, 0x1818, 0x1818, 0x7800,
0x1038, 0x6cc6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00ff,
0x3030, 0x1800, 0x0000, 0x0000, 0x0000, 0x780c, 0x7ccc, 0x7600,
0xe060, 0x607c, 0x6666, 0xdc00, 0x0000, 0x78cc, 0xc0cc, 0x7800,
0x1c0c, 0x0c7c, 0xcccc, 0x7600, 0x0000, 0x78cc, 0xfcc0, 0x7800,
0x386c, 0x60f0, 0x6060, 0xf000, 0x0000, 0x76cc, 0xcc7c, 0x0cf8,
0xe060, 0x6c76, 0x6666, 0xe600, 0x3000, 0x7030, 0x3030, 0x7800,
0x0c00, 0x0c0c, 0x0ccc, 0xcc78, 0xe060, 0x666c, 0x786c, 0xe600,
0x7030, 0x3030, 0x3030, 0x7800, 0x0000, 0xccfe, 0xfed6, 0xc600,
0x0000, 0xf8cc, 0xcccc, 0xcc00, 0x0000, 0x78cc, 0xcccc, 0x7800,
0x0000, 0xdc66, 0x667c, 0x60f0, 0x0000, 0x76cc, 0xcc7c, 0x0c1e,
0x0000, 0xdc76, 0x6660, 0xf000, 0x0000, 0x7cc0, 0x780c, 0xf800,
0x1030, 0x7c30, 0x3034, 0x1800, 0x0000, 0xcccc, 0xcccc, 0x7600,
0x0000, 0xcccc, 0xcc78, 0x3000, 0x0000, 0xc6d6, 0xfefe, 0x6c00,
0x0000, 0xc66c, 0x386c, 0xc600, 0x0000, 0xcccc, 0xcc7c, 0x0cf8,
0x0000, 0xfc98, 0x3064, 0xfc00, 0x1c30, 0x30e0, 0x3030, 0x1c00,
0x1818, 0x1800, 0x1818, 0x1800, 0xe030, 0x301c, 0x3030, 0xe000,
0x76dc, 0x0000, 0x0000, 0x0000
]

; ── Console rows (8-char label + 8 spaces; suffix written at runtime) ─────────
; Each row is 16 words (0x10 addresses apart).
.PLACE 0x2900 "LOAD MOV        "
.PLACE 0x2910 "ADD SUB         "
.PLACE 0x2920 "BITWISE         "
.PLACE 0x2930 "JUMP FLW        "
.PLACE 0x2940 "STACK           "

; ── Macros ───────────────────────────────────────────────────────────────────

; EXPECT_J val, lbl — asserts R0 == val; jumps to lbl if not
@macro EXPECT_J [val, lbl]
    LOAD R15, val
    CMP R0, R15
    JNE lbl
@endmacro

; EXPECT_REG_J reg, val, lbl — asserts reg == val; jumps to lbl if not
@macro EXPECT_REG_J [reg, val, lbl]
    LOAD R15, val
    CMP reg, R15
    JNE lbl
@endmacro

; PASS_ROW addr — load console suffix address and call write_pass
@macro PASS_ROW [addr]
    LOAD R12, addr
    CALL .write_pass
@endmacro

; FAIL_ROW addr — load console suffix address and call write_fail
@macro FAIL_ROW [addr]
    LOAD R12, addr
    CALL .write_fail
@endmacro

; ── Init + Test 1: LOAD / MOV / INC / DEC ────────────────────────────────────
.main:
    LOAD SP, #0x4000
    LOAD DISP, #0x3000
    LOAD R8, #0x0000          ; failure counter
    LOAD R14, #0x0300

    LOAD R0, #0x0042
    @EXPECT_J #0x0042, .t1_fail

    LOAD R1, #0x00FF
    MOV R0, R1
    @EXPECT_J #0x00FF, .t1_fail

    INC R0
    @EXPECT_J #0x0100, .t1_fail

    DEC R0
    @EXPECT_J #0x00FF, .t1_fail

    NOP
    @PASS_ROW #0x2908
    JMP .t2_arith

.t1_fail:
    INC R8
    @FAIL_ROW #0x2908

; ── Test 2: ADD / SUB / Z flag ───────────────────────────────────────────────
.t2_arith:
    LOAD R0, #0x0005
    LOAD R1, #0x0003

    ADD R2, R0, R1
    @EXPECT_REG_J R2, #0x0008, .t2_fail

    SUB R3, R0, R1
    @EXPECT_REG_J R3, #0x0002, .t2_fail

    SUB R4, R0, R0            ; result is 0, Z must be set
    JZ .t2_pass
    JMP .t2_fail

.t2_pass:
    @PASS_ROW #0x2918
    JMP .t3_bitwise

.t2_fail:
    INC R8
    @FAIL_ROW #0x2918
    JMP .t3_bitwise

; ── Test 3: AND / OR / XOR / NOT / SHL / SHR ─────────────────────────────────
.t3_bitwise:
    LOAD R0, #0xFF00
    LOAD R1, #0x0FF0

    AND R2, R0, R1
    @EXPECT_REG_J R2, #0x0F00, .t3_fail

    OR R2, R0, R1
    @EXPECT_REG_J R2, #0xFFF0, .t3_fail

    XOR R2, R0, R1
    @EXPECT_REG_J R2, #0xF0F0, .t3_fail

    NOT R2, R0
    @EXPECT_REG_J R2, #0x00FF, .t3_fail

    LOAD R0, #0x0001
    LOAD R1, #0x0004
    SHL R2, R0, R1
    @EXPECT_REG_J R2, #0x0010, .t3_fail

    LOAD R0, #0x0100
    SHR R2, R0, R1
    @EXPECT_REG_J R2, #0x0010, .t3_fail

    @PASS_ROW #0x2928
    JMP .t4_flow

.t3_fail:
    INC R8
    @FAIL_ROW #0x2928
    JMP .t4_flow

; ── Test 4: JMP / JE / JNE / JG / JL ────────────────────────────────────────
; Uses chained labels so each branch instruction is itself the jump target
; of the previous one — a clean way to test all conditionals in sequence.
.t4_flow:
    LOAD R0, #0x1234
    LOAD R1, #0x1234
    CMP R0, R1
    JE .t4_jne_test
    JMP .t4_fail

.t4_jne_test:
    LOAD R0, #0x0001
    LOAD R1, #0x0002
    CMP R0, R1
    JNE .t4_jg_test
    JMP .t4_fail

.t4_jg_test:
    LOAD R0, #0x0005
    LOAD R1, #0x0003
    CMP R0, R1
    JG .t4_jl_test
    JMP .t4_fail

.t4_jl_test:
    CMP R1, R0
    JL .t4_pass
    JMP .t4_fail

.t4_pass:
    @PASS_ROW #0x2938
    JMP .t5_stack

.t4_fail:
    INC R8
    @FAIL_ROW #0x2938
    JMP .t5_stack

; ── Test 5: PUSH / POP / CALL / RET / STORE / LOAD indirect ──────────────────
.t5_stack:
    LOAD R0, #0x1234
    PUSH R0
    LOAD R0, #0x0000
    POP R0
    @EXPECT_J #0x1234, .t5_fail

    CALL .double              ; doubles R0: 0x1234 → 0x2468
    @EXPECT_J #0x2468, .t5_fail

    LOAD R0, #0xBEEF
    LOAD R1, #0x0400
    STORE R1, R0              ; mem[0x0400] = 0xBEEF
    LOAD R2, R1               ; R2 = mem[R1] = 0xBEEF
    @EXPECT_REG_J R2, #0xBEEF, .t5_fail

    @PASS_ROW #0x2948
    JMP .done

.t5_fail:
    INC R8
    @FAIL_ROW #0x2948

; ── Final result ──────────────────────────────────────────────────────────────
.done:
    LOAD R0, #0x0000
    CMP R8, R0
    JE .all_pass
    LOAD R0, #0xDEAD
    STORE R14, R0
    HLT

.all_pass:
    LOAD R0, #0xBEEF
    STORE R14, R0
    HLT

; ── Write subroutines ─────────────────────────────────────────────────────────
; Input: R12 = start address in the console buffer (0x2900-row + 8).
; Writes " : PASS" or " : FAIL" (7 chars) starting at R12.

.write_pass:
    LOAD R9, #0x0020          ; ' '
    STORE R12, R9
    INC R12
    LOAD R9, #0x003A          ; ':'
    STORE R12, R9
    INC R12
    LOAD R9, #0x0020          ; ' '
    STORE R12, R9
    INC R12
    LOAD R9, #0x0050          ; 'P'
    STORE R12, R9
    INC R12
    LOAD R9, #0x0041          ; 'A'
    STORE R12, R9
    INC R12
    LOAD R9, #0x0053          ; 'S'
    STORE R12, R9
    INC R12
    STORE R12, R9             ; 'S' (second)
    RET

.write_fail:
    LOAD R9, #0x0020          ; ' '
    STORE R12, R9
    INC R12
    LOAD R9, #0x003A          ; ':'
    STORE R12, R9
    INC R12
    LOAD R9, #0x0020          ; ' '
    STORE R12, R9
    INC R12
    LOAD R9, #0x0046          ; 'F'
    STORE R12, R9
    INC R12
    LOAD R9, #0x0041          ; 'A'
    STORE R12, R9
    INC R12
    LOAD R9, #0x0049          ; 'I'
    STORE R12, R9
    INC R12
    LOAD R9, #0x004C          ; 'L'
    STORE R12, R9
    RET

; ── Subroutines ───────────────────────────────────────────────────────────────
.double:
    ADD R0, R0, R0
    RET
