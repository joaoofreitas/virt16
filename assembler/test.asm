; test.asm — full instruction set test bench
;
; Runs every instruction group in order.
; On the first mismatch execution jumps to .fail.
;
; After running, check mem[0x0300] in the Memory Viewer:
;   0xBEEF = all tests passed
;   0xDEAD = something failed (step through in debug to find where)
;
; R15 is reserved as a scratch register for EXPECT macros.
; R14 is reserved for the result address (0x0300).

; ── Macros ───────────────────────────────────────────────────────────────────

; EXPECT val  — asserts R0 == val, jumps to .fail otherwise
@macro EXPECT [val]
    LOAD R15, val
    CMP R0, R15
    JNE .fail
@endmacro

; EXPECT_REG reg, val  — asserts reg == val, jumps to .fail otherwise
@macro EXPECT_REG [reg, val]
    LOAD R15, val
    CMP reg, R15
    JNE .fail
@endmacro

; ── Section 1: LOAD / MOV / INC / DEC ────────────────────────────────────────
.main:
    LOAD SP, #0x4000
    LOAD R14, #0x0300

    ; LOAD immediate
    LOAD R0, #0x0042
    @EXPECT #0x0042

    ; MOV
    LOAD R1, #0x00FF
    MOV R0, R1
    @EXPECT #0x00FF

    ; INC
    INC R0                  ; 0x00FF -> 0x0100
    @EXPECT #0x0100

    ; DEC
    DEC R0                  ; 0x0100 -> 0x00FF
    @EXPECT #0x00FF

    ; NOP (no result to check; if execution continues past it, NOP is correct)
    NOP

    ; ── Section 2: ADD / SUB / Z flag ────────────────────────────────────────
    LOAD R0, #0x0005
    LOAD R1, #0x0003

    ; ADD: R2 = 5 + 3 = 8
    ADD R2, R0, R1
    @EXPECT_REG R2, #0x0008

    ; SUB: R3 = 5 - 3 = 2
    SUB R3, R0, R1
    @EXPECT_REG R3, #0x0002

    ; Z flag auto-set: R0 - R0 = 0, Z must be 1 -> JZ succeeds
    SUB R4, R0, R0
    JZ .test_bitwise
    JMP .fail

; ── Section 3: AND / OR / XOR / NOT / SHL / SHR ─────────────────────────────
.test_bitwise:
    LOAD R0, #0xFF00
    LOAD R1, #0x0FF0

    ; AND: 0xFF00 & 0x0FF0 = 0x0F00
    AND R2, R0, R1
    @EXPECT_REG R2, #0x0F00

    ; OR: 0xFF00 | 0x0FF0 = 0xFFF0
    OR R2, R0, R1
    @EXPECT_REG R2, #0xFFF0

    ; XOR: 0xFF00 ^ 0x0FF0 = 0xF0F0
    XOR R2, R0, R1
    @EXPECT_REG R2, #0xF0F0

    ; NOT: ~0xFF00 = 0x00FF
    NOT R2, R0
    @EXPECT_REG R2, #0x00FF

    ; SHL: 1 << 4 = 0x0010
    LOAD R0, #0x0001
    LOAD R1, #0x0004
    SHL R2, R0, R1
    @EXPECT_REG R2, #0x0010

    ; SHR: 0x0100 >> 4 = 0x0010
    LOAD R0, #0x0100
    SHR R2, R0, R1
    @EXPECT_REG R2, #0x0010

    JMP .test_flow

; ── Section 4: JMP / JE / JNE / JG / JL ─────────────────────────────────────
.test_flow:
    ; JE: equal values must branch
    LOAD R0, #0x1234
    LOAD R1, #0x1234
    CMP R0, R1
    JE .test_flow_jne
    JMP .fail

.test_flow_jne:
    ; JNE: unequal values must branch
    LOAD R0, #0x0001
    LOAD R1, #0x0002
    CMP R0, R1
    JNE .test_flow_jg
    JMP .fail

.test_flow_jg:
    ; JG: greater must branch (5 > 3)
    LOAD R0, #0x0005
    LOAD R1, #0x0003
    CMP R0, R1
    JG .test_flow_jl
    JMP .fail

.test_flow_jl:
    ; JL: less must branch (3 < 5)
    CMP R1, R0
    JL .test_stack
    JMP .fail

; ── Section 5: PUSH / POP / CALL / RET / STORE / LOAD indirect ───────────────
.test_stack:
    ; PUSH / POP round-trip
    LOAD R0, #0x1234
    PUSH R0
    LOAD R0, #0x0000        ; clobber R0
    POP R0                  ; must restore 0x1234
    @EXPECT #0x1234

    ; CALL / RET: double R0 via subroutine (0x1234 * 2 = 0x2468)
    CALL .double
    @EXPECT #0x2468

    ; STORE then LOAD indirect: write 0xBEEF to 0x0400, read it back
    LOAD R0, #0xBEEF
    LOAD R1, #0x0400
    STORE R1, R0            ; mem[0x0400] = 0xBEEF
    LOAD R2, R1             ; R2 = mem[R1] = mem[0x0400] = 0xBEEF
    @EXPECT_REG R2, #0xBEEF

    ; ── all tests passed ──────────────────────────────────────────────────────
    LOAD R0, #0xBEEF
    STORE R14, R0           ; mem[0x0300] = 0xBEEF
    HLT

; ── Failure handler ───────────────────────────────────────────────────────────
.fail:
    LOAD R0, #0xDEAD
    STORE R14, R0           ; mem[0x0300] = 0xDEAD
    HLT

; ── Subroutines ───────────────────────────────────────────────────────────────
.double:
    ADD R0, R0, R0          ; R0 = R0 * 2
    RET
