; test.asm — exercises the core instruction set
;
; After running, inspect memory at 0x0200-0x0203 in the Memory Viewer:
;   [0x0200] = 0x0010  (ADD: 5+3=8, then doubled by subroutine = 16)
;   [0x0201] = 0x0002  (SUB: 5-3=2)
;   [0x0202] = 0xABCD  (PUSH/POP round-trip)
;   [0x0203] = 0xBEEF  (success marker — only written if all tests pass)
;
; If the VM halts before writing 0xBEEF, a test failed.

.main:
    LOAD SP, #0x4000    ; init stack at 0x4000 (grows downward)

    ; --- LOAD immediate / MOV ---
    LOAD R0, #0x0005
    LOAD R1, #0x0003
    MOV R2, R0          ; R2 = 5

    ; --- INC / DEC ---
    INC R2              ; R2 = 6
    DEC R2              ; R2 = 5  (back to original)

    ; --- ADD: R3 = R0 + R1 = 8 ---
    ADD R3, R0, R1

    ; --- SUB: R4 = R0 - R1 = 2 ---
    SUB R4, R0, R1

    ; --- CMP + conditional jump ---
    CMP R0, R1          ; 5 > 3, sets G flag
    JG .after_cmp       ; should jump; if not, HLT below is the failure signal
    HLT

.after_cmp:
    ; --- PUSH / POP ---
    LOAD R5, #0xABCD
    PUSH R5
    LOAD R5, #0x0000    ; clobber R5
    POP R5              ; R5 should be 0xABCD again

    ; --- CALL / RET: double R3 via subroutine ---
    CALL .double_r3     ; R3 = R3 * 2 = 16

    ; --- store results for inspection ---
    LOAD R7, #0x0200
    STORE R7, R3        ; [0x0200] = 0x0010
    INC R7
    STORE R7, R4        ; [0x0201] = 0x0002
    INC R7
    STORE R7, R5        ; [0x0202] = 0xABCD

    ; --- success marker ---
    LOAD R0, #0xBEEF
    INC R7
    STORE R7, R0        ; [0x0203] = 0xBEEF

    HLT

.double_r3:
    ADD R3, R3, R3      ; R3 = R3 + R3
    RET
