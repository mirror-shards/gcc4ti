|******************************************************************************
|
| project name:    GrayScale-Support for TIGCC
| author:          thomas.nussbaumer@gmx.net
|                  Julien Muchembled (original implementation for UniversalOS)
|
| compatible with HW1/HW2/HW3/HW4 on all AMS versions up to 3.10
|
|******************************************************************************

|------------------------------------------------------------------------------
| uncomment the following global to simulate HW2 on the VTI
| (this will not use port 70001D, therefore it flickers extremely; additionally
|  the complete HW detection is bypassed and always reports HW2)
|------------------------------------------------------------------------------
|.globl ALWAYS_HW2_TESTING
|------------------------------------------------------------------------------
| comment the following global to allocate the same amount of memory on HW1 as
| on newer models
|------------------------------------------------------------------------------
.globl ALLOCATE_LESS_ON_HW1
ALLOCATE_LESS_ON_HW1:


	.xdef GrayOn,GrayOff,__gray_D_plane,__D_plane,__gray_L_plane,__L_plane,__gray_handle,__gray_hw_type
	.xdef __switch_cnt,__gray_switch_cnt,__gray_old_int1_hw1,__gray_old_int1_hw2,__gray_old_int1_handler
	.xdef __gray_sync_n_count,__gray_plane_index
	.xdef __gray_dbl_offset,__gray_L_plane2,__L_plane2,__gray_D_plane2,__D_plane2

.even
|==============================================================================
| EXPORTED: GrayOn function (turn grayscales on) - trashes d1/d2/a0/a1
|==============================================================================
GrayOn:
	move.w   (__gray_handle,%pc),%d0        | if __gray_handle is not 0 we have
	bne      __gray_init_return_1           | already allocated memory -> out here
	move.l   0xc8.w,%a1
	lea      (__gray_switch_cnt,%pc),%a0    | reset plane switch counter to 0
	clr.l    (%a0)

|==============================================================================
| checks for HW version (VTI is treated as HW1, because port 0x70001D is not
|                        emulated by the VTI and this would cause NEVER switch
|                        planes behaviour if we would use the HW2 support)
|
| IMPORTANT NOTE: This function patches 2 locations in the code of the
|                 grayscale support depending on the HW version. Patching the
|                 locations for both types IS necessary, because if a program
|                 is transfered from one calc to another one the default values
|                 got already overwritten (if program was not archived)
|==============================================================================
__gray_check_hw_version:
.ifndef ALWAYS_HW2_TESTING
	move.l   %a1,%d0
	and.l    #0xE00000,%d0          | get the ROM base
	move.l   %d0,%a0
	move.l   0x104(%a0),%a0         | get pointer to the hardware param block
	add.l    #0x10000,%d0
	cmp.l    %d0,%a0                | check if the HW parameter block is near
	bcc.s    __gray_patches_for_hw1 | if it is too far, it is HW1
	cmp.w    #22,(%a0)              | check if the parameter block contains HW
	bls.s    __gray_patches_for_hw1 | if it is too small, it is HW1
	moveq    #1,%d1
	cmp.l    22(%a0),%d1            | check the hardware version
	beq.s    __gray_patches_for_hw1 | if not 1, it is HW2 (or an unknown HW)
    |--------------------------------------------------------------------------
    | check for VTI (trick suggested by Julien Muchembled)
    | optimized by Lionel Debroux
    |--------------------------------------------------------------------------
	trap     #12         | enter supervisor mode. returns old (%sr) in %d0.w
	move.w   #0x3000,%sr | set a non-existing flag in %sr (but keep s-flag !!)
	move.w   %d0,%d1     | save %d0.w content in %d1
	move.w   %sr,%d0     | get %sr content and check for non-existing flag
	move.w   %d1,%sr     | restore old %sr.
	lsl.w    #3,%d0
	bpl.s    __gray_hw2type_detected  | flag not set -> no VTI
    |--------------------------------------------------------------------------
    | HW1 detected
    |--------------------------------------------------------------------------
__gray_patches_for_hw1:
	moveq    #0,%d0      | Reset hwtype to 0 (on VTI, in this code path, it wasn't)
