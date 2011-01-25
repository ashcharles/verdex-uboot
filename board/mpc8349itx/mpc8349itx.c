/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006. All rights reserved.
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
 * MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <i2c.h>
#include <spd.h>
#include <miiphy.h>

#ifdef CONFIG_PCI
#include <asm/mpc8349_pci.h>
#include <pci.h>
#endif

#ifdef CONFIG_SPD_EEPROM
#include <spd_sdram.h>
#else
#include <asm/mmu.h>
#endif
#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
#endif

#ifndef CONFIG_SPD_EEPROM
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 ddr_size;		/* The size of RAM, in bytes */
	u32 ddr_size_log2 = 0;

	for (ddr_size = CFG_DDR_SIZE * 0x100000; ddr_size > 1; ddr_size >>= 1) {
		if (ddr_size & 1) {
			return -1;
		}
		ddr_size_log2++;
	}

	im->sysconf.ddrlaw[0].ar =
	    LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);
	im->sysconf.ddrlaw[0].bar = (CFG_DDR_SDRAM_BASE >> 12) & 0xfffff;

	/* Only one CS0 for DDR */
	im->ddr.csbnds[0].csbnds = 0x0000000f;
	im->ddr.cs_config[0] = CFG_DDR_CONFIG;

	debug("cs0_bnds = 0x%08x\n", im->ddr.csbnds[0].csbnds);
	debug("cs0_config = 0x%08x\n", im->ddr.cs_config[0]);

	debug("DDR:bar=0x%08x\n", im->sysconf.ddrlaw[0].bar);
	debug("DDR:ar=0x%08x\n", im->sysconf.ddrlaw[0].ar);

	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;/* Was "2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT" */
	im->ddr.sdram_cfg = SDRAM_CFG_SREN | SDRAM_CFG_SDRAM_TYPE_DDR;
	im->ddr.sdram_mode =
	    (0x0000 << SDRAM_MODE_ESD_SHIFT) | (0x0032 << SDRAM_MODE_SD_SHIFT);
	im->ddr.sdram_interval =
	    (0x0410 << SDRAM_INTERVAL_REFINT_SHIFT) | (0x0100 <<
						       SDRAM_INTERVAL_BSTOPRE_SHIFT);
	im->ddr.sdram_clk_cntl =
	    DDR_SDRAM_CLK_CNTL_SS_EN | DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05;

	udelay(200);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	debug("DDR:timing_cfg_1=0x%08x\n", im->ddr.timing_cfg_1);
	debug("DDR:timing_cfg_2=0x%08x\n", im->ddr.timing_cfg_2);
	debug("DDR:sdram_mode=0x%08x\n", im->ddr.sdram_mode);
	debug("DDR:sdram_interval=0x%08x\n", im->ddr.sdram_interval);
	debug("DDR:sdram_cfg=0x%08x\n", im->ddr.sdram_cfg);

	return CFG_DDR_SIZE;
}
#endif

#ifdef CONFIG_PCI
/*
 * Initialize PCI Devices, report devices found
 */
#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc83xxmitx_config_table[] = {
	{
	 PCI_ANY_ID,
	 PCI_ANY_ID,
	 PCI_ANY_ID,
	 PCI_ANY_ID,
	 0x0f,
	 PCI_ANY_ID,
	 pci_cfgfunc_config_device,
	 {
	  PCI_ENET0_IOADDR,
	  PCI_ENET0_MEMADDR,
	  PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER}
	 },
	{}
}
#endif

volatile static struct pci_controller hose[] = {
	{
#ifndef CONFIG_PCI_PNP
	      config_table:pci_mpc83xxmitx_config_table,
#endif
	 },
	{
#ifndef CONFIG_PCI_PNP
	      config_table:pci_mpc83xxmitx_config_table,
#endif
	 }
};
#endif				/* CONFIG_PCI */

/* If MPC8349E-mITX is soldered with SDRAM, then initialize it. */

