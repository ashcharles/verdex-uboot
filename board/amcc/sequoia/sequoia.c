/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,	    AMCC/IBM, alain.saurel@fr.ibm.com
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

#include <common.h>
#include <asm/processor.h>
#include <ppc440.h>
#include "sequoia.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

ulong flash_get_size (ulong base, int banknum);

int board_early_init_f(void)
{
	u32 sdr0_cust0;
	u32 sdr0_pfc1, sdr0_pfc2;
	u32 reg;

	mtdcr(ebccfga, xbcfg);
	mtdcr(ebccfgd, 0xb8400000);

	/*--------------------------------------------------------------------
	 * Setup the GPIO pins
	 *-------------------------------------------------------------------*/
	/* test-only: take GPIO init from pcs440ep ???? in config file */
	out32(GPIO0_OR, 0x00000000);
	out32(GPIO0_TCR, 0x0000000f);
	out32(GPIO0_OSRL, 0x50015400);
	out32(GPIO0_OSRH, 0x550050aa);
	out32(GPIO0_TSRL, 0x50015400);
	out32(GPIO0_TSRH, 0x55005000);
	out32(GPIO0_ISR1L, 0x50000000);
	out32(GPIO0_ISR1H, 0x00000000);
	out32(GPIO0_ISR2L, 0x00000000);
	out32(GPIO0_ISR2H, 0x00000100);
	out32(GPIO0_ISR3L, 0x00000000);
	out32(GPIO0_ISR3H, 0x00000000);

	out32(GPIO1_OR, 0x00000000);
	out32(GPIO1_TCR, 0xc2000000);
	out32(GPIO1_OSRL, 0x5c280000);
	out32(GPIO1_OSRH, 0x00000000);
	out32(GPIO1_TSRL, 0x0c000000);
	out32(GPIO1_TSRH, 0x00000000);
	out32(GPIO1_ISR1L, 0x00005550);
	out32(GPIO1_ISR1H, 0x00000000);
	out32(GPIO1_ISR2L, 0x00050000);
	out32(GPIO1_ISR2H, 0x00000000);
	out32(GPIO1_ISR3L, 0x01400000);
	out32(GPIO1_ISR3H, 0x00000000);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(uic0sr, 0xffffffff);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(uic0pr, 0xfffff7ff);	/* per ref-board manual */
	mtdcr(uic0tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic0vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic0sr, 0xffffffff);	/* clear all */

	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000000);	/* all non-critical */
	mtdcr(uic1pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic1tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	mtdcr(uic2sr, 0xffffffff);	/* clear all */
	mtdcr(uic2er, 0x00000000);	/* disable all */
	mtdcr(uic2cr, 0x00000000);	/* all non-critical */
	mtdcr(uic2pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic2tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic2vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic2sr, 0xffffffff);	/* clear all */

	/* 50MHz tmrclk */
	*(unsigned char *)(CFG_BCSR_BASE | 0x04) = 0x00;

	/* clear write protects */
	*(unsigned char *)(CFG_BCSR_BASE | 0x07) = 0x00;

	/* enable Ethernet */
	*(unsigned char *)(CFG_BCSR_BASE | 0x08) = 0x00;

	/* enable USB device */
	*(unsigned char *)(CFG_BCSR_BASE | 0x09) = 0x20;

	/* select Ethernet pins */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) | SDR0_PFC1_SELECT_CONFIG_4;
	mfsdr(SDR0_PFC2, sdr0_pfc2);
	sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) | SDR0_PFC2_SELECT_CONFIG_4;
	mtsdr(SDR0_PFC2, sdr0_pfc2);
	mtsdr(SDR0_PFC1, sdr0_pfc1);

	/* PCI arbiter enabled */
	mfsdr(sdr_pci0, reg);
	mtsdr(sdr_pci0, 0x80000000 | reg);

	/* setup NAND FLASH */
	mfsdr(SDR0_CUST0, sdr0_cust0);
	sdr0_cust0 = SDR0_CUST0_MUX_NDFC_SEL	|
		SDR0_CUST0_NDFC_ENABLE		|
		SDR0_CUST0_NDFC_BW_8_BIT	|
		SDR0_CUST0_NDFC_ARE_MASK	|
		(0x80000000 >> (28 + CFG_NAND_CS));
	mtsdr(SDR0_CUST0, sdr0_cust0);

	return 0;
}

/*---------------------------------------------------------------------------+
  | misc_init_r.
  +---------------------------------------------------------------------------*/