.ifdef ALLOCATE_LESS_ON_HW1
	moveq    #0xF,%d1    | Set HW1 value for patching code
	moveq    #0,%d2      | Set HW1 value for patching code
.endif
	bra.s    __gray_init_hwtype_in_d0
    |--------------------------------------------------------------------------
    | HW2 detected
    |--------------------------------------------------------------------------
.endif | ALWAYS_HW2_TESTING
__gray_hw2type_detected:
	moveq    #1,%d0      | Set hwtype to 1
.ifdef ALLOCATE_LESS_ON_HW1
	moveq    #0x1E,%d1   | Set HW2 value for patching code
	moveq    #0xF,%d2    | Set HW2 value for patching code
.endif
    |--------------------------------------------------------------------------
    | patches code according to HW version
    |
    | necessary memory for HW1 == 1 plane + 8 Bytes == 3848
    | necessary memory for HW2 == 2 planes + 8 Bytes == 7688
    | if not ALLOCATE_LESS_ON_HW1, then 7688 for both
    |
    | the additional 8 bytes are necessary for rounding to a multiple of 8
    |--------------------------------------------------------------------------
__gray_init_hwtype_in_d0:
	lea     (__gray_hw_type,%pc),%a0
.ifdef ALLOCATE_LESS_ON_HW1
	move.b   %d1,(__gray_size_to_allocate - __gray_hw_type,%a0)
	move.b   %d2,(__gray_size_to_add - __gray_hw_type,%a0)
.endif

	move.w   %d0,(%a0)

|==============================================================================
| INTERNAL: allocates memory
|
| modifies: __gray_handle
|           __gray_used_mem
|           __gray_L_plane
|
| Note: __gray_D_plane will not be changed by this function! (will be set by
|                                                        __gray_init_handler)
|==============================================================================
__gray_init_mem:
    |--------------------------------------------------------------------------
    | HeapAllocHigh(HW1=3848 bytes unless ALLOCATE_LESS_ON_HW1 or HW2=7688 bytes)
    |--------------------------------------------------------------------------
	movea.l  (0x92*4,%a1),%a0 /* HeapAllocHigh */
	.word    0x4878                       | opcode of "PEA value.w"
__gray_size_to_allocate:                      | the size gets patched !!
	.word    0x1e08
	jsr      (%a0)
	addq.l   #4,%sp
	lea      (__gray_handle,%pc),%a0
	move.w   %d0,(%a0)+                   | store handle in handle variable
	beq      __gray_init_rts              | alloc failed (handle=0) -> out here
	clr.w    (%a0)                        | clears __gray_dbl_offset
    |--------------------------------------------------------------------------
    | HeapDeref(__gray_handle)
    |--------------------------------------------------------------------------
	move.l   0xc8.w,%a0
	move.w   %d0,-(%sp)
	movea.l  (0x96*4,%a0),%a0 /* HeapDeref */
	jsr      (%a0)
	addq.l   #2,%sp
    |--------------------------------------------------------------------------
    | align memory address to next 8-byte boundary and store address in
    | __gray_used_mem
    |
    | for HW1: __gray_L_plane gets set to the same address as __gray_used_mem,
    | unless ALLOCATE_LESS_ON_HW1
    | for HW2: __gray_L_plane gets set to __gray_used_mem + 0xf00
    |--------------------------------------------------------------------------
	move.l   %a0,%d0
	addq.l   #7,%d0
	andi.b   #0xF8,%d0
	lea      (__gray_used_mem,%pc),%a0
	move.l   %d0,(%a0)
	.word    0x0680              | opcode of "addi.l #value,%d0"
	.word    0x0000
__gray_size_to_add:
	.word    0x0F00              | gets patched (HW1:0, HW2 or ALLOCATE_LESS_ON_HW1:0x0f00)
	move.l   %d0,(__gray_L_plane - __gray_used_mem,%a0)

