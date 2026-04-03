; interrupt_test.asm — timer and keyboard interrupt test bench
;
; Prints 5 rows to the 16x16 console:
;   Row 0  TIMER ON  — timer ISR fires when EI is active
;   Row 1  TIMER DI  — timer ISR blocked when DI
;   Row 2  RETI OK   — RETI resumes execution and re-enables interrupts
;   Row 3  ISR SAFE  — ISR preserves caller registers via PUSH/POP
;   Row 4  TPER DIS  — TPER=0 disables timer even with EI
;
; Scratch convention (shared, not saved across tests):
;   R9, R10  — scratch for ISRs and write routines
;   R11      — sentinel register for RETI test
;   R12      — console address for write subroutines
;   R13      — loop counter
;   R15      — zero constant (set once in .main, never modified by ISRs)
;
; ISR counter addresses:
;   0x0200  — timer fires counter (test 0)
;   0x0201  — timer blocked counter (test 1 — should remain 0)
;   0x0202  — RETI marker (test 2 — ISR sets to 1)
;   0x0203  — ISR safe counter (test 3)
;   0x0204  — TPER=0 counter (test 4 — should remain 0)

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

; ── Console rows ──────────────────────────────────────────────────────────────
.PLACE 0x2900 "TIMER ON        "
.PLACE 0x2910 "TIMER DI        "
.PLACE 0x2920 "RETI OK         "
.PLACE 0x2930 "ISR SAFE        "
.PLACE 0x2940 "TPER DIS        "

; ── Macros ────────────────────────────────────────────────────────────────────

@macro PASS_ROW [addr]
    LOAD R12, addr
    CALL .write_pass
@endmacro

@macro FAIL_ROW [addr]
    LOAD R12, addr
    CALL .write_fail
@endmacro

; ── Main ──────────────────────────────────────────────────────────────────────
.main:
    LOAD SP, #0x4000
    LOAD DISP, #0x3000
    LOAD R15, #0x0000           ; R15 = 0 constant (ISRs must not touch it)

    ; Zero ISR counters
    LOAD R9, #0x0200
    STORE R9, R15
    INC R9
    STORE R9, R15               ; mem[0x0201] = 0
    INC R9
    STORE R9, R15               ; mem[0x0202] = 0
    INC R9
    STORE R9, R15               ; mem[0x0203] = 0
    INC R9
    STORE R9, R15               ; mem[0x0204] = 0

    ; ── Test 0: TIMER ON ──────────────────────────────────────────────────────
    ; TPER=20, loop 300 iters (~900 steps) → ~45 timer fires expected
    LOAD TVEC, #.isr_count
    LOAD TPER, #20
    EI
    LOAD R13, #300
    JMP .t0_loop

.t0_loop:
    DEC R13
    CMP R13, R15
    JNE .t0_loop
    DI
    LOAD R9, #0x0200
    LOAD R0, R9                 ; R0 = fire count
    CMP R0, R15                 ; count > 0?
    JE .t0_fail
    @PASS_ROW #0x2908
    JMP .t1

.t0_fail:
    @FAIL_ROW #0x2908
    JMP .t1

; ── Test 1: TIMER DI ──────────────────────────────────────────────────────────
; Same period but interrupts disabled — counter must stay 0
.t1:
    LOAD TVEC, #.isr_count2
    LOAD TPER, #20
    DI
    LOAD R13, #300
    JMP .t1_loop

.t1_loop:
    DEC R13
    CMP R13, R15
    JNE .t1_loop
    LOAD R9, #0x0201
    LOAD R0, R9                 ; R0 = counter (must be 0)
    CMP R0, R15
    JNE .t1_fail
    @PASS_ROW #0x2918
    JMP .t2

.t1_fail:
    @FAIL_ROW #0x2918
    JMP .t2

; ── Test 2: RETI OK ───────────────────────────────────────────────────────────
; ISR sets mem[0x0202]=1 then RETI. Verify execution resumes (R11 unchanged)
; and the marker was written.
.t2:
    LOAD TVEC, #.isr_reti
    LOAD TPER, #20
    LOAD R11, #0xBEEF           ; sentinel — ISR must not corrupt R11
    EI
    LOAD R13, #200
    JMP .t2_loop