void sdram_init(void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile lbus83xx_t *lbc = &immap->lbus;

#if defined(CFG_BR2_PRELIM) \
	&& defined(CFG_OR2_PRELIM) \
	&& defined(CFG_LBLAWBAR2_PRELIM) \
	&& defined(CFG_LBLAWAR2_PRELIM) \
	&& !defined(CONFIG_COMPACT_FLASH)

	uint *sdram_addr = (uint *) CFG_LBC_SDRAM_BASE;

	puts("\n   SDRAM on Local Bus: ");
	print_size(CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers, already done in cpu_init.c
	 */

	/*setup mtrpt, lsrt and lbcr for LB bus */
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->mrtpr = CFG_LBC_MRTPR;
	lbc->lsrt = CFG_LBC_LSRT;
	asm("sync");

	/*
	 * Configure the SDRAM controller Machine Mode register.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_5;	/* 0x40636733; normal operation */

	lbc->lsdmr = CFG_LBC_LSDMR_1;	/*0x68636733; precharge all the banks */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	lbc->lsdmr = CFG_LBC_LSDMR_2;	/*0x48636733; auto refresh */
	asm("sync");
	*sdram_addr = 0xff; /*1 time*/
	udelay(100);
	*sdram_addr = 0xff; /*2 times*/
	udelay(100);
	*sdram_addr = 0xff; /*3 times*/
	udelay(100);
	*sdram_addr = 0xff; /*4 times*/
	udelay(100);
	*sdram_addr = 0xff; /*5 times*/
	udelay(100);
	*sdram_addr = 0xff; /*6 times*/
	udelay(100);
	*sdram_addr = 0xff; /*7 times*/
	udelay(100);
	*sdram_addr = 0xff; /*8 times*/
	udelay(100);

	lbc->lsdmr = CFG_LBC_LSDMR_4;	/*0x58636733;mode register write operation */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	lbc->lsdmr = CFG_LBC_LSDMR_5;	/*0x40636733;normal operation */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

#else
	puts("SDRAM on Local Bus is NOT available!\n");

#ifdef CFG_BR2_PRELIM
	lbc->bank[2].br = CFG_BR2_PRELIM;
	lbc->bank[2].or = CFG_OR2_PRELIM;
#endif

#ifdef CFG_BR3_PRELIM
	lbc->bank[3].br = CFG_BR3_PRELIM;
	lbc->bank[3].or = CFG_OR3_PRELIM;
#endif
#endif
}

long int initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;
#ifdef CONFIG_DDR_ECC
	volatile ddr83xx_t *ddr = &im->ddr;
#endif

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CFG_DDR_BASE & LAWBAR_BAR;
#ifdef CONFIG_SPD_EEPROM
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif

#ifdef CONFIG_DDR_ECC
	if (ddr->sdram_cfg & SDRAM_CFG_ECC_EN)
		/* Unlike every other board, on the 83xx spd_sdram() returns
		   megabytes instead of just bytes.  That's why we need to
		   multiple by 1MB when calling ddr_enable_ecc(). */
		ddr_enable_ecc(msize * 1048576);
#endif

	/*
	 * Initialize SDRAM if it is on local bus.
	 */
	sdram_init();
	puts("   DDR RAM: ");
	/* return total bus SDRAM size(bytes)  -- DDR */
	return msize * 1024 * 1024;
}

int checkboard(void)
{
	puts("Board: Freescale MPC8349E-mITX\n");

	return 0;
}

/*
 * Implement a work-around for a hardware problem with compact
 * flash.
 *
 * Program the UPM if compact flash is enabled.
 */
int misc_init_f(void)
{
	volatile u32 *vsc7385_cpuctrl;

	/* 0x1c0c0 is the VSC7385 CPU Control (CPUCTRL) Register.  The power up
	   default of VSC7385 L1_IRQ and L2_IRQ requests are active high.  That
	   means it is 0 when the IRQ is not active.  This makes the wire-AND
	   logic always assert IRQ7 to CPU even if there is no request from the
	   switch.  Since the compact flash and the switch share the same IRQ,
	   the Linux kernel will think that the compact flash is requesting irq
	   and get stuck when it tries to clear the IRQ.  Thus we need to set
	   the L2_IRQ0 and L2_IRQ1 to active low.

	   The following code sets the L1_IRQ and L2_IRQ polarity to active low.
	   Without this code, compact flash will not work in Linux because
	   unlike U-Boot, Linux uses the IRQ, so this code is necessary if we
	   don't enable compact flash for U-Boot.
	 */

	vsc7385_cpuctrl = (volatile u32 *)(CFG_VSC7385_BASE + 0x1c0c0);
	*vsc7385_cpuctrl |= 0x0c;

#ifdef CONFIG_COMPACT_FLASH
	/* UPM Table Configuration Code */
	static uint UPMATable[] = {
		0xcffffc00, 0x0fffff00, 0x0fafff00, 0x0fafff00,
		0x0faffd00, 0x0faffc04, 0x0ffffc00, 0x3ffffc01,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfff7fc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		0xcffffc00, 0x0fffff00, 0x0ff3ff00, 0x0ff3ff00,
		0x0ff3fe00, 0x0ffffc00, 0x3ffffc05, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
		0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01
	};
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile lbus83xx_t *lbus = &immap->lbus;

	lbus->bank[3].br = CFG_BR3_PRELIM;
	lbus->bank[3].or = CFG_OR3_PRELIM;

	/* Program the MAMR. RFEN=0, OP=00, UWPL=1, AM=000, DS=01, G0CL=000,
	   GPL4=0, RLF=0001, WLF=0001, TLF=0001, MAD=000000
	 */
	lbus->mamr = 0x08404440;

	upmconfig(0, UPMATable, sizeof(UPMATable) / sizeof(UPMATable[0]));

	puts("UPMA:  Configured for compact flash\n");
#endif

	return 0;
}

