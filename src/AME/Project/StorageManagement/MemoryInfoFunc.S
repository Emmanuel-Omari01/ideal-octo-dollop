/* AME code */

    .extern _ame_fill_stack_return
    .global _ame_fill_stack_pattern
_ame_fill_stack_pattern:
/* fill stack with pattern */
    .set _stack_pattern, 3Fh
    .global _stack_pattern
    mov    #_stack_pattern,r2
    mov    #1000h, r3   // stack-size
    mov    #_ustack, r1 // end of stack
    sub    r3,r1        /* size of stack in R3 (R1=R1-R3) */
#ifdef __RX_ALLOW_STRING_INSNS__
    sstr.b
#else
1:  cmp     r1, r3
    beq     2f
    mov.b   r2, [r1+]
    bra     1b
2:
#endif

    mov    #_stack_pattern,r2
    mov    #1000h, r3   // stack-size
    mov    #_istack, r1 // end of stack
    sub    r3,r1        /* size of stack in R3 (R1=R1-R3) */
#ifdef __RX_ALLOW_STRING_INSNS__
    sstr.b
#else
1:  cmp     r1, r3
    beq     2f
    mov.b   r2, [r1+]
    bra     1b
2:
#endif
    bra _ame_fill_stack_return


    .global	_get_rom_size
_get_rom_size:
	mov		#etext, r1
	mov		#stext, r2
	sub		r2,r1
	rts

    .global	_get_used_var_size
_get_used_var_size:
	mov		#_end,r1
	rts

	.global _get_end_of_ustack
_get_end_of_ustack:
	mov     #_ustack, r1
//	mvfc	USP, r2
//	sub		r2,r1
	rts

	.global _get_end_of_istack
_get_end_of_istack:
	mov     #_istack, r1
//	mvfc	ISP, r2
//	sub		r2,r1
	rts
/* End of AME code */