|==============================================================================
| INTERNAL: initialize grayscale handler
|==============================================================================
__gray_init_handler:
	lea      (__gray_L_plane,%pc),%a0
	move.w   #0x3BF,%d1
    |--------------------------------------------------------------------------
    | (1) "backup" old INT1 handler in __gray_old_int1_handler (the address part
    |     of a JUMP address instruction at the end of the HW1 int handler)
    |--------------------------------------------------------------------------
	move.l   0x64.w,(__gray_old_int1_handler - __gray_L_plane,%a0)
	tst.w    (__gray_hw_type - __gray_L_plane,%a0)
	beq.s    __gray_init_hw1_handler

    |--------------------------------------------------------------------------
    | HW2 specific initializations:
    |
    | (1) set __gray_D_plane to __gray_used_mem
    | (2) copy content of 0x4c00 to darkplane
    |--------------------------------------------------------------------------
	movea.l  (__gray_used_mem,%pc),%a1
	move.l   %a1,(0x4,%a0)               | set __gray_D_plane
	lea      0x4C00.w,%a0
	move.w   %d1,%d0
__gray_cpy_d_plane:
	move.l   (%a0)+,(%a1)+
	dbf      %d0, __gray_cpy_d_plane
	lea      (__gray_int1_handler_hw2,%pc),%a0

	bra.s    __gray_init_replace_vector
__gray_init_hw1_handler:
    |--------------------------------------------------------------------------
    | HW1 specific initializations:
    |--------------------------------------------------------------------------
	move.l   (%a0),%a1
	lea      (__gray_int1_handler_hw1,%pc),%a0
    |--------------------------------------------------------------------------
    | Install our own INT1 handler
    |--------------------------------------------------------------------------
__gray_init_replace_vector:
	move.l   %a0,%d2
	lea      0x600001,%a0
	moveq.l  #2,%d0
	bclr.b   %d0,(%a0)
	move.l   %d2,0x64.w
	bset.b   %d0,(%a0)
    |--------------------------------------------------------------------------
    | clear light plane (done for both HW types)
    |--------------------------------------------------------------------------
__gray_clr_l_plane:
	clr.l    (%a1)+
	dbf      %d1, __gray_clr_l_plane
    |--------------------------------------------------------------------------
    | PortSet(__gray_D_plane,239,127)
    |--------------------------------------------------------------------------
	move.l   0xc8.w,%a0
	move.l   #0xEF007F,-(%sp)
	move.l   (__gray_D_plane,%pc),-(%sp)
	movea.l  (0x1A2*4,%a0),%a1 /* PortSet */
	jsr      (%a1)
	addq.l   #8,%sp
__gray_ok:
	lea      (__gray_L_plane,%pc),%a0
	lea      (__gray_L_plane2,%pc),%a1
	move.l   (%a0)+,(%a1)+        | copy __gray_L_plane to __gray_L_plane2
	move.l   (%a0)+,(%a1)+        | copy __gray_D_plane to __gray_D_plane2
__gray_init_return_1:
	moveq    #0x1,%d0
__gray_init_rts:
	rts