/*
 * Make sure the EEPROM has the HRCW correctly programmed.
 * Make sure the RTC is correctly programmed.
 *
 * The MPC8349E-mITX can be configured to load the HRCW from
 * EEPROM instead of flash.  This is controlled via jumpers
 * LGPL0, 1, and 3.  Normally, these jumpers are set to 000 (all
 * jumpered), but if they're set to 001 or 010, then the HRCW is
 * read from the "I2C EEPROM".
 *
 * This function makes sure that the I2C EEPROM is programmed
 * correctly.
 */
int misc_init_r(void)
{
	int rc = 0;

#ifdef CONFIG_HARD_I2C

	unsigned int orig_bus = i2c_get_bus_num();;
	u8 i2c_data;

#ifdef CFG_I2C_RTC_ADDR
	u8 ds1339_data[17];
#endif

#ifdef CFG_I2C_EEPROM_ADDR
	static u8 eeprom_data[] =	/* HRCW data */
	{
		0xaa, 0x55, 0xaa,
		0x7c, 0x02, 0x40, 0x05, 0x04, 0x00, 0x00,
		0x7c, 0x02, 0x41, 0xb4, 0x60, 0xa0, 0x00,
	};

	u8 data[sizeof(eeprom_data)];
#endif

	printf("Board revision: ");
	i2c_set_bus_num(1);
	if (i2c_read(CFG_I2C_8574A_ADDR2, 0, 0, &i2c_data, sizeof(i2c_data)) == 0)
		printf("%u.%u (PCF8475A)\n", (i2c_data & 0x02) >> 1, i2c_data & 0x01);
	else if (i2c_read(CFG_I2C_8574_ADDR2, 0, 0, &i2c_data, sizeof(i2c_data)) == 0)
		printf("%u.%u (PCF8475)\n",  (i2c_data & 0x02) >> 1, i2c_data & 0x01);
	else {
		printf("Unknown\n");
		rc = 1;
	}

#ifdef CFG_I2C_EEPROM_ADDR
	i2c_set_bus_num(0);

	if (i2c_read(CFG_I2C_EEPROM_ADDR, 0, 2, data, sizeof(data)) == 0) {
		if (memcmp(data, eeprom_data, sizeof(data)) != 0) {
			if (i2c_write
			    (CFG_I2C_EEPROM_ADDR, 0, 2, eeprom_data,
			     sizeof(eeprom_data)) != 0) {
				puts("Failure writing the HRCW to EEPROM via I2C.\n");
				rc = 1;
			}
		}
	} else {
		puts("Failure reading the HRCW from EEPROM via I2C.\n");
		rc = 1;
	}
#endif

#ifdef CFG_I2C_RTC_ADDR
	i2c_set_bus_num(1);

	if (i2c_read(CFG_I2C_RTC_ADDR, 0, 1, ds1339_data, sizeof(ds1339_data))
	    == 0) {

		/* Work-around for MPC8349E-mITX bug #13601.
		   If the RTC does not contain valid register values, the DS1339
		   Linux driver will not work.
		 */

		/* Make sure status register bits 6-2 are zero */
		ds1339_data[0x0f] &= ~0x7c;

		/* Check for a valid day register value */
		ds1339_data[0x03] &= ~0xf8;
		if (ds1339_data[0x03] == 0) {
			ds1339_data[0x03] = 1;
		}

		/* Check for a valid date register value */
		ds1339_data[0x04] &= ~0xc0;
		if ((ds1339_data[0x04] == 0) ||
		    ((ds1339_data[0x04] & 0x0f) > 9) ||
		    (ds1339_data[0x04] >= 0x32)) {
			ds1339_data[0x04] = 1;
		}

		/* Check for a valid month register value */
		ds1339_data[0x05] &= ~0x60;

		if ((ds1339_data[0x05] == 0) ||
		    ((ds1339_data[0x05] & 0x0f) > 9) ||
		    ((ds1339_data[0x05] >= 0x13)
		     && (ds1339_data[0x05] <= 0x19))) {
			ds1339_data[0x05] = 1;
		}

		/* Enable Oscillator and rate select */
		ds1339_data[0x0e] = 0x1c;

		/* Work-around for MPC8349E-mITX bug #13330.
		   Ensure that the RTC control register contains the value 0x1c.
		   This affects SATA performance.
		 */

		if (i2c_write
		    (CFG_I2C_RTC_ADDR, 0, 1, ds1339_data,
		     sizeof(ds1339_data))) {
			puts("Failure writing to the RTC via I2C.\n");
			rc = 1;
		}
	} else {
		puts("Failure reading from the RTC via I2C.\n");
		rc = 1;
	}
#endif

	i2c_set_bus_num(orig_bus);
#endif

	return rc;
}

#if defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	u32 *p;
	int len;

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	ft_cpu_setup(blob, bd);

	p = ft_get_prop(blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32(bd->bi_memstart);
		*p = cpu_to_be32(bd->bi_memsize);
	}
}
#endif
