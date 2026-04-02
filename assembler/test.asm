; test.asm — full instruction set test bench with console output
;
; Runs all 16 test groups and prints results on the 16x16 console.
; Switch to Console mode in the Monitor tab after loading.
;
; Console layout (each row: 8-char label + " : PASS" or " : FAIL"):
;   Row  0  LOAD MOV  — LOAD imm, MOV, INC, DEC, NOP
;   Row  1  ADD SUB   — ADD, SUB, Z from arithmetic
;   Row  2  BITWISE   — AND, OR, XOR, NOT, SHL, SHR
;   Row  3  JUMP FLW  — JMP, JE, JNE, JG, JL
;   Row  4  STACK     — PUSH/POP, CALL/RET, STORE, LOAD indirect
;   Row  5  INC DECZ  — INC/DEC wrapping to 0 sets Z; no-wrap clears Z
;   Row  6  ADD OVFL  — 0xFFFF+1=0x0000, Z set, result correct
;   Row  7  SUB BORW  — 0x0000-0x0001=0xFFFF, Z clear, result correct
;   Row  8  CARRY JC  — JC branches on C=1, stays on C=0 (ADD + SUB)
;   Row  9  CMP ISOL  — CMP resets all flags; JG/JL "should not branch" cases
;   Row 10  FLG PRST  — E and G flags survive an intervening LOAD
;   Row 11  SHL EDGE  — SHL by 0, SHL overflow truncation, SHR by 8
;   Row 12  NOT EDGE  — NOT 0x0000, NOT 0xFFFF, NOT 0xAAAA
;   Row 13  LIFO STK  — PUSH×3 then POP×3 must yield values in reverse order
;   Row 14  NESTCALL  — two-level CALL/RET with independent return addresses
;   Row 15  SEQ MEM   — write then read back 4 adjacent memory cells
;
; mem[0x0300] on exit: 0xBEEF = all pass, 0xDEAD = at least one failure.
;
; Register conventions:
;   R8  — failure counter (incremented per failed test)
;   R9  — write subroutine char scratch
;   R12 — write subroutine destination (console cursor)
;   R14 — result address (0x0300)
;   R15 — EXPECT scratch

; ── Font data (ASCII 32–127, 4 words per glyph) ──────────────────────────────
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

; ── Console rows: 8-char label padded to 16 chars with spaces ─────────────────
; The runtime writes " : PASS" or " : FAIL" starting at label+8 (offset 0x08).
.PLACE 0x2900 "LOAD MOV        "
.PLACE 0x2910 "ADD SUB         "
.PLACE 0x2920 "BITWISE         "
.PLACE 0x2930 "JUMP FLW        "
.PLACE 0x2940 "STACK           "
.PLACE 0x2950 "INC DECZ        "
.PLACE 0x2960 "ADD OVFL        "
.PLACE 0x2970 "SUB BORW        "
.PLACE 0x2980 "CARRY JC        "
.PLACE 0x2990 "CMP ISOL        "
.PLACE 0x29A0 "FLG PRST        "
.PLACE 0x29B0 "SHL EDGE        "
.PLACE 0x29C0 "NOT EDGE        "
.PLACE 0x29D0 "LIFO STK        "
.PLACE 0x29E0 "NESTCALL        "
.PLACE 0x29F0 "SEQ MEM         "

; ── Macros ────────────────────────────────────────────────────────────────────

; EXPECT_J val, lbl — assert R0 == val; jump to lbl if not
@macro EXPECT_J [val, lbl]
    LOAD R15, val
    CMP R0, R15
    JNE lbl
@endmacro

; EXPECT_REG_J reg, val, lbl — assert reg == val; jump to lbl if not
@macro EXPECT_REG_J [reg, val, lbl]
    LOAD R15, val
    CMP reg, R15
    JNE lbl
@endmacro

; PASS_ROW addr — write " : PASS" suffix at console address addr
@macro PASS_ROW [addr]
    LOAD R12, addr
    CALL .write_pass
@endmacro

; FAIL_ROW addr — write " : FAIL" suffix at console address addr
@macro FAIL_ROW [addr]
    LOAD R12, addr
    CALL .write_fail
@endmacro

; ── Test 1 (row 0): LOAD / MOV / INC / DEC / NOP ─────────────────────────────
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

    INC R0                    ; 0x00FF → 0x0100
    @EXPECT_J #0x0100, .t1_fail

    DEC R0                    ; 0x0100 → 0x00FF
    @EXPECT_J #0x00FF, .t1_fail

    NOP
    @PASS_ROW #0x2908
    JMP .t2

