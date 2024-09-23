
.PLACE 0x200 "TEXT" ;0x200 will have T on left side and E on right side (8bits each) and 0x201 will have X on left side and T on right side (8bits each)

@define DE 0xDE
@define AD 0xAD
@define BE 0xBE
@define EF 0xEF
@define ZERO 0x00
@define ONE 0x01

@macro PUSH_CLEAR_SCREEN [] 
    PUSH R0 ; Push R0 onto the stack
    PUSH A ; Push A onto the stack
@endmacro

@macro POP_CLEAR_SCREEN []
    POP A ; Pop A from the stack
    POP R0 ; Pop R0 from the stack
    RET ; Return from the subroutine
@endmacro

@macro ADD [n]
    ADD R1, n ; Add n to R1
@endmacro

.CLEAR_SCREEN:
    LOAD A, 0xFF; Load 255 into A
    LOAD R0, DISP ; Load the display address into R0
    STORE R0, %ZERO ; Store 0 into the display address
    INC R0 ; Increment R0
    DEC A ; Decrement A
    CMP A, #0 ; Compare A to 0
    JNE CLEAR_SCREEN ; Remove this cause CLEAR_SCREEN is not a routine

    @POP_CLEAR_SCREEN ; POP the registers for CLEAR_SCREEN and Return
    

main: 
    LOAD R1, %ONE ; Load 1 into R1

    @PUSH_CLEAR_SCREEN ; Push the registers for CLEAR_SCREEN
    CALL CLEAR_SCREEN
    @ADD %DE ; Add 0xDE to R1
    HLT ; Halt the program
