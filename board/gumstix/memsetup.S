#include <config.h>
#include <version.h>
#include <asm/arch/pxa-regs.h>


.macro CPWAIT
	mrc	 p15,0,r0,c2,c0,0
	mov	 r0,r0
	sub	 pc,pc,#4
.endm


.globl lowlevel_init
lowlevel_init:


	/* Set up GPIO pins first */

	ldr		r0,	=GPSR0
	ldr		r1,	=CFG_GPSR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPSR1
	ldr		r1,	=CFG_GPSR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPSR2
	ldr		r1,	=CFG_GPSR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GPSR3
	ldr		r1,	=CFG_GPSR3_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR0
	ldr		r1,	=CFG_GPCR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR1
	ldr		r1,	=CFG_GPCR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR2
	ldr		r1,	=CFG_GPCR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR3
	ldr		r1,	=CFG_GPCR3_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR0
	ldr		r1,	=CFG_GPDR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR1
	ldr		r1,	=CFG_GPDR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR2
	ldr		r1,	=CFG_GPDR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR3
	ldr		r1,	=CFG_GPDR3_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR0_L
	ldr		r1,	=CFG_GAFR0_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR0_U
	ldr		r1,	=CFG_GAFR0_U_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR1_L
	ldr		r1,	=CFG_GAFR1_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR1_U
	ldr		r1,	=CFG_GAFR1_U_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR2_L
	ldr		r1,	=CFG_GAFR2_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR2_U
	ldr		r1,	=CFG_GAFR2_U_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR3_L
	ldr		r1,	=CFG_GAFR3_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR3_U
	ldr		r1,	=CFG_GAFR3_U_VAL
	str		r1,   [r0]

	ldr		r0,	=PSSR
	ldr		r1,	=CFG_PSSR_VAL
	str		r1,	[r0]

	cmp	pc, #0xa0000000			/** test if we're in SDRAM		**/
	bhi	end_of_memsetup			/** if we are, then jump		**/
	
	/* Clock initialization */
clock_setup:

	/* Disable the peripheral clocks, and set the core clock frequency */

	/* Write the preferred values for L, N, PPDIS, and CPDIS to CCCR */
	ldr	r1, =CCCR
	ldr	r2, =CCCR_VAL
	str	r2, [r1]

	/* Set CLKCFG[F] and turn on turbo mode */
	mrc	p14, 0, r2, c6, c0, 0
	orr	r2, r2, #0x3	/* Turbo, freq. change */
	mcr	p14, 0, r2, c6, c0, 0

	/* Re-read CCCR then check cp14,6 until it says it's set */
	ldr	r0, [r1]
wait_for_speed_change:
	mrc	p14, 0, r0, c6, c0, 0
	cmp	r2, r0
	bne	wait_for_speed_change

	/* No SDRAM controller refresh needed, since it's not initialized yet! */