int misc_init_r(void)
{
	uint pbcr;
	int size_val = 0;
	u32 reg;
#ifdef CONFIG_440EPX
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;
	char *act = getenv("usbact");
#endif

	/*
	 * FLASH stuff...
	 */

	/* Re-do sizing to get full correct info */

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	mtdcr(ebccfga, pb3cr);
#else
	mtdcr(ebccfga, pb0cr);
#endif
	pbcr = mfdcr(ebccfgd);
	switch (gd->bd->bi_flashsize) {
	case 1 << 20:
		size_val = 0;
		break;
	case 2 << 20:
		size_val = 1;
		break;
	case 4 << 20:
		size_val = 2;
		break;
	case 8 << 20:
		size_val = 3;
		break;
	case 16 << 20:
		size_val = 4;
		break;
	case 32 << 20:
		size_val = 5;
		break;
	case 64 << 20:
		size_val = 6;
		break;
	case 128 << 20:
		size_val = 7;
		break;
	}
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	mtdcr(ebccfga, pb3cr);
#else
	mtdcr(ebccfga, pb0cr);
#endif
	mtdcr(ebccfgd, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

#ifdef CFG_ENV_IS_IN_FLASH
	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CFG_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

	/* Env protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    CFG_ENV_ADDR_REDUND,
			    CFG_ENV_ADDR_REDUND + 2*CFG_ENV_SECT_SIZE - 1,
			    &flash_info[0]);
#endif

	/*
	 * USB suff...
	 */
#ifdef CONFIG_440EPX
	if (act == NULL || strcmp(act, "hostdev") == 0)	{
		/* SDR Setting */
		mfsdr(SDR0_PFC1, sdr0_pfc1);
		mfsdr(SDR0_USB0, usb2d0cr);
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mfsdr(SDR0_USB2H0CR, usb2h0cr);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;	/*0*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ;	/*1*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;		/*0*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;		/*1*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;		/*1*/

		/* An 8-bit/60MHz interface is the only possible alternative
		   when connecting the Device to the PHY */
		usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
		usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_16BIT_30MHZ;	/*1*/

		/* To enable the USB 2.0 Device function through the UTMI interface */
		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_USB2DEV_SELECTION;		/*1*/

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_USB2D_SEL;		/*0*/

		mtsdr(SDR0_PFC1, sdr0_pfc1);
		mtsdr(SDR0_USB0, usb2d0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2H0CR, usb2h0cr);

		/*clear resets*/
		udelay (1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Host(int phy) Device(ext phy)\n");

	} else if (strcmp(act, "dev") == 0) {
		/*-------------------PATCH-------------------------------*/
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;	/*0*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;		/*0*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;		/*1*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;		/*1*/
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x672c6000);

		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000080);

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x60206000);

		*(unsigned int *)(0xe0000350) = 0x00000001;

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x60306000);
		/*-------------------PATCH-------------------------------*/

		/* SDR Setting */
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mfsdr(SDR0_USB2H0CR, usb2h0cr);
		mfsdr(SDR0_USB0, usb2d0cr);
		mfsdr(SDR0_PFC1, sdr0_pfc1);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;	/*0*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_8BIT_60MHZ;	/*0*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PUREN;		/*1*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_DEV;		/*0*/
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_DEV;		/*0*/

		usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
		usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_8BIT_60MHZ;		/*0*/

		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_EBC_SELECTION;		/*0*/

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_EBCHR_SEL;		/*1*/

		mtsdr(SDR0_USB2H0CR, usb2h0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB0, usb2d0cr);
		mtsdr(SDR0_PFC1, sdr0_pfc1);

		/*clear resets*/
		udelay (1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Device(int phy)\n");
	}
#endif /* CONFIG_440EPX */

	/*
	 * Clear PLB4A0_ACR[WRP]
	 * This fix will make the MAL burst disabling patch for the Linux
	 * EMAC driver obsolete.
	 */
	reg = mfdcr(plb4_acr) & ~PLB4_ACR_WRP;
	mtdcr(plb4_acr, reg);

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");

#ifdef CONFIG_440EPX
	printf("Board: Sequoia - AMCC PPC440EPx Evaluation Board");
#else
	printf("Board: Rainier - AMCC PPC440GRx Evaluation Board");
#endif
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

#if defined(CFG_DRAM_TEST)
int testdram(void)
{
	unsigned long *mem = (unsigned long *)0;
	const unsigned long kend = (1024 / sizeof(unsigned long));
	unsigned long k, n;

	mtmsr(0);

	for (k = 0; k < CFG_MBYTES_SDRAM;
	     ++k, mem += (1024 / sizeof(unsigned long))) {
		if ((k & 1023) == 0) {
			printf("%3d MB\r", k / 1024);
		}

		memset(mem, 0xaaaaaaaa, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0xaaaaaaaa) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}

		memset(mem, 0x55555555, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0x55555555) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}
	}
	printf("SDRAM test passes\n");
	return 0;
}
#endif