|==============================================================================
|  EXPORTED: GrayOff function (turn grayscales off)
|            NOTE: ALWAYS returns 1 !!
|==============================================================================
GrayOff:
	lea      (__gray_handle,%pc),%a0
	move.w   (%a0),%d0
	beq.s    __gray_init_return_1           | no handle? -> nothing to do
	move.w   %d0,-(%sp)			| push handle here so we don't
	                                        | have to remember its address
	clr.l    (%a0)				| 0->handle AND(!!) 0->__gray_dbl_offset
	lea      0x600001,%a0			| address of memory mapped IO port

    |--------------------------------------------------------------------------
    | cleanup for HW1 calcs (applied to HW2+ calcs as well, but it's a no-op)
    |--------------------------------------------------------------------------
	move.w   #0x980,(0x600010-0x600001,%a0)	| restore used plane to 0x4c00
__gray_restore_old_int1:
	moveq    #2,%d0
	bclr.b   %d0,(%a0)
	move.l   (__gray_old_int1_handler,%pc),0x64.w			| restore old INT1 handler
	bset.b   %d0,(%a0)

    |--------------------------------------------------------------------------
    | copy __gray_D_plane contents to LCD_MEM
    |--------------------------------------------------------------------------
	movea.l  (__gray_D_plane,%pc),%a1
	lea      0x4C00.w,%a0			| LCD_MEM
	move.w   #0x3BF,%d0			| LCD_SIZE/4-1
__gray_dark2lcd:
	move.l   (%a1)+,(%a0)+
	dbf      %d0, __gray_dark2lcd

	lea      (__gray_L_plane,%pc),%a0		| restore plane pointers to 0x4c00
	clr.l    (__gray_sync_n_count - __gray_L_plane, %a0)
	lea      0x4C00.w,%a1
	move.l   %a1,(%a0)+			|__gray_L_plane
	move.l   %a1,(%a0)+			|__gray_D_plane
	move.l   %a1,(%a0)			|__gray_used_mem
    |--------------------------------------------------------------------------
    | HeapFree(__gray_handle)
    |--------------------------------------------------------------------------
	movea.l  0xc8.w,%a0
	movea.l  (0x97*4,%a0),%a0 /* HeapFree */
	jsr      (%a0)
	addq.l   #2,%sp
    |--------------------------------------------------------------------------
    | PortRestore()
    |--------------------------------------------------------------------------
	movea.l  0xc8.w,%a0
	movea.l  (0x1A3*4,%a0),%a0 /* PortRestore */
	jsr      (%a0)
__gray_off_out:
	bra.s    __gray_ok

__gray_hw_type:    | stores HW type (0==HW1 or VTI, 1==HW2)
	.word 0

|==============================================================================
| Interrupt 1 handler for HW1
|==============================================================================
__gray_int1_handler_hw1:
	move.l  %a0,-(%sp)
    |--------------------------------------------------------------------------
    | Load skip counter and increment it (count = (count+1)&0x3). Skip any
    | further operation if count is 1, 2 or 3. This means that every 4th call
    | of the INT1 handler is a candidate for a plane switch
    |--------------------------------------------------------------------------
	lea      (__gray_skipcount,%pc),%a0
	addq.b   #1,(%a0)
	andi.b   #0x3,(%a0)+            | IMPORTANT: a0 points now to __gray_phase!
	bne.s    __gray_proceed_old2
	move.l  %d0,-(%sp)
    |--------------------------------------------------------------------------
    | to evaluate which plane we use counter __gray_phase. This counter
    | performs the following counting 8->4->0->8.
    | 0 will use D_plane pointer
    | 4 will use L_plane pointer
    | (8 will be skipped, so it stays at D_plane)
    |--------------------------------------------------------------------------
	move.w   (%a0),%d0
	subq.w   #4,%d0                  | subtract 4 from phase counter
	bcc.s    __gray_store            | not negative -> don't reset
	moveq    #0x8,%d0                | reset phase counter to 8
__gray_store:
	move.w   %d0,(%a0)               | store new phase counter value
	cmp.b    #8,%d0
	beq.s    __gray_proceed_old      | for value 8 we do nothing (dark plane
	                                 | stays active)
	lea      (__gray_D_plane,%pc),%a0
    |--------------------------------------------------------------------------
    | doublebuffer extension ... add content of __gray_dbl_offset to %d0
    |--------------------------------------------------------------------------
	add.w    (__gray_dbl_offset-__gray_D_plane,%a0),%d0
	suba.w   %d0,%a0
	move.l   (%a0),%d0               | load the address of this plane
	lsr.l    #3,%d0                  | reduce to address / 8
	move.w   %d0,0x600010            | set new plane startaddress
	lea      (__gray_switch_cnt,%pc),%a0  | increment switch count
	addq.l   #1,(%a0)
__gray_proceed_old:
	move.l  (%sp)+,%d0
__gray_proceed_old2:
	move.l  (%sp)+,%a0
    |--------------------------------------------------------------------------
    |  JUMP to previous installed interrupt handler
    |--------------------------------------------------------------------------
	bra     __gray_old_int1_handler - 2
__gray_dummy1:                           | NOT used yet (just for alignment)
	.byte    0x00
|------------------------------------------------------------------------------
| __gray_skipcount is a one byte counter which performs the following counting:
| 3 -> 0 -> 1 -> 2 -> 3
|------------------------------------------------------------------------------
__gray_skipcount:
	.byte    0x03
__gray_phase:
	.word    0x04                    | performs: 4->0->8->4
__switch_cnt:
__gray_switch_cnt:
	.long    0x00000000

|------------------------------------------------------------------------------
| handle to allocated memory used by grayscale
|------------------------------------------------------------------------------
__gray_handle:
	.word    0

|------------------------------------------------------------------------------
| DOUBLEBUFFER extension
|------------------------------------------------------------------------------
__gray_dbl_offset: | has to be directly AFTER __gray_handle!!
	.word    0
__L_plane2:
__gray_L_plane2:
	.long    0x0
__D_plane2:
__gray_D_plane2:
	.long    0x0

|------------------------------------------------------------------------------
| pointer to light plane
| HW1: same as __gray_used_mem
| HW2: __gray_used_mem + 0xf00
|------------------------------------------------------------------------------
__L_plane:
__gray_L_plane:
	.long    0x00004c00
|------------------------------------------------------------------------------
| pointer to dark plane (set by __gray_init_handler)
| HW1: 0x4c00
| HW2: same as __gray_used_mem
|------------------------------------------------------------------------------
__D_plane:
__gray_D_plane:
	.long    0x00004c00
|------------------------------------------------------------------------------
| pointer to allocated memory ALIGNED to 8-byte boundary
|------------------------------------------------------------------------------
__gray_used_mem:
	.long    0x00004c00
|------------------------------------------------------------------------------
| This variable is very hard to describe. Indeed this isn't one variable,
| but two variables combined in one.
|
| Description will be added later ....
|------------------------------------------------------------------------------
__gray_sync_n_count:
	.word    0x0000
|------------------------------------------------------------------------------
| holds the index of the plane which should be drawn next (NOTE: this label
| is never addressed directly, but indirectly from label __gray_sync_n_count.
| So don't move it to anywhere else!)
|------------------------------------------------------------------------------
__gray_plane_index:
	.word    0x0000
|==============================================================================
| Interrupt 1 handler for HW2
|
| port 70001D (bit 7) is used to synchronized to the LCD hardware. Here are the
| docs of this port (taken from Johan Eilert's j89hw.txt)
|
| $70001D RW ($06)
|	:7	 Toggles every FS (every time the LCD restarts at line 0)
|	:6-4	-
|	:3	 Battery checker bit B (? ???)
|	:2	 ? (set)
|	:1	 Screen enable (clear this bit to shut down LCD)
|	:0	 Battery checker bit A (? enable $600000:2)
|	     (AMS:) The battery checker bits must both be set (AB=11) prior to
|	     checking the voltage level with $600000:2.  Then, after use, bit B
|	     must be cleared (AB=10) while the battery trig hardware settles to
|	     the "rest" voltage value (%111).  Finally, both bits should be
|	     cleared.
|==============================================================================
__gray_int1_handler_hw2:
	move.w   %sr,-(%sp)                | save content of status register on stack
	move.w   #0x2700,%sr               | disable ALL interrupts (no one should
	                                   | interrupt us ...)
	movem.l  %d0-%d7/%a0-%a6,-(%sp)
__gray_startagain:
	moveq    #0x0,%d1
	lea      (__gray_sync_n_count,%pc),%a0
	move.w   (%a0),%d0
	bne.s    __gray_copy_first_or_sec  | there is a third of the plane left to
	                                   | copy -> do it now!
	move.l   (%a0),%d0
.ifdef ALWAYS_HW2_TESTING
	move.b   %d0,%d1
	eor.b    #0x80,%d1
.else
	move.b   0x70001D,%d1              | get flipping bit
	eor.b    %d0,%d1
	bpl      __gray_to_oldint          | not flipped yet -> proceed to previous
	                                   | installed int handler
.endif
	eor.l    %d1,(%a0)                 | store new flip "bit" and reset the
	                                   | work left status

    |--------------------------------------------------------------------------
    | NOTE: if we detect a pageflip we start our copying work with the lowest
    |       third. this way it will not interfere with the LCD hardware refresh
    |
    | The 3 thirds are copied in the following order:
    |
    | last third -> first third -> second third
    |--------------------------------------------------------------------------
	move.w   #0xA00,%d0
__gray_copy_next_third:
	addq.w   #1,(%a0)+
	bra.s    __gray_perform_copying
__gray_copy_first_or_sec:
    |--------------------------------------------------------------------------
    | if __gray_sync_n_count == 1 -> copy first third of screen
    | otherwise -> set __gray_sync_n_count to 0 and copy second third of screen
    |--------------------------------------------------------------------------
	subq.w   #1,%d0
	beq.s    __gray_copy_next_third
	clr.w    (%a0)+
	move.w   #0x500,%d0             | setup to copy second third of screen
__gray_perform_copying:
	move.b   (%a0),%d1              | fetch index of plane to draw next
	beq      __gray_update_index    | skip index 0 -> stay at darkplane

    |--------------------------------------------------------------------------
    | If we'll come here we will copy 1 third of the screen from a specific
    | plane to the video buffer at 0x4c00. Register D0 holds the offset of
    | which third should be copied and register D1 contains the "index" of the
    | "source" plane
    |
    | index = 0 -> darkplane (skipped, will not come here!)
    | index = 4 -> darkplane
    | index = 8 -> lightplane
    |
    | Due to the fact that the indices are cycled from 8 down to 0 the skipped
    | index 0 causes the darkplane to stay active twice as long as the light
    | plane.
    |
    | The copying is performed in a kind of "hardcore" style by using 14
    | registers. This way 56 bytes are copied with a pair of instructions.
    |--------------------------------------------------------------------------

    |--------------------------------------------------------------------------
    | doublebuffer extension ... add content of __gray_dbl_offset to %d0
    |--------------------------------------------------------------------------
	lea      (__gray_dbl_offset,%pc),%a1
	add.w    (%a1),%d1
	neg.w    %d1
	movea.l  (__gray_used_mem,%pc,%d1.w),%a0
	move.l   %sp,(__gray_save_sp - __gray_dbl_offset,%a1)

	lea      0x4C00.w,%a1
	adda.w   %d0,%a0
	adda.w   %d0,%a1
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*1,%a1)  | size of this instruction: 6 bytes
	movem.l  (%a0)+,%d0-%d7/%a2-%a7      | size of this instruction: 4 bytes
	movem.l  %d0-%d7/%a2-%a7,(56*2,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*3,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*4,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*5,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*6,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*7,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*8,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*9,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*10,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*11,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*12,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*13,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*14,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*15,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*16,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*17,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*18,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*19,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*20,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a7
	movem.l  %d0-%d7/%a2-%a7,(56*21,%a1)
	movem.l  (%a0)+,%d0-%d7/%a2-%a5      | Remainder: 48 bytes
	movem.l  %d0-%d7/%a2-%a5,(56*22,%a1)
	move.l   __gray_save_sp(%pc),%sp

    |--------------------------------------------------------------------------
    | evaluate if there is still a third of the screen to copy or if we
    | should proceed to the next plane
    |--------------------------------------------------------------------------
__gray_update_index:
	lea      (__gray_sync_n_count,%pc),%a0
	move.w   (%a0)+,%d0
	beq      __gray_startagain          | no third left to copy -> check again
	                                    | the pageflip bit if yet a pageflip
	                                    | had occured
	subq.w   #1,%d0
	bne.s    __gray_to_oldint           | if there is "copy work" left ->
	                                    | don't modify the plane to display

    | increment switch count here, because a complete page was drawn if we come here
	addq.l   #1,(__gray_switch_cnt - __gray_sync_n_count - 2, %a0)

	subq.b   #4,(%a0)                   | cycle __gray_plane_index by decrementing
	bcc.s    __gray_to_oldint           | it and wrap around to 8 if negative.
	move.b   #0x8,(%a0)
__gray_to_oldint:
	movem.l  (%sp)+,%d0-%d7/%a0-%a6
	move.w   (%sp)+,%sr                 | restore content of status register
    |--------------------------------------------------------------------------
    |  JUMP to previous installed interrupt handler
    |--------------------------------------------------------------------------
	.word    0x4ef9                     | opcode of "JMP address" instruction
__gray_old_int1_hw1:
__gray_old_int1_hw2:
__gray_old_int1_handler:
	.long    0x00000000
__gray_save_sp:
	.long    0x00000000


| #############################################################################
|  Revision History
| #############################################################################
|
| Revision 3.18 2016/04 Lionel Debroux
| Bumped to version 3.57
| Saved two additional bytes in GrayOff.
|
| Revision 3.18 2014/01 Lionel Debroux
| Bumped to version 3.56
| Saved 46 additional bytes by:
| * inlining internal (non-exported) __gray_check_hw_version, __gray_init_mem,
|   __gray_init_handler subroutines into their sole caller, GrayOn().
| * using smarter SMC for setting __gray_size_to_allocate and
|   __gray_size_to_add;
| * reordering code in order to use several shorter branches;
| * using 56-byte movem instead of 52-byte movem, like in Grib (-6 bytes,
|   marginal speedup).
| Some of these changes were pioneered in the ExtGraph version.
| Add ALLOCATE_LESS_ON_HW1 knob.
|
| Revision 3.17 2006/10/30 07:24:15  Kevin Kofler
| Bumped version to 3.55.
| Size optimization by Martial Demolins (use moveq instead of bclr/bset #imm).
|
| Revision 3.16 2005/10/09 01:48:20  Kevin Kofler
| Bumped version to 3.54.
| Size optimization by Jesse Frey (register saving, ROM_CALL optimization),
| spelling/typo fixes by Kevin Kofler.
|
| Revision 3.15 2005/08/22 20:23:40  Kevin Kofler
| Bumped version to 3.53.
| Fixed calls to GrayOn with grayscale already enabled.
|
| Revision 3.14 2005/07/02 02:56:36  Kevin Kofler
| Bumped version to 3.52.
|
| Revision 3.13 2005/06/11 05:58:27  Kevin Kofler
| Removed commented-out junk.
| Remove Id and Log tags.
|
| Revision 3.12 2005/06/07 16:51:00  Lionel Debroux
| Optimized the routine for size: saved 40 bytes.
|
| Revision 3.11 2004/02/25 03:49:03  Kevin Kofler
| Don't use 0x40000 to set interrupts on code path that affect HW3.
| Use 0xE00000 as ROM_base mask instead of 0x600000.
| Bumped version to 3.51.
|
| Revision 3.10 2002/04/05 11:34:23  tnussb
| (1) Resets now __D_plane2,__L_plane2 and __gray_dbl_offset at end of GrayOn()
|     to make sure nothing happens if doublebuffer macros get called without
|     setting a buffer first. Nevertheless NO program should call one of the
|     doublebuffer macros without setting a buffer previously.
| (2) Some further size optimizations. Now the size is exactly as long as
|     without before the integration of doublebuffering. Quite smart, isn't it?
|     More functionality, some minor drawbacks fixed and no increase in size ...
| (3) Changed return value of GrayOff() function to void.
|
| Revision 3.9  2002/04/04 18:50:36  tnussb
| first working version of internal doublebuffer support (not checked on
| real calcs yet, but on the VTI)
|
| Revision 3.8  2002/04/04 16:39:05  tnussb
| Debug version checked in previously. This one is the correct one.
|
| Revision 3.7  2002/04/04 16:35:20  tnussb
| (1) documentation of HW2 interrupt handler heavily extended
| (2) HW2: plane switch counting fixed (was incremented too often previously)
| (3) global ALWAYS_HW2_TESTING added which can be used to force the use of
|     HW2 grayscales. Now the HW2 grayscales can be tested on the VTI if
|     this global is defined. Of course this will flicker extremely, because
|     the VTI doesn't simulate port 70001D, but its still better than no
|     testing possibility.
| (4) don't trashes %sr on HW2 anymore (restores previous setting)
| (5) This version was tested on a real HW1 and a real HW2 calc. It works.
|     (Thanx to Sebastian Reichelt and stoopid guy)
|
| Revision 3.6  2002/04/04 11:58:19  tnussb
| (1) size optimizations
| (2) unnecessary cleanup removed from GrayOff()
| (3) interrupt handler for HW1 rewritten (uses now plane pointer directly)
| (4) "heavily" documented
|
| Revision 3.5  2002/04/04 11:54:39  tnussb
| (1) exports __gray_old_int1_hw1 and __gray_old_int1_hw2. this way it is
|     possible to modify them after grayscales are turned on
| (2) comments changed to GNU Assembler style
| [NOTE: CVS time and date doesn't fit to real implementation data]
|
| Revision 3.0  2002/04/04 11:50:56  tnussb
| grayscale support used for TIGCC up to version v0.93
| [NOTE: CVS time and date doesn't fit to real implementation data]
|