memory_setup:
	/* Write MSC0, MSC1, MSC2 */
	ldr	r3, =MSC0		/* Configures /CS0 and /CS1 */
	ldr	r2, =0x033C35D8		/* From P30-PXA270 design guide */
	str	r2, [r3]
	ldr	r2, [r3]		/* When programming a different memory type in an MSC register, ensure that the new value has 
					 * been accepted and programmed before issuing a command to that memory. To do this, read the 
					 * MSC register before accessing the memory. */

	/* Write MECR, MCMEM0, MCMEM1, MCATT0, MCATT1, MCIO0, MCIO1 */
	ldr	r3, =MEMC_BASE

	/* MECR: Memory Expansion Card Register                             */
	ldr     r2, =CFG_MECR_VAL
	str     r2, [r3, #MECR_OFFSET]
	ldr	r2, [r3, #MECR_OFFSET]

	/* MCMEM0: Card Interface slot 0 timing                             */
	ldr     r2, =CFG_MCMEM0_VAL
	str     r2, [r3, #MCMEM0_OFFSET]
	ldr	r2, [r3, #MCMEM0_OFFSET]

	/* MCMEM1: Card Interface slot 1 timing                             */
	ldr     r2, =CFG_MCMEM1_VAL
	str     r2, [r3, #MCMEM1_OFFSET]
	ldr	r2, [r3, #MCMEM1_OFFSET]

	/* MCATT0: Card Interface Attribute Space Timing, slot 0            */
	ldr     r2, =CFG_MCATT0_VAL
	str     r2, [r3, #MCATT0_OFFSET]
	ldr	r2, [r3, #MCATT0_OFFSET]

	/* MCATT1: Card Interface Attribute Space Timing, slot 1            */
	ldr     r2, =CFG_MCATT1_VAL
	str     r2, [r3, #MCATT1_OFFSET]
	ldr	r2, [r3, #MCATT1_OFFSET]

	/* MCIO0: Card Interface I/O Space Timing, slot 0                   */
	ldr     r2, =CFG_MCIO0_VAL
	str     r2, [r3, #MCIO0_OFFSET]
	ldr	r2, [r3, #MCIO0_OFFSET]

	/* MCIO1: Card Interface I/O Space Timing, slot 1                   */
	ldr     r2, =CFG_MCIO1_VAL
	str     r2, [r3, #MCIO1_OFFSET]
	ldr	r2, [r3, #MCIO1_OFFSET]


	/* FLYCNFG (skip on gumstix) */

	/* Reset the system appropriately. Configure, but do not enable, each SDRAM partition pair 
	 * by clearing the enable bits MDCNFG[DEx] when writing to the MDCNFG register. */

	ldr	r0, =MDCNFG_VAL_13_10	/* Load the value for MDCNFG */
memory_timing_setup:

	ldr	r3, =MDCNFG		/* Load the SDRAM Configuration register. Must not be enabled yet. */
	str	r0, [r3]		/* Write to MDCNFG register */
	ldr	r0, [r3]

	/* Set MDREFR[K0RUN]. Properly configure MDREFR[K0DB2] and MDREFR[K0DB4]. 
	 * Retain the current values of MDREFR[APD] (clear) and MDREFR[SLFRSH] (set). 
	 * MDREFR[DRI] must contain a valid value (not all 0s). If required, MDREFR[KxFREE] 
	 * can be de-asserted. */

	ldr	r3, =MDREFR
	ldr	r2, [r3]		/* read MDREFR value */

	ldr	r1, =0xfff
	bic	r2, r2, r1
	orr	r2, r2, #0x001		/* configure a valid SDRAM Refresh Interval (DRI) */

	/* SDCLK0 n/c *
	/* SDCLK1 goes to SDRAM */
	bic	r2, r2, #(MDREFR_K2FREE | MDREFR_K1FREE | MDREFR_K0FREE)	/* Clear free run */
	str	r2, [r3]

	/* In systems that contain synchronous flash memory, write to the SXCNFG to configure all 
	 * appropriate bits, including the enables. While the synchronous flash banks are being 
	 * configured, the SDRAM banks must be disabled and MDREFR[APD] must be de-asserted 
	 * (auto-power-down disabled). (skip on gumstix)*/

	/*  In systems that contain SDRAM, toggle the SDRAM controller through the following state 
	 * sequence: self-refresh and clock-stop to self-refresh to power-down to PWRDNX to NOP. */
	orr	r2, r2, #(MDREFR_K1RUN|MDREFR_K2RUN)	/* assert K1RUN (and K2RUN) */
	bic	r2, r2, #(MDREFR_K1DB2|MDREFR_K2DB2)	/* clear K1DB2 (and K2DB2) */
	str	r2, [r3]		/* change from "self-refresh and clock-stop" to "self-refresh" state */

	bic	r2, r2, #MDREFR_SLFRSH	/* clear SLFRSH bit field */
	str	r2, [r3]		/* change from "self-refresh" to "Power-down" state */

	orr	r2, r2, #MDREFR_E1PIN	/* set the E1PIN bit field */
	str	r2, [r3]		/* change from "Power-down" to "PWRDNX" state */

	nop	/* no action is required to change from "PWRDNX" to "NOP" state */

	/* Appropriately configure, but do not enable, each SDRAM partition pair. 
	 * SDRAM partitions are disabled by keeping the MDCNFG[DEx] bits clear.
	 * (note: already done, but manuall repeats the instruction, so what do I know?) */
	ldr	r3, =MDCNFG		/* Load the SDRAM Configuration register. Must not be enabled yet. */
	str	r0, [r3]		/* Write to MDCNFG register */
	ldr	r0, [r3]


	/* For systems that contain SDRAM, wait the NOP power-up waiting period required by the 
	 * SDRAMs (normally 100-200 Î¼sec) to ensure the SDRAMs receive a stable clock with a NOP 
	 * condition. */

	ldr	r3, =OSCR		/* reset the OS Timer Count to zero */
	mov	r2, #0 
	str	r2, [r3] 
	ldr	r4, =0x300		/* really 0x28a is about 200usec, so 0x300 should be plenty */
20:
	ldr	r2, [r3] 
	cmp	r4, r2
	bgt	20b 

	/* Ensure the XScale core memory-management data cache (Coprocessor 15, Register 1, bit 2) is 
	 * disabled. If this bit is enabled, the refreshes triggered by the next step may not be passed 
	 * properly through to the memory controller. Coprocessor 15, register 1, bit 2 must be re- 
	 * enabled after the refreshes are performed if data cache is preferred. */

	mrc	p15, 0, r0, c1, c0, 0	/* Read the register */
	bic	r0, #0x4		/* turn data cache off */
	mcr	p15, 0, r0, c1, c0, 0

	CPWAIT				/* wait for co-processor */

	/* On hardware reset in systems that contain SDRAM, trigger a number (the number required by 
	 * the SDRAM manufacturer) of refresh cycles by attempting non-burst read or write accesses to
	 * any disabled SDRAM bank. Each such access causes a simultaneous CBR for all four banks, 
	 * which in turn causes a pass through the CBR state and a return to NOP. On the first pass, the 
	 * PALL state is incurred before the CBR state. */

	ldr	r3, =CFG_DRAM_BASE
	mov	r2, #2	/* now must do 2 or more refresh or CBR commands before the first access */
CBR_refresh1:
	str	r2, [r3]
	subs	r2, r2, #1 
	bne	CBR_refresh1 

	/* Can re-enable DCACHE if it was disabled above (skip on gumstix) */

	/* In systems that contain SDRAM, enable SDRAM partitions by setting MDCNFG[DEx] bits. */

	ldr	r3, =MDCNFG		/* sdram config -- sdram enable */
	ldr	r2, [r3] 
	orr	r2, r2, #(MDCNFG_DE0|MDCNFG_DE1)	/* enable partitions 0,1 */
	orr	r2, r2, #(MDCNFG_DE2|MDCNFG_DE3)	/* enable partitions 2,3 */
	str	r2, [r3]		/* write to MDCNFG */

	/*  In systems that contain SDRAM, write the MDMRS register to trigger an MRS command to 
	 * all enabled banks of SDRAM. For each SDRAM partition pair that has one or both partitions 
	 * enabled, this forces a pass through the MRS state and a return to NOP. The CAS latency is the 
	 * only variable option and is derived from what was programmed into the MDCNFG[MDTC0] 
	 * and MDCNFG[MDTC2] fields. The burst type and length are always programmed to 
	 * sequential and four, respectively. */

	ldr	r3, =MDMRS		/* write the MDMRS */
	ldr	r2, =0x00320032		/* CAS latency = 3 */
	str	r2, [r3]

	/*  In systems that contain SDRAM or synchronous flash, optionally enable auto-power-down by 
	 * setting MDREFR[APD]. */

	ldr	r3, =MDREFR		/* enable auto-power-down */
	ldr	r2, [r3] 
	orr	r2, r2, #MDREFR_APD	/* set the APD bit */
	str	r2, [r3]		/* write to MDREFR */


	/* Now we check to make sure we got the column/row addressing right by checking for mirrors
	 * in low SDRAM.  If we find a mirror, then the location of the mirror will clue us to
	 * what the alignment should actually be.  We start off configuring for 13x10 then scale down */
	mov	r0, #0x00000800
	mov	r1, #CFG_DRAM_BASE
	str	r1, [r1]		/* Write the address to base of RAM */
	str	r0, [r1, r0]		/* Write the offset to the location */
	ldr	r1, [r1]		/* Read back base of RAM */
	cmp	r0, r1			/* See if we found a mirror */
	bne	end_of_memsetup

	/* If we get here, we found a mirror, so restart RAM with different settings and try again */
	ldr	r0, =MDCNFG_VAL_13_9
	b	memory_timing_setup

end_of_memsetup:

	/* Set voltage here if necessary */
//setvoltage:
//
//	mov	r10, lr
//	bl	initPXAvoltage		/* In case the board is rebooting with a    */
//	mov	lr, r10			/* low voltage raise it up to a good one.   */


wakeup:
	/* Are we waking from sleep? */
	ldr	r0, =RCSR
	ldr	r1, [r0]
	and	r1, r1, #(RCSR_GPR | RCSR_SMR | RCSR_WDR | RCSR_HWR)
	str	r1, [r0]
	teq	r1, #RCSR_SMR

	bne	initirqs

	ldr	r0, =PSSR
	mov	r1, #PSSR_PH
	str	r1, [r0]

	/* if so, resume at PSPR */
	ldr	r0, =PSPR
	ldr	r1, [r0]
	mov	pc, r1

	/* Disable (mask) all interrupts at interrupt controller */
initirqs:

	mov	r1, #0

	mcr	p6, 0, r1, c2, c0, 0
	//ldr	r2, =ICLR
	//str	r1, [r2]
	mcr	p6, 0, r1, c8, c0, 0
	//ldr	r2, =ICLR2
	//str	r1, [r2]

	mcr	p6, 0, r1, c1, c0, 0
	//ldr	r2, =ICMR
	//str	r1, [r2]
	mcr	p6, 0, r1, c7, c0, 0
	//ldr	r2, =ICMR2
	//str	r1, [r2]

	/* Re-write MDREFR */
	ldr	r3, =MDREFR
	ldr	r2, [r3]
	str	r2, [r3]

	/* enable the 32Khz oscillator for RTC and PowerManager             */

	ldr     r1, =OSCC
	ldr	r2, [r1]
	orr     r2, r2, #OSCC_OON
	str     r2, [r1]

	/* Now also set up OS timer channel 10 to tick at 32KHz  for CHOUT<0> */

	ldr	r1, =OMCR10
	mov	r2, #(OMCR_C | OMCR_P | OMCR_R | 1)	/* Match channel 10, start on write to OSCR, continue after match, reset on match, tick at 32KHz */
	str	r2, [r1]
	ldr	r1, =OSMR10
	mov	r2, #1
	str	r2, [r1]
	ldr	r1, =OSCR10	/* Write to OSCR10 to start the counter */
	str	r2, [r1]

	/* NOTE:  spin here until OSCC.OOK get set, meaning the PLL         */
	/* has settled.                                                     */
#if 0
	ldr	r1, =OSCC
60:
	ldr	r2, [r1]
	tst	r2, #OSCC_OOK
	beq	60b
#endif

	/* Exercise ethernet reset line - nRESET was cleared when GPIO set up up above */

	ldr	r3, =OSCR		/* reset the OS Timer Count to zero */
	mov	r2, #0 
	str	r2, [r3] 
	ldr	r4, =0x300		/* really 0x28a is about 200usec, so 0x300 should be plenty */
70:
	ldr	r2, [r3] 
	cmp	r4, r2
	bgt	70b 

	ldr		r0,	=GPSR3
	ldr		r1,	=0x00000800
	str		r1,	[r0]



	mov	pc, lr

