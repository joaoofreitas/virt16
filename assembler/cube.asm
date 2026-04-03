; cube.asm — rotating wireframe cube on 32x32 graphics display
;
; Rotates a unit cube around both Y and X axes using Q7 fixed-point math.
; Sin/cos lookup tables and signed multiply come from the math library.
;
; Display: 32x32 pixels at 0x3000, pixel format 0xRGB0
; Data placement avoids overlap with sin (0x2A00) / cos (0x2B00) / VRAM (0x3000).

@import "lib/math.asm"

; Cube vertices: 8 x 3 words (x, y, z), half-size = 8 pixels
.PLACE 0x0900 [
    0xFFF8, 0xFFF8, 0xFFF8,
    0x0008, 0xFFF8, 0xFFF8,
    0x0008, 0x0008, 0xFFF8,
    0xFFF8, 0x0008, 0xFFF8,
    0xFFF8, 0xFFF8, 0x0008,
    0x0008, 0xFFF8, 0x0008,
    0x0008, 0x0008, 0x0008,
    0xFFF8, 0x0008, 0x0008
]

; Edge list: 12 x 2 words (start vertex, end vertex)
.PLACE 0x0920 [
    0x0000, 0x0001,
    0x0001, 0x0002,
    0x0002, 0x0003,
    0x0003, 0x0000,
    0x0004, 0x0005,
    0x0005, 0x0006,
    0x0006, 0x0007,
    0x0007, 0x0004,
    0x0000, 0x0004,
    0x0001, 0x0005,
    0x0002, 0x0006,
    0x0003, 0x0007
]

; Scratch memory map:
;   0x0880  angle_y          0x0886  vertex x (scratch)
;   0x0881  angle_x          0x0887  vertex y (scratch)
;   0x0882  sin_y            0x0888  vertex z (scratch)
;   0x0883  cos_y            0x0889  x1 (after Y rotation)
;   0x0884  sin_x            0x088A  z1 (after Y rotation)
;   0x0885  cos_x            0x088B  frame flag (set by timer ISR)
;   0x0940  projected coords (8 x 2 words: sx, sy)

.main:
    LOAD SP, #0x4000
    LOAD DISP, #0x3000
    LOAD R0, #0
    LOAD R1, #0x0880
    STORE R1, R0
    LOAD R1, #0x0881
    STORE R1, R0
    LOAD R1, #0x088B
    STORE R1, R0
    ; set up timer interrupt for frame pacing
    LOAD TVEC, #.timer_isr
    LOAD TPER, #0x6000
    EI
    JMP .frame

.frame:
    ; wait for timer interrupt to set the frame flag
    LOAD R0, #0x088B
    LOAD R0, R0
    LOAD R1, #1
    CMP R0, R1
    JNE .frame
    ; clear flag
    LOAD R0, #0x088B
    LOAD R1, #0
    STORE R0, R1
    ; render
    CALL .clear
    CALL .trig
    CALL .project
    CALL .edges
    ; advance angle_y by 3, angle_x by 1
    LOAD R2, #0x00FF
    LOAD R1, #0x0880
    LOAD R0, R1
    LOAD R3, #3
    ADD R0, R0, R3
    AND R0, R0, R2
    STORE R1, R0
    LOAD R1, #0x0881
    LOAD R0, R1
    INC R0
    AND R0, R0, R2
    STORE R1, R0
    JMP .frame

; Fill the 32x32 display with black
.clear:
    LOAD R0, #0x3000
    LOAD R1, #0
    LOAD R2, #0x3400
.clear_lp:
    STORE R0, R1
    INC R0
    CMP R0, R2
    JNE .clear_lp
    RET

; Cache sin/cos values for both rotation angles
.trig:
    LOAD R0, #0x0880
    LOAD R0, R0
    LOAD R1, #0x2A00
    ADD R2, R1, R0
    LOAD R2, R2
    LOAD R3, #0x0882
    STORE R3, R2
    LOAD R1, #0x2B00
    ADD R2, R1, R0
    LOAD R2, R2
    LOAD R3, #0x0883
    STORE R3, R2
    LOAD R0, #0x0881
    LOAD R0, R0
    LOAD R1, #0x2A00
    ADD R2, R1, R0
    LOAD R2, R2
    LOAD R3, #0x0884
    STORE R3, R2
    LOAD R1, #0x2B00
    ADD R2, R1, R0
    LOAD R2, R2
    LOAD R3, #0x0885
    STORE R3, R2
    RET

; Signed fixed-point multiply with arithmetic right shift by 7.
; in: R0, R1   out: R2 = (R0 * R1) >>> 7   clobbers: R3-R6
.fpm7:
    CALL .smul
    LOAD R3, #0x8000
    AND R4, R2, R3
    LOAD R3, #7
    SHR R2, R2, R3
    LOAD R3, #0x8000
    CMP R4, R3
    JNE .fpm7_done
    LOAD R3, #0xFE00
    OR R2, R2, R3
.fpm7_done:
    RET

; Project all 8 vertices: Y rotation, then X rotation, then to screen coords.
; R15 = vertex counter, R14 = projected-coords write pointer
.project:
    LOAD R15, #0
    LOAD R14, #0x0940