.t1_fail:
    INC R8
    @FAIL_ROW #0x2908

; ── Test 2 (row 1): ADD / SUB / Z flag ───────────────────────────────────────
.t2:
    LOAD R0, #0x0005
    LOAD R1, #0x0003

    ADD R2, R0, R1
    @EXPECT_REG_J R2, #0x0008, .t2_fail

    SUB R3, R0, R1
    @EXPECT_REG_J R3, #0x0002, .t2_fail

    SUB R4, R0, R0            ; result = 0 → Z must be set
    JZ .t2_pass
    JMP .t2_fail

.t2_pass:
    @PASS_ROW #0x2918
    JMP .t3

.t2_fail:
    INC R8
    @FAIL_ROW #0x2918
    JMP .t3

; ── Test 3 (row 2): AND / OR / XOR / NOT / SHL / SHR ─────────────────────────
.t3:
    LOAD R0, #0xFF00
    LOAD R1, #0x0FF0

    AND R2, R0, R1
    @EXPECT_REG_J R2, #0x0F00, .t3_fail

    OR R2, R0, R1
    @EXPECT_REG_J R2, #0xFFF0, .t3_fail

    XOR R2, R0, R1
    @EXPECT_REG_J R2, #0xF0F0, .t3_fail

    NOT R2, R0                ; ~0xFF00 = 0x00FF
    @EXPECT_REG_J R2, #0x00FF, .t3_fail

    LOAD R0, #0x0001
    LOAD R1, #0x0004
    SHL R2, R0, R1            ; 1 << 4 = 0x0010
    @EXPECT_REG_J R2, #0x0010, .t3_fail

    LOAD R0, #0x0100
    SHR R2, R0, R1            ; 0x0100 >> 4 = 0x0010
    @EXPECT_REG_J R2, #0x0010, .t3_fail

    @PASS_ROW #0x2928
    JMP .t4

.t3_fail:
    INC R8
    @FAIL_ROW #0x2928
    JMP .t4

; ── Test 4 (row 3): JE / JNE / JG / JL ──────────────────────────────────────
; Each branch instruction is itself the jump target of the previous one,
; so a wrong result at any step propagates straight to .t4_fail.
.t4:
    LOAD R0, #0x1234
    LOAD R1, #0x1234
    CMP R0, R1
    JE .t4_jne                ; equal → must branch
    JMP .t4_fail

.t4_jne:
    LOAD R0, #0x0001
    LOAD R1, #0x0002
    CMP R0, R1
    JNE .t4_jg                ; not equal → must branch
    JMP .t4_fail

.t4_jg:
    LOAD R0, #0x0005
    LOAD R1, #0x0003
    CMP R0, R1
    JG .t4_jl                 ; 5 > 3 → must branch
    JMP .t4_fail

.t4_jl:
    CMP R1, R0                ; 3 < 5 → L must be set
    JL .t4_pass
    JMP .t4_fail

.t4_pass:
    @PASS_ROW #0x2938
    JMP .t5

.t4_fail:
    INC R8
    @FAIL_ROW #0x2938
    JMP .t5

; ── Test 5 (row 4): PUSH / POP / CALL / RET / STORE / LOAD indirect ──────────
.t5:
    LOAD R0, #0x1234
    PUSH R0
    LOAD R0, #0x0000
    POP R0
    @EXPECT_J #0x1234, .t5_fail

    CALL .double              ; 0x1234 × 2 = 0x2468
    @EXPECT_J #0x2468, .t5_fail

    LOAD R0, #0xBEEF
    LOAD R1, #0x0400
    STORE R1, R0              ; mem[0x0400] = 0xBEEF
    LOAD R2, R1               ; R2 = mem[R1] = 0xBEEF
    @EXPECT_REG_J R2, #0xBEEF, .t5_fail

    @PASS_ROW #0x2948
    JMP .t6

.t5_fail:
    INC R8
    @FAIL_ROW #0x2948

; ── Test 6 (row 5): INC / DEC wrap to zero sets Z, non-zero clears Z ─────────
.t6:
    LOAD R0, #0xFFFF
    INC R0                    ; 0xFFFF + 1 = 0x0000, Z must be set
    @EXPECT_J #0x0000, .t6_fail
    JZ .t6_dec
    JMP .t6_fail

.t6_dec:
    LOAD R0, #0x0001
    DEC R0                    ; 0x0001 - 1 = 0x0000, Z must be set
    @EXPECT_J #0x0000, .t6_fail
    JZ .t6_nowrap
    JMP .t6_fail

