/*
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * CPU specific code for the MPC83xx family.
 *
 * Derived from the MPC8260 and MPC85xx.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc83xx.h>
#include <ft_build.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;


int checkcpu(void)
{
	volatile immap_t *immr;
	ulong clock = gd->cpu_clk;
	u32 pvr = get_pvr();
	u32 spridr;
	char buf[32];

	immr = (immap_t *)CFG_IMMR;

	if ((pvr & 0xFFFF0000) != PVR_83xx) {
		puts("Not MPC83xx Family!!!\n");
		return -1;
	}

	spridr = immr->sysconf.spridr;
	puts("CPU: ");
	switch(spridr) {
	case SPR_8349E_REV10:
	case SPR_8349E_REV11:
		puts("MPC8349E, ");
		break;
	case SPR_8349_REV10:
	case SPR_8349_REV11:
		puts("MPC8349, ");
		break;
	case SPR_8347E_REV10_TBGA:
	case SPR_8347E_REV11_TBGA:
	case SPR_8347E_REV10_PBGA:
	case SPR_8347E_REV11_PBGA:
		puts("MPC8347E, ");
		break;
	case SPR_8347_REV10_TBGA:
	case SPR_8347_REV11_TBGA:
	case SPR_8347_REV10_PBGA:
	case SPR_8347_REV11_PBGA:
		puts("MPC8347, ");
		break;
	case SPR_8343E_REV10:
	case SPR_8343E_REV11:
		puts("MPC8343E, ");
		break;
	case SPR_8343_REV10:
	case SPR_8343_REV11:
		puts("MPC8343, ");
		break;
	case SPR_8360E_REV10:
	case SPR_8360E_REV11:
	case SPR_8360E_REV12:
		puts("MPC8360E, ");
		break;
	case SPR_8360_REV10:
	case SPR_8360_REV11:
	case SPR_8360_REV12:
		puts("MPC8360, ");
		break;
	default:
		puts("Rev: Unknown\n");
		return -1;	/* Not sure what this is */
	}

#if defined(CONFIG_MPC8349)
	printf("Rev: %02x at %s MHz\n", (spridr & 0x0000FFFF)>>4 |(spridr & 0x0000000F), strmhz(buf, clock));
#else
	printf("Rev: %02x at %s MHz\n", spridr & 0x0000FFFF, strmhz(buf, clock));
#endif
	return 0;
}


/*
 * Program a UPM with the code supplied in the table.
 *
 * The 'dummy' variable is used to increment the MAD. 'dummy' is
 * supposed to be a pointer to the memory of the device being
 * programmed by the UPM.  The data in the MDR is written into
 * memory and the MAD is incremented every time there's a read
 * from 'dummy'. Unfortunately, the current prototype for this
 * function doesn't allow for passing the address of this
 * device, and changing the prototype will break a number lots
 * of other code, so we need to use a round-about way of finding
 * the value for 'dummy'.
 *
 * The value can be extracted from the base address bits of the
 * Base Register (BR) associated with the specific UPM.  To find
 * that BR, we need to scan all 8 BRs until we find the one that
 * has its MSEL bits matching the UPM we want.  Once we know the
 * right BR, we can extract the base address bits from it.
 *
 * The MxMR and the BR and OR of the chosen bank should all be
 * configured before calling this function.
 *
 * Parameters:
 * upm: 0=UPMA, 1=UPMB, 2=UPMC
 * table: Pointer to an array of values to program
 * size: Number of elements in the array.  Must be 64 or less.
 */
void upmconfig (uint upm, uint *table, uint size)
{
#if defined(CONFIG_MPC834X)
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile lbus83xx_t *lbus = &immap->lbus;
	volatile uchar *dummy = NULL;
	const u32 msel = (upm + 4) << BR_MSEL_SHIFT;	/* What the MSEL field in BRn should be */
	volatile u32 *mxmr = &lbus->mamr + upm;	/* Pointer to mamr, mbmr, or mcmr */
	uint i;

	/* Scan all the banks to determine the base address of the device */
	for (i = 0; i < 8; i++) {
		if ((lbus->bank[i].br & BR_MSEL) == msel) {
			dummy = (uchar *) (lbus->bank[i].br & BR_BA);
			break;
		}
	}

	if (!dummy) {
		printf("Error: %s() could not find matching BR\n", __FUNCTION__);
		hang();
	}

	/* Set the OP field in the MxMR to "write" and the MAD field to 000000 */
	*mxmr = (*mxmr & 0xCFFFFFC0) | 0x10000000;

	for (i = 0; i < size; i++) {
		lbus->mdr = table[i];
		__asm__ __volatile__ ("sync");
		*dummy;	/* Write the value to memory and increment MAD */
		__asm__ __volatile__ ("sync");
	}

	/* Set the OP field in the MxMR to "normal" and the MAD field to 000000 */
	*mxmr &= 0xCFFFFFC0;
#else
	printf("Error: %s() not defined for this configuration.\n", __FUNCTION__);
	hang();
#endif
}