.proj_lp:
    ; vertex address = 0x0900 + index * 3
    ADD R7, R15, R15
    ADD R7, R7, R15
    LOAD R8, #0x0900
    ADD R7, R7, R8

    ; load x, y, z into scratch
    LOAD R8, R7
    LOAD R9, #0x0886
    STORE R9, R8
    INC R7
    LOAD R8, R7
    LOAD R9, #0x0887
    STORE R9, R8
    INC R7
    LOAD R8, R7
    LOAD R9, #0x0888
    STORE R9, R8

    ; Y rotation: x1 = fpm7(x, cos_y) + fpm7(z, sin_y)
    LOAD R0, #0x0886
    LOAD R0, R0
    LOAD R1, #0x0883
    LOAD R1, R1
    CALL .fpm7
    PUSH R2

    LOAD R0, #0x0888
    LOAD R0, R0
    LOAD R1, #0x0882
    LOAD R1, R1
    CALL .fpm7
    POP R3
    ADD R2, R3, R2
    LOAD R3, #0x0889
    STORE R3, R2

    ; Y rotation: z1 = fpm7(z, cos_y) - fpm7(x, sin_y)
    LOAD R0, #0x0888
    LOAD R0, R0
    LOAD R1, #0x0883
    LOAD R1, R1
    CALL .fpm7
    PUSH R2

    LOAD R0, #0x0886
    LOAD R0, R0
    LOAD R1, #0x0882
    LOAD R1, R1
    CALL .fpm7
    POP R3
    SUB R2, R3, R2
    LOAD R3, #0x088A
    STORE R3, R2

    ; X rotation: y2 = fpm7(y, cos_x) - fpm7(z1, sin_x)
    LOAD R0, #0x0887
    LOAD R0, R0
    LOAD R1, #0x0885
    LOAD R1, R1
    CALL .fpm7
    PUSH R2

    LOAD R0, #0x088A
    LOAD R0, R0
    LOAD R1, #0x0884
    LOAD R1, R1
    CALL .fpm7
    POP R3
    SUB R2, R3, R2

    ; x2 = x1 (from Y rotation, unchanged by X rotation)
    LOAD R3, #0x0889
    LOAD R3, R3

    ; screen coords: sx = 16 + x2, sy = 16 - y2
    LOAD R4, #16
    ADD R3, R4, R3
    SUB R2, R4, R2

    ; store projected screen coords
    STORE R14, R3
    INC R14
    STORE R14, R2
    INC R14

    INC R15
    LOAD R0, #8
    CMP R15, R0
    JNE .proj_lp
    RET

; Draw all 12 wireframe edges
.edges:
    LOAD R15, #0x0920
    LOAD R14, #12

.edge_lp:
    ; load vertex indices for this edge
    LOAD R0, R15
    INC R15
    LOAD R1, R15
    INC R15

    ; convert vertex index to projected-coords address (0x0940 + idx*2)
    ADD R0, R0, R0
    LOAD R2, #0x0940
    ADD R0, R0, R2
    ADD R1, R1, R1
    ADD R1, R1, R2

    ; load screen coords into R6-R9 for .line
    LOAD R6, R0
    INC R0
    LOAD R7, R0
    LOAD R8, R1
    INC R1
    LOAD R9, R1

    PUSH R15
    PUSH R14
    CALL .line
    POP R14
    POP R15

    DEC R14
    JZ .edges_done
    JMP .edge_lp

.edges_done:
    RET

; Bresenham line draw from (R6,R7) to (R8,R9).
; Uses R5=color, R10=dx, R11=dy(neg), R12=sx, R13=sy, R14=err.
.line:
    LOAD R5, #0xFFF0
    ; runaway guard: any line must finish within a bounded number of steps
    LOAD R15, #0x0080

    ; compute dx, sx
    LOAD R12, #1
    CMP R6, R8
    JL .ln_dxp
    JE .ln_dx0
    LOAD R12, #0xFFFF
    SUB R10, R6, R8
    JMP .ln_dy
.ln_dxp:
    SUB R10, R8, R6
    JMP .ln_dy
.ln_dx0:
    LOAD R10, #0
    LOAD R12, #0

.ln_dy:
    ; compute dy (stored negative), sy
    LOAD R13, #1
    CMP R7, R9
    JL .ln_dyp
    JE .ln_dy0
    LOAD R13, #0xFFFF
    SUB R11, R7, R9
    JMP .ln_err
.ln_dyp:
    SUB R11, R9, R7
    JMP .ln_err
.ln_dy0:
    LOAD R11, #0
    LOAD R13, #0

.ln_err:
    NOT R11, R11
    INC R11
    ADD R14, R10, R11

.ln_px:
    DEC R15
    JZ .ln_end

    ; bounds check: skip if x > 31 or y > 31 (unsigned, catches negatives too)
    LOAD R0, #31
    CMP R6, R0
    JG .ln_skip
    CMP R7, R0
    JG .ln_skip

    ; plot pixel: address = 0x3000 + y*32 + x
    LOAD R0, #5
    SHL R0, R7, R0
    ADD R0, R0, R6
    LOAD R1, #0x3000
    ADD R0, R0, R1
    STORE R0, R5

.ln_skip:
    ; done when current point equals end point
    CMP R6, R8
    JNE .ln_step
    CMP R7, R9
    JE .ln_end

.ln_step:
    ADD R0, R14, R14

    ; signed: if e2 >= dy then step x
    SUB R1, R0, R11
    LOAD R2, #0x8000
    AND R1, R1, R2
    CMP R1, R2
    JE .ln_chky
    ADD R14, R14, R11
    ADD R6, R6, R12

.ln_chky:
    ; signed: if e2 <= dx then step y
    SUB R1, R10, R0
    LOAD R2, #0x8000
    AND R1, R1, R2
    CMP R1, R2
    JE .ln_px
    ADD R14, R14, R10
    ADD R7, R7, R13
    JMP .ln_px

.ln_end:
    RET

; Timer ISR: set frame flag and return
.timer_isr:
    PUSH R0
    PUSH R1
    LOAD R0, #0x088B
    LOAD R1, #1
    STORE R0, R1
    POP R1
    POP R0
    RETI