.t6_nowrap:
    LOAD R0, #0x0001
    INC R0                    ; 0x0001 + 1 = 0x0002, Z must NOT be set
    JZ .t6_fail
    @PASS_ROW #0x2958
    JMP .t7

.t6_fail:
    INC R8
    @FAIL_ROW #0x2958
    JMP .t7

; ── Test 7 (row 6): ADD overflow wraps and sets Z ─────────────────────────────
.t7:
    LOAD R0, #0xFFFF
    LOAD R1, #0x0001
    ADD R2, R0, R1            ; 0xFFFF + 1 = 0x0000 (16-bit wrap)
    @EXPECT_REG_J R2, #0x0000, .t7_fail
    JZ .t7_pass               ; Z must be set (result is 0)
    JMP .t7_fail

.t7_pass:
    @PASS_ROW #0x2968
    JMP .t8

.t7_fail:
    INC R8
    @FAIL_ROW #0x2968
    JMP .t8

; ── Test 8 (row 7): SUB borrow wraps and leaves Z clear ──────────────────────
.t8:
    LOAD R0, #0x0000
    LOAD R1, #0x0001
    SUB R2, R0, R1            ; 0x0000 - 1 = 0xFFFF (borrow/wrap)
    @EXPECT_REG_J R2, #0xFFFF, .t8_fail
    JZ .t8_fail               ; Z must NOT be set (result is 0xFFFF ≠ 0)
    @PASS_ROW #0x2978
    JMP .t9

.t8_fail:
    INC R8
    @FAIL_ROW #0x2978
    JMP .t9

; ── Test 9 (row 8): JC branches on C=1, stays on C=0 ─────────────────────────
; Tests all four combinations: JC×{stay,branch} for both ADD and SUB carry.
.t9:
    LOAD R0, #0x0001
    LOAD R1, #0x0001
    ADD R2, R0, R1            ; no overflow → C=0
    JC .t9_fail               ; must NOT branch

    LOAD R0, #0xFFFF
    LOAD R1, #0x0001
    ADD R2, R0, R1            ; overflow → C=1
    JC .t9_sub                ; must branch
    JMP .t9_fail

.t9_sub:
    LOAD R0, #0x0005
    LOAD R1, #0x0003
    SUB R2, R0, R1            ; no borrow → C=0
    JC .t9_fail               ; must NOT branch

    LOAD R0, #0x0001
    LOAD R1, #0x0002
    SUB R2, R0, R1            ; borrow → C=1
    JC .t9_pass               ; must branch
    JMP .t9_fail

.t9_pass:
    @PASS_ROW #0x2988
    JMP .t10

.t9_fail:
    INC R8
    @FAIL_ROW #0x2988
    JMP .t10

; ── Test 10 (row 9): CMP flag isolation and reset ────────────────────────────
; Verifies that CMP resets all three flags before setting one,
; and that "should not branch" cases are respected.
.t10:
    LOAD R0, #0x0001
    LOAD R1, #0x0005
    CMP R0, R1                ; L=1, G=0 → JG must NOT branch
    JG .t10_fail
    JL .t10_jg_ok             ; L is set → JL must branch
    JMP .t10_fail

.t10_jg_ok:
    LOAD R0, #0x0005
    LOAD R1, #0x0001
    CMP R0, R1                ; G=1, L=0 → JL must NOT branch
    JL .t10_fail
    JG .t10_reset             ; G is set → JG must branch
    JMP .t10_fail

.t10_reset:
    LOAD R0, #0x0005
    LOAD R1, #0x0003
    CMP R0, R1                ; G=1
    LOAD R0, #0x0001
    LOAD R1, #0x0005
    CMP R0, R1                ; must reset G=0 and set L=1
    JG .t10_fail              ; G must be cleared now
    @PASS_ROW #0x2998
    JMP .t11

.t10_fail:
    INC R8
    @FAIL_ROW #0x2998
    JMP .t11

; ── Test 11 (row 10): flags survive an intervening non-CMP instruction ────────
.t11:
    LOAD R0, #0xABCD
    LOAD R1, #0xABCD
    CMP R0, R1                ; E=1
    LOAD R2, #0x1234          ; non-CMP: must not disturb flags
    JE .t11_e_ok
    JMP .t11_fail

.t11_e_ok:
    LOAD R0, #0x0099
    LOAD R1, #0x0001
    CMP R0, R1                ; G=1
    LOAD R2, #0x5678          ; non-CMP: must not disturb flags
    JG .t11_g_ok
    JMP .t11_fail

.t11_g_ok:
    @PASS_ROW #0x29A8
    JMP .t12

