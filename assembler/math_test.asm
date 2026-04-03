; math_test.asm — smoke test for @import and math library
; Tests: MUL, SMUL, table lookup for sin/cos

@import "lib/math.asm"

.main:
    LOAD SP, #0x4000

    ; Test MUL: 3 * 7 = 21 (0x0015)
    LOAD R7, #3
    LOAD R8, #7
    @MUL R9, R7, R8         ; R9 = 21

    ; Test SMUL: 5 * (-3) — stored as 0xFFFD
    LOAD R7, #5
    LOAD R8, #0xFFFD        ; -3 in two's complement
    @SMUL R9, R7, R8        ; R9 = -15 = 0xFFF1

    ; Test FPMUL8: sin(64) * 128 → Q8 * Q8 → should give sin(90°) ≈ 128
    LOAD R10, #0x2A00
    LOAD R11, #64
    ADD R10, R10, R11       ; R10 = address of sin[64]
    LOAD R7, R10            ; R7 = sin[64] = 0x0080 (128)
    LOAD R8, #0x0080        ; 128 (= 1.0 in Q8)
    @FPMUL8 R9, R7, R8      ; R9 = (128*128)>>8 = 16384>>8 = 64... hmm 0.5

    HLT
