
.PLACE 0x3000 "TEXT" ;0x200 will have T on left side and E on right side (8bits each) and 0x201 will have X on left side and T on right side (8bits each)

.PLACE 0x3010 "TEXT"

@define DE #0xDE
@define AD #0xAD
@define BE #0xBE
@define EF #0xEF
@define ZERO #0x00
@define ONE #0x01

@define DISP #0x3000 ; Display address
@define SP #0x0200 ; Stack pointer

@macro PUSH_CLEAR_SCREEN [] 
    PUSH R0 ; Push R0 onto the stack
    PUSH R1 ; Push R1 onto the stack
    PUSH A ; Push A onto the stack

    LOAD A, #0xFF; Load 255 into A
    MOV R0, DISP ; Load the display address into R0
    LOAD R1, %ZERO ; Load 0 into R1
@endmacro

@macro POP_CLEAR_SCREEN []
    POP A ; Pop A from the stack
    POP R1 ; Pop R1 from the stack
    POP R0 ; Pop R0 from the stack
@endmacro

@macro ADD [n]
    LOAD R0, n ; Load n into R0
    ADD R1, R1, R0 ; Add R0 to R1
@endmacro

.CLEAR_SCREEN:
    STORE R0, R1; Store content of R1 (zero) into the display address in R0
    INC R0 ; Increment R0
    DEC A ; Decrement A
    CMP A, R1 ; Compare A to 0
    JNE .CLEAR_SCREEN 
    
    RET
    

.main: 
    LOAD SP, %SP ; Load the stack pointer
    LOAD DISP, %DISP ; Load the display address

    LOAD R1, %ONE ; Load 1 into R1

    @PUSH_CLEAR_SCREEN ; Push the registers for CLEAR_SCREEN
    CALL .CLEAR_SCREEN
    @POP_CLEAR_SCREEN ; POP the registers for CLEAR_SCREEN and Return

    @ADD %DE ; Add 0xDE to R1
    HLT ; Halt the program