.t11_fail:
    INC R8
    @FAIL_ROW #0x29A8
    JMP .t12

; ── Test 12 (row 11): SHL/SHR edge cases ─────────────────────────────────────
.t12:
    LOAD R0, #0x1234
    LOAD R1, #0x0000
    SHL R2, R0, R1            ; shift by 0 → value unchanged
    @EXPECT_REG_J R2, #0x1234, .t12_fail

    LOAD R0, #0x8000
    LOAD R1, #0x0001
    SHL R2, R0, R1            ; 0x8000 << 1 overflows 16 bits → 0x0000
    @EXPECT_REG_J R2, #0x0000, .t12_fail

    LOAD R0, #0x0100
    LOAD R1, #0x0008
    SHR R2, R0, R1            ; 0x0100 >> 8 = 0x0001
    @EXPECT_REG_J R2, #0x0001, .t12_fail

    @PASS_ROW #0x29B8
    JMP .t13

.t12_fail:
    INC R8
    @FAIL_ROW #0x29B8
    JMP .t13

; ── Test 13 (row 12): NOT edge cases ─────────────────────────────────────────
.t13:
    LOAD R0, #0x0000
    NOT R1, R0
    @EXPECT_REG_J R1, #0xFFFF, .t13_fail

    LOAD R0, #0xFFFF
    NOT R1, R0
    @EXPECT_REG_J R1, #0x0000, .t13_fail

    LOAD R0, #0xAAAA
    NOT R1, R0                ; ~0xAAAA = 0x5555
    @EXPECT_REG_J R1, #0x5555, .t13_fail

    @PASS_ROW #0x29C8
    JMP .t14

.t13_fail:
    INC R8
    @FAIL_ROW #0x29C8
    JMP .t14

; ── Test 14 (row 13): PUSH×3 then POP×3 must yield values in LIFO order ──────
.t14:
    LOAD R0, #0x1111
    PUSH R0
    LOAD R0, #0x2222
    PUSH R0
    LOAD R0, #0x3333
    PUSH R0

    POP R3
    @EXPECT_REG_J R3, #0x3333, .t14_fail
    POP R4
    @EXPECT_REG_J R4, #0x2222, .t14_fail
    POP R5
    @EXPECT_REG_J R5, #0x1111, .t14_fail

    @PASS_ROW #0x29D8
    JMP .t15

.t14_fail:
    INC R8
    @FAIL_ROW #0x29D8
    JMP .t15

; ── Test 15 (row 14): two-level nested CALL / RET ────────────────────────────
; .nest_outer sets R5 = 0xAAAA then calls .nest_inner which sets R6 = 0xBBBB.
; Both values must survive back to the call site.
.t15:
    LOAD R5, #0x0000
    LOAD R6, #0x0000
    CALL .nest_outer
    @EXPECT_REG_J R5, #0xAAAA, .t15_fail
    @EXPECT_REG_J R6, #0xBBBB, .t15_fail

    @PASS_ROW #0x29E8
    JMP .t16

.t15_fail:
    INC R8
    @FAIL_ROW #0x29E8
    JMP .t16

; ── Test 16 (row 15): write then read back 4 sequential memory cells ──────────
.t16:
    LOAD R0, #0x0500
    LOAD R1, #0x1111
    STORE R0, R1              ; mem[0x0500] = 0x1111
    INC R0
    LOAD R1, #0x2222
    STORE R0, R1              ; mem[0x0501] = 0x2222
    INC R0
    LOAD R1, #0x3333
    STORE R0, R1              ; mem[0x0502] = 0x3333
    INC R0
    LOAD R1, #0x4444
    STORE R0, R1              ; mem[0x0503] = 0x4444

    LOAD R0, #0x0500
    LOAD R2, R0
    @EXPECT_REG_J R2, #0x1111, .t16_fail
    INC R0
    LOAD R2, R0
    @EXPECT_REG_J R2, #0x2222, .t16_fail
    INC R0
    LOAD R2, R0
    @EXPECT_REG_J R2, #0x3333, .t16_fail
    INC R0
    LOAD R2, R0
    @EXPECT_REG_J R2, #0x4444, .t16_fail

    @PASS_ROW #0x29F8
    JMP .done

.t16_fail:
    INC R8
    @FAIL_ROW #0x29F8

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
; Input: R12 = console address (row base + 8).
; Writes 7 chars: " : PASS" or " : FAIL".

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

.nest_outer:
    LOAD R5, #0xAAAA
    CALL .nest_inner
    RET

.nest_inner:
    LOAD R6, #0xBBBB
    RET