.t2_loop:
    DEC R13
    CMP R13, R15
    JNE .t2_loop
    DI
    ; Check marker written by ISR
    LOAD R9, #0x0202
    LOAD R0, R9
    CMP R0, R15
    JE .t2_fail                 ; marker == 0 → ISR never fired
    ; Check sentinel unchanged
    LOAD R9, #0xBEEF
    CMP R11, R9
    JNE .t2_fail
    @PASS_ROW #0x2928
    JMP .t3

.t2_fail:
    @FAIL_ROW #0x2928
    JMP .t3

; ── Test 3: ISR SAFE ──────────────────────────────────────────────────────────
; ISR pushes/pops R1 and R2, so caller values must survive.
.t3:
    LOAD R1, #0xAAAA
    LOAD R2, #0xBBBB
    LOAD TVEC, #.isr_safe
    LOAD TPER, #20
    EI
    LOAD R13, #200
    JMP .t3_loop

.t3_loop:
    DEC R13
    CMP R13, R15
    JNE .t3_loop
    DI
    ; Verify R1 and R2 survived the ISR
    LOAD R9, #0xAAAA
    CMP R1, R9
    JNE .t3_fail
    LOAD R9, #0xBBBB
    CMP R2, R9
    JNE .t3_fail
    @PASS_ROW #0x2938
    JMP .t4

.t3_fail:
    @FAIL_ROW #0x2938
    JMP .t4

; ── Test 4: TPER DIS ──────────────────────────────────────────────────────────
; TPER=0 disables timer. EI active but counter at 0x0204 must stay 0.
.t4:
    LOAD TVEC, #.isr_tper0
    LOAD TPER, #0               ; disabled
    EI
    LOAD R13, #300
    JMP .t4_loop

.t4_loop:
    DEC R13
    CMP R13, R15
    JNE .t4_loop
    DI
    LOAD R9, #0x0204
    LOAD R0, R9                 ; must be 0
    CMP R0, R15
    JNE .t4_fail
    @PASS_ROW #0x2948
    HLT

.t4_fail:
    @FAIL_ROW #0x2948
    HLT

; ── Write subroutines ─────────────────────────────────────────────────────────
; Input: R12 = console address (row base + 8).

.write_pass:
    LOAD R9, #0x0020
    STORE R12, R9
    INC R12
    LOAD R9, #0x003A
    STORE R12, R9
    INC R12
    LOAD R9, #0x0020
    STORE R12, R9
    INC R12
    LOAD R9, #0x0050
    STORE R12, R9
    INC R12
    LOAD R9, #0x0041
    STORE R12, R9
    INC R12
    LOAD R9, #0x0053
    STORE R12, R9
    INC R12
    STORE R12, R9
    RET

.write_fail:
    LOAD R9, #0x0020
    STORE R12, R9
    INC R12
    LOAD R9, #0x003A
    STORE R12, R9
    INC R12
    LOAD R9, #0x0020
    STORE R12, R9
    INC R12
    LOAD R9, #0x0046
    STORE R12, R9
    INC R12
    LOAD R9, #0x0041
    STORE R12, R9
    INC R12
    LOAD R9, #0x0049
    STORE R12, R9
    INC R12
    LOAD R9, #0x004C
    STORE R12, R9
    RET

; ── ISR: isr_count — increment counter at 0x0200 (test 0) ────────────────────
.isr_count:
    LOAD R9, #0x0200
    LOAD R10, R9
    INC R10
    STORE R9, R10
    RETI

; ── ISR: isr_count2 — increment counter at 0x0201 (test 1) ───────────────────
.isr_count2:
    LOAD R9, #0x0201
    LOAD R10, R9
    INC R10
    STORE R9, R10
    RETI

; ── ISR: isr_reti — set mem[0x0202] = 1, then RETI (test 2) ─────────────────
.isr_reti:
    LOAD R9, #0x0202
    LOAD R10, #0x0001
    STORE R9, R10
    RETI

; ── ISR: isr_safe — preserve R1/R2, increment counter at 0x0203 (test 3) ─────
.isr_safe:
    PUSH R1
    PUSH R2
    LOAD R9, #0x0203
    LOAD R10, R9
    INC R10
    STORE R9, R10
    LOAD R1, #0x0000            ; clobber R1/R2 intentionally
    LOAD R2, #0x0000
    POP R2
    POP R1
    RETI

; ── ISR: isr_tper0 — increment counter at 0x0204 (test 4, should never fire) ─
.isr_tper0:
    LOAD R9, #0x0204
    LOAD R10, R9
    INC R10
    STORE R9, R10
    RETI
