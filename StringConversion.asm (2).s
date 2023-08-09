; StringConversion.asm
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 number to string conversions
;
; Used in ECE319K Labs 7,8,9,10. You write these two functions

 
    .global   Dec2String
    .global   Fix2String
       .thumb
       .data
       .align 4
;global variables go here
Ran   .space  4
       .text
       .align  4

count      .long 0
count_push .long 0
count_pop  .long 4


  
;-----------------------Dec2String-----------------------
; Convert a 32-bit number into unsigned decimal format
; String the string into the empty array add null-termination
; Input: R0 (call by value) 32-bit unsigned number
;        R1 pointer to empty array
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
Dec2String:  .asmfunc
; binding
   	;count = 0 (no EQU pseudo op, see above global variables)
; allocation
    PUSH {R11, LR}
    SUB R13, #4    ;allocate one 32-bit wide space on stack
    MOV R11, R13
; initialization
    MOV R2, #0      ; initialize local variable count to zero
    LDR R3, count
    STR R2, [R11, R3]
    PUSH {R4, R5}
; main operation
loop_push_digit
    ;d = n % 10
    MOV R2, #10 
    UDIV  R3, R0, R2      ; div to get quotient, n//10
    MUL   R4, R3, R2      ; need for computing remainder, (n//10) * 10
    SUB   R3, R0, R4      ; the mod (remainder), d = n % 10
    PUSH {R3}             ; push digit onto stack
    ;count += 1
    LDR R3, count
    LDR R2, [R11, R3]
    ADD R2, #1
    STR R2, [R11, R3]
    ;n=n//10
    MOV R2, #10
    UDIV R0, R0, R2
    CMP R0, #0
    BEQ loop_pop_ascii
    ;if not zero, loop_push

    B loop_push_digit

loop_pop_ascii
    ;break if count reaches zero
    LDR R3, count
    LDR R2, [R11, R3]
    CMP R2, #0
    BEQ end_dec_string

    ; buf[i] = (d+'0'); i+=1
    POP {R2}
    ADD R2, #0x30
    STRB R2, [R1], #1

    ;count -= 1
    LDR R3, count
    LDR R2, [R11, R3]
    SUB R2, #1
    STR R2, [R11, R3]
    B loop_pop_ascii

end_dec_string
;write null character to end
    MOV  R2, #0
    STRB R2, [R1]

;deallocation
    POP {R4, R5}
    ADD R13, #4
    POP {R11, PC}
    .endasmfunc
;* * * * * * * * End of Dec2String * * * * * * * *


; -----------------------Fix2String----------------------
; Create characters for LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
;          R1 pointer to empty array
; Outputs: none
; E.g., R0=0,    then create "0.000 "
;       R0=3,    then create "0.003"
;       R0=89,   then create "0.089"
;       R0=123,  then create "0.123"
;       R0=9999, then create "9.999"
;       R0>9999, then create "*.***"
; Invariables: This function must not permanently modify registers R4 to R11
Fix2String:  .asmfunc
; binding
	;count_push = 0
	;count_pop  = 4
; allocation
    PUSH {R11, LR}
    SUB R13, #8    ;allocate two 32-bit wide space on stack for local variables
    MOV R11, R13
; initialization
    MOV R2, #0      ; initialize local variable count to zero
    LDR R3, count_push
    STR R2, [R11, R3]
    PUSH {R4, R5}
; main operation

    ;check if number is too large for fixed point
    MOV R2, #9999
    CMP R0, R2
    BHI too_big

loop_push_fix_digit
    ;d = n % 10
    MOV R2, #10 
    UDIV  R3, R0, R2      ; div to get quotient, n//10
    MUL   R4, R3, R2      ; need for computing remainder, (n//10) * 10
    SUB   R3, R0, R4      ; the mod (remainder), d = n % 10
	PUSH {R3}             ; push digit onto stack
	;count += 1
    LDR R3, count_push
    LDR R2, [R11, R3]
    ADD R2, #1
    STR R2, [R11, R3]

    ;n=n//10
    MOV R2, #10
    UDIV R0, R0, R2

    CMP R0, #0
    BEQ init_fixed_string

    ;if not zero, loop
    B loop_push_fix_digit

init_fixed_string
    ;write 0.000 to buffer to initialize the string
    ;the main loop will overwrite these values starting from the LSB and exit leaving the correct amount of zeros in place
    MOV R2, #0x30       ;ascii 0  = 0x30
    MOV R3, #46         ;ascii .  = 46
    MOV R4, #32			;ascii [space character] = 32
    MOV R5, #0			;null
    STRB R2, [R1], #1
    STRB R3, [R1], #1
    STRB R2, [R1], #1
    STRB R2, [R1], #1
    STRB R2, [R1], #1
    STRB R4, [R1], #1
    STRB R5, [R1]
    LDR R3, count_push
    LDR R2, [R11, R3]
    CMP R2, #4
    BHS bandaid_fix_for_4_digits
cont
    ADD R2, #1
    SUB R1, R2
	B loop_pop_fixed

bandaid_fix_for_4_digits
	SUB R1, #1
	B cont


loop_pop_fixed
    ;break if count_push reaches 0
    LDR R3, count_push
    LDR R2, [R11, R3]
    CMP R2, #0
    BEQ deallocate

    POP {R2}
    ADD  R2, #0x30
    STRB R2, [R1], #1

    LDR R3, count_push
    LDR R2, [R11, R3]
    CMP R2, #4
    BEQ last_bandaid
    b cont_sorry
last_bandaid
    ADD R1, #1
    
cont_sorry
    ;count_push -= 1
    LDR R3, count_push
    LDR R2, [R11, R3]
    SUB R2, #1
    STR R2, [R11, R3]

    ;count_pop +=1
    LDR R3, count_pop
    LDR R2, [R11, R3]
    ADD R2, #1
    STR R2, [R11, R3]
    
    B loop_pop_fixed

    
too_big
    ;write *.*** string to buf then exit with deallocation
    MOV  R2, #42         ;ascii *  = 42
    MOV  R3, #46         ;ascii .  = 46  
    MOV  R4, #0
    STRB R2, [R1], #1   ;store then increment, all ascii characters are one byte
    STRB R3, [R1], #1    
    STRB R2, [R1], #1
    STRB R2, [R1], #1
    STRB R2, [R1], #1
    STRB R4, [R1]       ;null character to end string
    B deallocate

deallocate
    POP {R4, R5}
    ADD R13, #8
    POP {R11, PC}
    .endasmfunc

    .end                             ; end of file