/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;
#if 0
	/*--------------------------------------------------------------------------+
	 *	Cactus is always configured as the host & requires the
	 *	PCI arbiter to be enabled ???
	 *--------------------------------------------------------------------------*/
	unsigned long strap;
	mfsdr(sdr_sdstp1, strap);
	if ((strap & SDR0_SDSTP1_PAE_MASK) == 0) {
		printf("PCI: SDR0_STRP1[PAE] not set.\n");
		printf("PCI: Configuration aborted.\n");
		return 0;
	}
#endif

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB3 devices to 0.
	  | Set PLB3 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	mfsdr(sdr_amp1, addr);
	mtsdr(sdr_amp1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb3_acr);
	mtdcr(plb3_acr, addr | 0x80000000);

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB4 devices to 0.
	  +-------------------------------------------------------------------------*/
	mfsdr(sdr_amp0, addr);
	mtsdr(sdr_amp0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb4_acr) | 0xa0000000;	/* Was 0x8---- */
	mtdcr(plb4_acr, addr);

	/*-------------------------------------------------------------------------+
	  | Set Nebula PLB4 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	/* Segment0 */
	addr = (mfdcr(plb0_acr) & ~plb0_acr_ppm_mask) | plb0_acr_ppm_fair;
	addr = (addr & ~plb0_acr_hbu_mask) | plb0_acr_hbu_enabled;
	addr = (addr & ~plb0_acr_rdp_mask) | plb0_acr_rdp_4deep;
	addr = (addr & ~plb0_acr_wrp_mask) | plb0_acr_wrp_2deep;
	mtdcr(plb0_acr, addr);

	/* Segment1 */
	addr = (mfdcr(plb1_acr) & ~plb1_acr_ppm_mask) | plb1_acr_ppm_fair;
	addr = (addr & ~plb1_acr_hbu_mask) | plb1_acr_hbu_enabled;
	addr = (addr & ~plb1_acr_rdp_mask) | plb1_acr_rdp_4deep;
	addr = (addr & ~plb1_acr_wrp_mask) | plb1_acr_wrp_2deep;
	mtdcr(plb1_acr, addr);

	return 1;
}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Set up Direct MMIO registers
	 *--------------------------------------------------------------------------*/
	/*--------------------------------------------------------------------------+
	  | PowerPC440EPX PCI Master configuration.
	  | Map one 1Gig range of PLB/processor addresses to PCI memory space.
	  |   PLB address 0xA0000000-0xDFFFFFFF ==> PCI address 0xA0000000-0xDFFFFFFF
	  |   Use byte reversed out routines to handle endianess.
	  | Make this region non-prefetchable.
	  +--------------------------------------------------------------------------*/
	out32r(PCIX0_PMM0MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM0LA, CFG_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIX0_PMM0PCILA, CFG_PCI_MEMBASE);	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM0PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_PMM0MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIX0_PMM1MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM1LA, CFG_PCI_MEMBASE2); /* PMM0 Local Address */
	out32r(PCIX0_PMM1PCILA, CFG_PCI_MEMBASE2);	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM1PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_PMM1MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIX0_PTM1MS, 0x00000001);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM1LA, 0);	/* Local Addr. Reg */
	out32r(PCIX0_PTM2MS, 0);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM2LA, 0);	/* Local Addr. Reg */

	/*--------------------------------------------------------------------------+
	 * Set up Configuration registers
	 *--------------------------------------------------------------------------*/

	/* Program the board's subsystem id/vendor id */
	pci_write_config_word(0, PCI_SUBSYSTEM_VENDOR_ID,
			      CFG_PCI_SUBSYS_VENDORID);
	pci_write_config_word(0, PCI_SUBSYSTEM_ID, CFG_PCI_SUBSYS_ID);

	/* Configure command register as bus master */
	pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER);

	/* 240nS PCI clock */
	pci_write_config_word(0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_write_config_word(0, PCI_ERREN, 0);

	pci_write_config_dword(0, PCI_BRDGOPT2, 0x00000101);

}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */

/*************************************************************************
 *  pci_master_init
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;

	/*--------------------------------------------------------------------------+
	  | Write the PowerPC440 EP PCI Configuration regs.
	  |   Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	  |   Enable PowerPC440 EP to act as a PCI memory target (PTM).
	  +--------------------------------------------------------------------------*/
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT) */

/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	/* Cactus is always configured as host. */
	return (1);
}
#endif				/* defined(CONFIG_PCI) */