int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
#ifndef MPC83xx_RESET
	ulong addr;
#endif

	volatile immap_t *immap = (immap_t *) CFG_IMMR;

#ifdef MPC83xx_RESET
	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~( MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/* enable Reset Control Reg */
	immap->reset.rpr = 0x52535445;
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* confirm Reset Control Reg is enabled */
	while(!((immap->reset.rcer) & RCER_CRE));

	printf("Resetting the board.");
	printf("\n");

	udelay(200);

	/* perform reset, only one bit */
	immap->reset.rcr = RCR_SWHR;

#else	/* ! MPC83xx_RESET */

	immap->reset.rmr = RMR_CSRE;    /* Checkstop Reset enable */

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
	addr = CFG_RESET_ADDRESS;

	printf("resetting the board.");
	printf("\n");
	((void (*)(void)) addr) ();
#endif	/* MPC83xx_RESET */

	return 1;
}


/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */

unsigned long get_tbclk(void)
{
	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return tbclk;
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
#ifdef CONFIG_MPC834X
	int re_enable = disable_interrupts();

	/* Reset the 83xx watchdog */
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	immr->wdt.swsrr = 0x556c;
	immr->wdt.swsrr = 0xaa39;

	if (re_enable)
		enable_interrupts ();
#else
	hang();
#endif
}
#endif

#if defined(CONFIG_OF_FLAT_TREE)
void
ft_cpu_setup(void *blob, bd_t *bd)
{
	u32 *p;
	int len;
	ulong clock;

	clock = bd->bi_busfreq;
	p = ft_get_prop(blob, "/cpus/" OF_CPU "/bus-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

	p = ft_get_prop(blob, "/" OF_SOC "/bus-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

	p = ft_get_prop(blob, "/" OF_SOC "/serial@4500/clock-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

	p = ft_get_prop(blob, "/" OF_SOC "/serial@4600/clock-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

#ifdef CONFIG_MPC83XX_TSEC1
	p = ft_get_prop(blob, "/" OF_SOC "/ethernet@24000/local-mac-address", &len);
		memcpy(p, bd->bi_enetaddr, 6);
#endif

#ifdef CONFIG_MPC83XX_TSEC2
	p = ft_get_prop(blob, "/" OF_SOC "/ethernet@25000/local-mac-address", &len);
		memcpy(p, bd->bi_enet1addr, 6);
#endif
}
#endif

#if defined(CONFIG_DDR_ECC)
void dma_init(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile dma83xx_t *dma = &immap->dma;
	volatile u32 status = swab32(dma->dmasr0);
	volatile u32 dmamr0 = swab32(dma->dmamr0);

	debug("DMA-init\n");

	/* initialize DMASARn, DMADAR and DMAABCRn */
	dma->dmadar0 = (u32)0;
	dma->dmasar0 = (u32)0;
	dma->dmabcr0 = 0;

	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* clear CS bit */
	dmamr0 &= ~DMA_CHANNEL_START;
	dma->dmamr0 = swab32(dmamr0);
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* while the channel is busy, spin */
	while(status & DMA_CHANNEL_BUSY) {
		status = swab32(dma->dmasr0);
	}

	debug("DMA-init end\n");
}

uint dma_check(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile dma83xx_t *dma = &immap->dma;
	volatile u32 status = swab32(dma->dmasr0);
	volatile u32 byte_count = swab32(dma->dmabcr0);

	/* while the channel is busy, spin */
	while (status & DMA_CHANNEL_BUSY) {
		status = swab32(dma->dmasr0);
	}

	if (status & DMA_CHANNEL_TRANSFER_ERROR) {
		printf ("DMA Error: status = %x @ %d\n", status, byte_count);
	}

	return status;
}

int dma_xfer(void *dest, u32 count, void *src)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile dma83xx_t *dma = &immap->dma;
	volatile u32 dmamr0;

	/* initialize DMASARn, DMADAR and DMAABCRn */
	dma->dmadar0 = swab32((u32)dest);
	dma->dmasar0 = swab32((u32)src);
	dma->dmabcr0 = swab32(count);

	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* init direct transfer, clear CS bit */
	dmamr0 = (DMA_CHANNEL_TRANSFER_MODE_DIRECT |
			DMA_CHANNEL_SOURCE_ADDRESS_HOLD_8B |
			DMA_CHANNEL_SOURCE_ADRESSS_HOLD_EN);

	dma->dmamr0 = swab32(dmamr0);

	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* set CS to start DMA transfer */
	dmamr0 |= DMA_CHANNEL_START;
	dma->dmamr0 = swab32(dmamr0);
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	return ((int)dma_check());
}
#endif /*CONFIG_DDR_ECC*/
