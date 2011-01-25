/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (c) 2005 Cisco Systems.  All rights reserved.
 * Roland Dreier <rolandd@cisco.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <asm/processor.h>
#include <asm-ppc/io.h>
#include <ppc4xx.h>
#include <common.h>
#include <pci.h>

#include "440spe_pcie.h"

#if defined(CONFIG_440SPE)
#if defined(CONFIG_PCI)

enum {
	PTYPE_ENDPOINT		= 0x0,
	PTYPE_LEGACY_ENDPOINT	= 0x1,
	PTYPE_ROOT_PORT		= 0x4,

	LNKW_X1			= 0x1,
	LNKW_X4			= 0x4,
	LNKW_X8			= 0x8
};

static int pcie_read_config(struct pci_controller *hose, unsigned int devfn,
	int offset, int len, u32 *val) {

	*val = 0;
	/*
	 * 440SPE implements only one function per port
	 */
	if (!((PCI_FUNC(devfn) == 0) && (PCI_DEV(devfn) == 1)))
		return 0;

	devfn = PCI_BDF(0,0,0);
	offset += devfn << 4;

	switch (len) {
	case 1:
		*val = in_8(hose->cfg_data + offset);
		break;
	case 2:
		*val = in_le16((u16 *)(hose->cfg_data + offset));
		break;
	default:
		*val = in_le32((u32 *)(hose->cfg_data + offset));
		break;
	}
	return 0;
}

static int pcie_write_config(struct pci_controller *hose, unsigned int devfn,
	int offset, int len, u32 val) {

	/*
	 * 440SPE implements only one function per port
	 */
	if (!((PCI_FUNC(devfn) == 0) && (PCI_DEV(devfn) == 1)))
		return 0;

	devfn = PCI_BDF(0,0,0);
	offset += devfn << 4;

	switch (len) {
	case 1:
		out_8(hose->cfg_data + offset, val);
		break;
	case 2:
		out_le16((u16 *)(hose->cfg_data + offset), val);
		break;
	default:
		out_le32((u32 *)(hose->cfg_data + offset), val);
		break;
	}
	return 0;
}

int pcie_read_config_byte(struct pci_controller *hose,pci_dev_t dev,int offset,u8 *val)
{
	u32 v;
	int rv;

	rv =  pcie_read_config(hose, dev, offset, 1, &v);
	*val = (u8)v;
	return rv;
}

int pcie_read_config_word(struct pci_controller *hose,pci_dev_t dev,int offset,u16 *val)
{
	u32 v;
	int rv;

	rv = pcie_read_config(hose, dev, offset, 2, &v);
	*val = (u16)v;
	return rv;
}

int pcie_read_config_dword(struct pci_controller *hose,pci_dev_t dev,int offset,u32 *val)
{
	u32 v;
	int rv;

	rv = pcie_read_config(hose, dev, offset, 3, &v);
	*val = (u32)v;
	return rv;
}

int pcie_write_config_byte(struct pci_controller *hose,pci_dev_t dev,int offset,u8 val)
{
	return pcie_write_config(hose,(u32)dev,offset,1,val);
}

int pcie_write_config_word(struct pci_controller *hose,pci_dev_t dev,int offset,u16 val)
{
	return pcie_write_config(hose,(u32)dev,offset,2,(u32 )val);
}

int pcie_write_config_dword(struct pci_controller *hose,pci_dev_t dev,int offset,u32 val)
{
	return pcie_write_config(hose,(u32)dev,offset,3,(u32 )val);
}

static void ppc440spe_setup_utl(u32 port) {

	volatile void *utl_base = NULL;

	/*
	 * Map UTL registers
	 */
	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE0), 0x0000000c);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE0), 0x20000000);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE0), 0x00007001);
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE0), 0x68782800);
		break;

	case 1:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE1), 0x0000000c);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE1), 0x20001000);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE1), 0x00007001);
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE1), 0x68782800);
		break;

	case 2:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE2), 0x0000000c);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE2), 0x20002000);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE2), 0x00007001);
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE2), 0x68782800);
		break;
	}
	utl_base = (unsigned int *)(CFG_PCIE_BASE + 0x1000 * port);

	/*
	 * Set buffer allocations and then assert VRB and TXE.
	 */
	out_be32(utl_base + PEUTL_OUTTR,   0x08000000);
	out_be32(utl_base + PEUTL_INTR,    0x02000000);
	out_be32(utl_base + PEUTL_OPDBSZ,  0x10000000);
	out_be32(utl_base + PEUTL_PBBSZ,   0x53000000);
	out_be32(utl_base + PEUTL_IPHBSZ,  0x08000000);
	out_be32(utl_base + PEUTL_IPDBSZ,  0x10000000);
	out_be32(utl_base + PEUTL_RCIRQEN, 0x00f00000);
	out_be32(utl_base + PEUTL_PCTL,    0x80800066);
}

static int check_error(void)
{
	u32 valPE0, valPE1, valPE2;
	int err = 0;

	/* SDR0_PEGPLLLCT1 reset */
	if (!(valPE0 = SDR_READ(PESDR0_PLLLCT1) & 0x01000000)) {
		printf("PCIE: SDR0_PEGPLLLCT1 reset error 0x%x\n", valPE0);
	}

	valPE0 = SDR_READ(PESDR0_RCSSET);
	valPE1 = SDR_READ(PESDR1_RCSSET);
	valPE2 = SDR_READ(PESDR2_RCSSET);

	/* SDR0_PExRCSSET rstgu */
	if (!(valPE0 & 0x01000000) ||
	    !(valPE1 & 0x01000000) ||
	    !(valPE2 & 0x01000000)) {
		printf("PCIE:  SDR0_PExRCSSET rstgu error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rstdl */
	if (!(valPE0 & 0x00010000) ||
	    !(valPE1 & 0x00010000) ||
	    !(valPE2 & 0x00010000)) {
		printf("PCIE:  SDR0_PExRCSSET rstdl error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rstpyn */
	if ((valPE0 & 0x00001000) ||
	    (valPE1 & 0x00001000) ||
	    (valPE2 & 0x00001000)) {
		printf("PCIE:  SDR0_PExRCSSET rstpyn error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET hldplb */
	if ((valPE0 & 0x10000000) ||
	    (valPE1 & 0x10000000) ||
	    (valPE2 & 0x10000000)) {
		printf("PCIE:  SDR0_PExRCSSET hldplb error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rdy */
	if ((valPE0 & 0x00100000) ||
	    (valPE1 & 0x00100000) ||
	    (valPE2 & 0x00100000)) {
		printf("PCIE:  SDR0_PExRCSSET rdy error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET shutdown */
	if ((valPE0 & 0x00000100) ||
	    (valPE1 & 0x00000100) ||
	    (valPE2 & 0x00000100)) {
		printf("PCIE:  SDR0_PExRCSSET shutdown error\n");
		err = -1;
	}
	return err;
}

/*
 * Initialize PCI Express core
 */
int ppc440spe_init_pcie(void)
{
	int time_out = 20;

	/* Set PLL clock receiver to LVPECL */
	SDR_WRITE(PESDR0_PLLLCT1, SDR_READ(PESDR0_PLLLCT1) | 1 << 28);

	if (check_error())
		return -1;

	if (!(SDR_READ(PESDR0_PLLLCT2) & 0x10000))
	{
		printf("PCIE: PESDR_PLLCT2 resistance calibration failed (0x%08x)\n",
		       SDR_READ(PESDR0_PLLLCT2));
		return -1;
	}
	/* De-assert reset of PCIe PLL, wait for lock */
	SDR_WRITE(PESDR0_PLLLCT1, SDR_READ(PESDR0_PLLLCT1) & ~(1 << 24));
	udelay(3);

	while (time_out) {
		if (!(SDR_READ(PESDR0_PLLLCT3) & 0x10000000)) {
			time_out--;
			udelay(1);
		} else
			break;
	}
	if (!time_out) {
		printf("PCIE: VCO output not locked\n");
		return -1;
	}
	return 0;
}

/*
 *  Yucca board as End point and root point setup
 *                    and
 *    testing inbound and out bound windows
 *
 *  YUCCA board can be plugged into another yucca board or you can get PCI-E
 *  cable which can be used to setup loop back from one port to another port.
 *  Please rememeber that unless there is a endpoint plugged in to root port it
 *  will not initialize. It is the same in case of endpoint , unless there is
 *  root port attached it will not initialize.
 *
 *  In this release of software all the PCI-E ports are configured as either
 *  endpoint or rootpoint.In future we will have support for selective ports
 *  setup as endpoint and root point in single board.
 *
 *  Once your board came up as root point , you can verify by reading
 *  /proc/bus/pci/devices. Where you can see the configuration registers
 *  of end point device attached to the port.
 *
 *  Enpoint cofiguration can be verified by connecting Yucca board to any
 *  host or another yucca board. Then try to scan the device. In case of
 *  linux use "lspci" or appripriate os command.
 *
 *  How do I verify the inbound and out bound windows ?(yucca to yucca)
 *  in this configuration inbound and outbound windows are setup to access
 *  sram memroy area. SRAM is at 0x4 0000 0000 , on PLB bus. This address
 *  is mapped at 0x90000000. From u-boot prompt write data 0xb000 0000,
 *  This is waere your POM(PLB out bound memory window) mapped. then
 *  read the data from other yucca board's u-boot prompt at address
 *  0x9000 0000(SRAM). Data should match.
 *  In case of inbound , write data to u-boot command prompt at 0xb000 0000
 *  which is mapped to 0x4 0000 0000. Now on rootpoint yucca u-boot prompt check
 *  data at 0x9000 0000(SRAM).Data should match.
 */
int ppc440spe_init_pcie_rootport(int port)
{
	static int core_init;
	volatile u32 val = 0;
	int attempts;

	if (!core_init) {
		++core_init;
		if (ppc440spe_init_pcie())
			return -1;
	}

	/*
	 * Initialize various parts of the PCI Express core for our port:
	 *
	 * - Set as a root port and enable max width
	 *   (PXIE0 -> X8, PCIE1 and PCIE2 -> X4).
	 * - Set up UTL configuration.
	 * - Increase SERDES drive strength to levels suggested by AMCC.
	 * - De-assert RSTPYN, RSTDL and RSTGU.
	 *
	 * NOTICE for revB chip: PESDRn_UTLSET2 is not set - we leave it with
	 * default setting 0x11310000. The register has new fields,
	 * PESDRn_UTLSET2[LKINE] in particular: clearing it leads to PCIE core
	 * hang.
	 */
	switch (port) {
	case 0:
		SDR_WRITE(PESDR0_DLPSET,  1 << 24 | PTYPE_ROOT_PORT << 20 | LNKW_X8 << 12);

		SDR_WRITE(PESDR0_UTLSET1, 0x21222222);
		if (!ppc440spe_revB())
			SDR_WRITE(PESDR0_UTLSET2, 0x11000000);
		SDR_WRITE(PESDR0_HSSL0SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL1SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL2SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL3SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL4SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL5SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL6SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL7SET1, 0x35000000);
		SDR_WRITE(PESDR0_RCSSET,
			  (SDR_READ(PESDR0_RCSSET) & ~(1 << 24 | 1 << 16)) | 1 << 12);
		break;

	case 1:
		SDR_WRITE(PESDR1_DLPSET, 1 << 24 | PTYPE_ROOT_PORT << 20 | LNKW_X4 << 12);
		SDR_WRITE(PESDR1_UTLSET1, 0x21222222);
		if (!ppc440spe_revB())
			SDR_WRITE(PESDR1_UTLSET2, 0x11000000);
		SDR_WRITE(PESDR1_HSSL0SET1, 0x35000000);
		SDR_WRITE(PESDR1_HSSL1SET1, 0x35000000);
		SDR_WRITE(PESDR1_HSSL2SET1, 0x35000000);
		SDR_WRITE(PESDR1_HSSL3SET1, 0x35000000);
		SDR_WRITE(PESDR1_RCSSET,
			  (SDR_READ(PESDR1_RCSSET) & ~(1 << 24 | 1 << 16)) | 1 << 12);
		break;

	case 2:
		SDR_WRITE(PESDR2_DLPSET, 1 << 24 | PTYPE_ROOT_PORT << 20 | LNKW_X4 << 12);
		SDR_WRITE(PESDR2_UTLSET1, 0x21222222);
		if (!ppc440spe_revB())
			SDR_WRITE(PESDR2_UTLSET2, 0x11000000);
		SDR_WRITE(PESDR2_HSSL0SET1, 0x35000000);
		SDR_WRITE(PESDR2_HSSL1SET1, 0x35000000);
		SDR_WRITE(PESDR2_HSSL2SET1, 0x35000000);
		SDR_WRITE(PESDR2_HSSL3SET1, 0x35000000);
		SDR_WRITE(PESDR2_RCSSET,
			  (SDR_READ(PESDR2_RCSSET) & ~(1 << 24 | 1 << 16)) | 1 << 12);
		break;
	}
	/*
	 * Notice: the following delay has critical impact on device
	 * initialization - if too short (<50ms) the link doesn't get up.
	 */
	mdelay(100);

	switch (port) {
	case 0:
		val = SDR_READ(PESDR0_RCSSTS);
		break;
	case 1:
		val = SDR_READ(PESDR1_RCSSTS);
		break;
	case 2:
		val = SDR_READ(PESDR2_RCSSTS);
		break;
	}

	if (val & (1 << 20)) {
		printf("PCIE%d: PGRST failed %08x\n", port, val);
		return -1;
	}

	/*
	 * Verify link is up
	 */
	val = 0;
	switch (port) {
	case 0:
		val = SDR_READ(PESDR0_LOOP);
		break;
	case 1:
		val = SDR_READ(PESDR1_LOOP);
		break;
	case 2:
		val = SDR_READ(PESDR2_LOOP);
		break;
	}
	if (!(val & 0x00001000)) {
		printf("PCIE%d: link is not up.\n", port);
		return -1;
	}

	/*
	 * Setup UTL registers - but only on revA!
	 * We use default settings for revB chip.
	 */
	if (!ppc440spe_revB())
		ppc440spe_setup_utl(port);

	/*
	 * We map PCI Express configuration access into the 512MB regions
	 *
	 * NOTICE: revB is very strict about PLB real addressess and ranges to
	 * be mapped for config space; it seems to only work with d_nnnn_nnnn
	 * range (hangs the core upon config transaction attempts when set
	 * otherwise) while revA uses c_nnnn_nnnn.
	 *
	 * For revA:
	 *     PCIE0: 0xc_4000_0000
	 *     PCIE1: 0xc_8000_0000
	 *     PCIE2: 0xc_c000_0000
	 *
	 * For revB:
	 *     PCIE0: 0xd_0000_0000
	 *     PCIE1: 0xd_2000_0000
	 *     PCIE2: 0xd_4000_0000
	 */

	switch (port) {
	case 0:
		if (ppc440spe_revB()) {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE0), 0x0000000d);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE0), 0x00000000);
		} else {
			/* revA */
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE0), 0x0000000c);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE0), 0x40000000);
		}
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE0), 0xe0000001); /* 512MB region, valid */
		break;

	case 1:
		if (ppc440spe_revB()) {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE1), 0x0000000d);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE1), 0x20000000);
		} else {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE1), 0x0000000c);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE1), 0x80000000);
		}
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE1), 0xe0000001); /* 512MB region, valid */
		break;

	case 2:
		if (ppc440spe_revB()) {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE2), 0x0000000d);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE2), 0x40000000);
		} else {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE2), 0x0000000c);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE2), 0xc0000000);
		}
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE2), 0xe0000001); /* 512MB region, valid */
		break;
	}

	/*
	 * Check for VC0 active and assert RDY.
	 */
	attempts = 10;
	switch (port) {
	case 0:
		while(!(SDR_READ(PESDR0_RCSSTS) & (1 << 16))) {
			if (!(attempts--)) {
				printf("PCIE0: VC0 not active\n");
				return -1;
			}
			mdelay(1000);
		}
		SDR_WRITE(PESDR0_RCSSET, SDR_READ(PESDR0_RCSSET) | 1 << 20);
		break;
	case 1:
		while(!(SDR_READ(PESDR1_RCSSTS) & (1 << 16))) {
			if (!(attempts--)) {
				printf("PCIE1: VC0 not active\n");
				return -1;
			}
			mdelay(1000);
		}

		SDR_WRITE(PESDR1_RCSSET, SDR_READ(PESDR1_RCSSET) | 1 << 20);
		break;
	case 2:
		while(!(SDR_READ(PESDR2_RCSSTS) & (1 << 16))) {
			if (!(attempts--)) {
				printf("PCIE2: VC0 not active\n");
				return -1;
			}
			mdelay(1000);
		}

		SDR_WRITE(PESDR2_RCSSET, SDR_READ(PESDR2_RCSSET) | 1 << 20);
		break;
	}
	mdelay(100);

	return 0;
}

int ppc440spe_init_pcie_endport(int port)
{
	static int core_init;
	volatile u32 val = 0;
	int attempts;

	if (!core_init) {
		++core_init;
		if (ppc440spe_init_pcie())
			return -1;
	}

	/*
	 * Initialize various parts of the PCI Express core for our port:
	 *
	 * - Set as a end port and enable max width
	 *   (PXIE0 -> X8, PCIE1 and PCIE2 -> X4).
	 * - Set up UTL configuration.
	 * - Increase SERDES drive strength to levels suggested by AMCC.
	 * - De-assert RSTPYN, RSTDL and RSTGU.
	 *
	 * NOTICE for revB chip: PESDRn_UTLSET2 is not set - we leave it with
	 * default setting 0x11310000. The register has new fields,
	 * PESDRn_UTLSET2[LKINE] in particular: clearing it leads to PCIE core
	 * hang.
	 */
	switch (port) {
	case 0:
		SDR_WRITE(PESDR0_DLPSET,  1 << 24 | PTYPE_LEGACY_ENDPOINT << 20 | LNKW_X8 << 12);

		SDR_WRITE(PESDR0_UTLSET1, 0x20222222);
		if (!ppc440spe_revB())
			SDR_WRITE(PESDR0_UTLSET2, 0x11000000);
		SDR_WRITE(PESDR0_HSSL0SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL1SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL2SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL3SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL4SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL5SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL6SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL7SET1, 0x35000000);
		SDR_WRITE(PESDR0_RCSSET,
			(SDR_READ(PESDR0_RCSSET) & ~(1 << 24 | 1 << 16)) | 1 << 12);
		break;

	case 1:
		SDR_WRITE(PESDR1_DLPSET, 1 << 24 | PTYPE_LEGACY_ENDPOINT << 20 | LNKW_X4 << 12);
		SDR_WRITE(PESDR1_UTLSET1, 0x20222222);
		if (!ppc440spe_revB())
			SDR_WRITE(PESDR1_UTLSET2, 0x11000000);
		SDR_WRITE(PESDR1_HSSL0SET1, 0x35000000);
		SDR_WRITE(PESDR1_HSSL1SET1, 0x35000000);
		SDR_WRITE(PESDR1_HSSL2SET1, 0x35000000);
		SDR_WRITE(PESDR1_HSSL3SET1, 0x35000000);
		SDR_WRITE(PESDR1_RCSSET,
			(SDR_READ(PESDR1_RCSSET) & ~(1 << 24 | 1 << 16)) | 1 << 12);
		break;

	case 2:
		SDR_WRITE(PESDR2_DLPSET, 1 << 24 | PTYPE_LEGACY_ENDPOINT << 20 | LNKW_X4 << 12);
		SDR_WRITE(PESDR2_UTLSET1, 0x20222222);
		if (!ppc440spe_revB())
			SDR_WRITE(PESDR2_UTLSET2, 0x11000000);
		SDR_WRITE(PESDR2_HSSL0SET1, 0x35000000);
		SDR_WRITE(PESDR2_HSSL1SET1, 0x35000000);
		SDR_WRITE(PESDR2_HSSL2SET1, 0x35000000);
		SDR_WRITE(PESDR2_HSSL3SET1, 0x35000000);
		SDR_WRITE(PESDR2_RCSSET,
			(SDR_READ(PESDR2_RCSSET) & ~(1 << 24 | 1 << 16)) | 1 << 12);
		break;
	}
	/*
	 * Notice: the following delay has critical impact on device
	 * initialization - if too short (<50ms) the link doesn't get up.
	 */
	mdelay(100);

	switch (port) {
	case 0: val = SDR_READ(PESDR0_RCSSTS); break;
	case 1: val = SDR_READ(PESDR1_RCSSTS); break;
	case 2: val = SDR_READ(PESDR2_RCSSTS); break;
	}

	if (val & (1 << 20)) {
		printf("PCIE%d: PGRST failed %08x\n", port, val);
		return -1;
	}

	/*
	 * Verify link is up
	 */
	val = 0;
	switch (port)
	{
		case 0:
			val = SDR_READ(PESDR0_LOOP);
			break;
		case 1:
			val = SDR_READ(PESDR1_LOOP);
			break;
		case 2:
			val = SDR_READ(PESDR2_LOOP);
			break;
	}
	if (!(val & 0x00001000)) {
		printf("PCIE%d: link is not up.\n", port);
		return -1;
	}

	/*
	 * Setup UTL registers - but only on revA!
	 * We use default settings for revB chip.
	 */
	if (!ppc440spe_revB())
		ppc440spe_setup_utl(port);

	/*
	 * We map PCI Express configuration access into the 512MB regions
	 *
	 * NOTICE: revB is very strict about PLB real addressess and ranges to
	 * be mapped for config space; it seems to only work with d_nnnn_nnnn
	 * range (hangs the core upon config transaction attempts when set
	 * otherwise) while revA uses c_nnnn_nnnn.
	 *
	 * For revA:
	 *     PCIE0: 0xc_4000_0000
	 *     PCIE1: 0xc_8000_0000
	 *     PCIE2: 0xc_c000_0000
	 *
	 * For revB:
	 *     PCIE0: 0xd_0000_0000
	 *     PCIE1: 0xd_2000_0000
	 *     PCIE2: 0xd_4000_0000
	 */
	switch (port) {
	case 0:
		if (ppc440spe_revB()) {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE0), 0x0000000d);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE0), 0x00000000);
		} else {
			/* revA */
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE0), 0x0000000c);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE0), 0x40000000);
		}
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE0), 0xe0000001); /* 512MB region, valid */
		break;

	case 1:
		if (ppc440spe_revB()) {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE1), 0x0000000d);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE1), 0x20000000);
		} else {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE1), 0x0000000c);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE1), 0x80000000);
		}
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE1), 0xe0000001); /* 512MB region, valid */
		break;

	case 2:
		if (ppc440spe_revB()) {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE2), 0x0000000d);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE2), 0x40000000);
		} else {
			mtdcr(DCRN_PEGPL_CFGBAH(PCIE2), 0x0000000c);
			mtdcr(DCRN_PEGPL_CFGBAL(PCIE2), 0xc0000000);
		}
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE2), 0xe0000001); /* 512MB region, valid */
		break;
	}

	/*
	 * Check for VC0 active and assert RDY.
	 */
	attempts = 10;
	switch (port) {
	case 0:
		while(!(SDR_READ(PESDR0_RCSSTS) & (1 << 16))) {
			if (!(attempts--)) {
				printf("PCIE0: VC0 not active\n");
				return -1;
			}
			mdelay(1000);
		}
		SDR_WRITE(PESDR0_RCSSET, SDR_READ(PESDR0_RCSSET) | 1 << 20);
		break;
	case 1:
		while(!(SDR_READ(PESDR1_RCSSTS) & (1 << 16))) {
			if (!(attempts--)) {
				printf("PCIE1: VC0 not active\n");
				return -1;
			}
			mdelay(1000);
		}

		SDR_WRITE(PESDR1_RCSSET, SDR_READ(PESDR1_RCSSET) | 1 << 20);
		break;
	case 2:
		while(!(SDR_READ(PESDR2_RCSSTS) & (1 << 16))) {
			if (!(attempts--)) {
				printf("PCIE2: VC0 not active\n");
				return -1;
			}
			mdelay(1000);
		}

		SDR_WRITE(PESDR2_RCSSET, SDR_READ(PESDR2_RCSSET) | 1 << 20);
		break;
	}
	mdelay(100);

	return 0;
}

void ppc440spe_setup_pcie_rootpoint(struct pci_controller *hose, int port)
{
	volatile void *mbase = NULL;
	volatile void *rmbase = NULL;

	pci_set_ops(hose,
		    pcie_read_config_byte,
		    pcie_read_config_word,
		    pcie_read_config_dword,
		    pcie_write_config_byte,
		    pcie_write_config_word,
		    pcie_write_config_dword);

	switch (port) {
	case 0:
		mbase = (u32 *)CFG_PCIE0_XCFGBASE;
		rmbase = (u32 *)CFG_PCIE0_CFGBASE;
		hose->cfg_data = (u8 *)CFG_PCIE0_CFGBASE;
		break;
	case 1:
		mbase = (u32 *)CFG_PCIE1_XCFGBASE;
		rmbase = (u32 *)CFG_PCIE1_CFGBASE;
		hose->cfg_data = (u8 *)CFG_PCIE1_CFGBASE;
		break;
	case 2:
		mbase = (u32 *)CFG_PCIE2_XCFGBASE;
		rmbase = (u32 *)CFG_PCIE2_CFGBASE;
		hose->cfg_data = (u8 *)CFG_PCIE2_CFGBASE;
		break;
	}

	/*
	 * Set bus numbers on our root port
	 */
	out_8((u8 *)mbase + PCI_PRIMARY_BUS, 0);
	out_8((u8 *)mbase + PCI_SECONDARY_BUS, 1);
	out_8((u8 *)mbase + PCI_SUBORDINATE_BUS, 1);

	/*
	 * Set up outbound translation to hose->mem_space from PLB
	 * addresses at an offset of 0xd_0000_0000.  We set the low
	 * bits of the mask to 11 to turn off splitting into 8
	 * subregions and to enable the outbound translation.
	 */
	out_le32(mbase + PECFG_POM0LAH, 0x00000000);
	out_le32(mbase + PECFG_POM0LAL, 0x00000000);

	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE0),  0x0000000d);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE0),  CFG_PCIE_MEMBASE +
			port * CFG_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE0), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE0),
			~(CFG_PCIE_MEMSIZE - 1) | 3);
		break;
	case 1:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE1),  0x0000000d);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE1),  (CFG_PCIE_MEMBASE +
			port * CFG_PCIE_MEMSIZE));
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE1), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE1),
			~(CFG_PCIE_MEMSIZE - 1) | 3);
		break;
	case 2:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE2),  0x0000000d);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE2),  (CFG_PCIE_MEMBASE +
			port * CFG_PCIE_MEMSIZE));
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE2), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE2),
			~(CFG_PCIE_MEMSIZE - 1) | 3);
		break;
	}

	/* Set up 16GB inbound memory window at 0 */
	out_le32(mbase + PCI_BASE_ADDRESS_0, 0);
	out_le32(mbase + PCI_BASE_ADDRESS_1, 0);
	out_le32(mbase + PECFG_BAR0HMPA, 0x7fffffc);
	out_le32(mbase + PECFG_BAR0LMPA, 0);

	out_le32(mbase + PECFG_PIM01SAH, 0xffff0000);
	out_le32(mbase + PECFG_PIM01SAL, 0x00000000);
	out_le32(mbase + PECFG_PIM0LAL, 0);
	out_le32(mbase + PECFG_PIM0LAH, 0);
	out_le32(mbase + PECFG_PIM1LAL,  0x00000000);
	out_le32(mbase + PECFG_PIM1LAH,  0x00000004);
	out_le32(mbase + PECFG_PIMEN, 0x1);

	/* Enable I/O, Mem, and Busmaster cycles */
	out_le16((u16 *)(mbase + PCI_COMMAND),
		 in_le16((u16 *)(mbase + PCI_COMMAND)) |
		 PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	printf("PCIE:%d successfully set as rootpoint\n",port);
}

int ppc440spe_setup_pcie_endpoint(struct pci_controller *hose, int port)
{
	volatile void *mbase = NULL;
	int attempts = 0;

	pci_set_ops(hose,
		    pcie_read_config_byte,
		    pcie_read_config_word,
		    pcie_read_config_dword,
		    pcie_write_config_byte,
		    pcie_write_config_word,
		    pcie_write_config_dword);

	switch (port) {
	case 0:
		mbase = (u32 *)CFG_PCIE0_XCFGBASE;
		hose->cfg_data = (u8 *)CFG_PCIE0_CFGBASE;
		break;
	case 1:
		mbase = (u32 *)CFG_PCIE1_XCFGBASE;
		hose->cfg_data = (u8 *)CFG_PCIE1_CFGBASE;
		break;
	case 2:
		mbase = (u32 *)CFG_PCIE2_XCFGBASE;
		hose->cfg_data = (u8 *)CFG_PCIE2_CFGBASE;
		break;
	}

	/*
	 * Set up outbound translation to hose->mem_space from PLB
	 * addresses at an offset of 0xd_0000_0000.  We set the low
	 * bits of the mask to 11 to turn off splitting into 8
	 * subregions and to enable the outbound translation.
	 */
	out_le32(mbase + PECFG_POM0LAH, 0x00001ff8);
	out_le32(mbase + PECFG_POM0LAL, 0x00001000);

	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE0),  0x0000000d);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE0),  CFG_PCIE_MEMBASE +
			port * CFG_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE0), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE0),
			~(CFG_PCIE_MEMSIZE - 1) | 3);
		break;
	case 1:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE1),  0x0000000d);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE1),  (CFG_PCIE_MEMBASE +
			port * CFG_PCIE_MEMSIZE));
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE1), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE1),
			~(CFG_PCIE_MEMSIZE - 1) | 3);
		break;
	case 2:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE2),  0x0000000d);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE2),  (CFG_PCIE_MEMBASE +
			port * CFG_PCIE_MEMSIZE));
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE2), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE2),
			~(CFG_PCIE_MEMSIZE - 1) | 3);
		break;
	}

	/* Set up 16GB inbound memory window at 0 */
	out_le32(mbase + PCI_BASE_ADDRESS_0, 0);
	out_le32(mbase + PCI_BASE_ADDRESS_1, 0);
	out_le32(mbase + PECFG_BAR0HMPA, 0x7fffffc);
	out_le32(mbase + PECFG_BAR0LMPA, 0);
	out_le32(mbase + PECFG_PIM0LAL, 0x00000000);
	out_le32(mbase + PECFG_PIM0LAH, 0x00000004);	/* pointing to SRAM */
	out_le32(mbase + PECFG_PIMEN, 0x1);

	/* Enable I/O, Mem, and Busmaster cycles */
	out_le16((u16 *)(mbase + PCI_COMMAND),
		 in_le16((u16 *)(mbase + PCI_COMMAND)) |
		 PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	out_le16(mbase + 0x200,0xcaad);			/* Setting vendor ID */
	out_le16(mbase + 0x202,0xfeed);			/* Setting device ID */
	attempts = 10;
	switch (port) {
	case 0:
		while (!(SDR_READ(PESDR0_RCSSTS) & (1 << 8))) {
			if (!(attempts--)) {
				printf("PCIE0: BMEN is  not active\n");
				return -1;
			}
			mdelay(1000);
		}
		break;
	case 1:
		while (!(SDR_READ(PESDR1_RCSSTS) & (1 << 8))) {
			if (!(attempts--)) {
				printf("PCIE1: BMEN is not active\n");
				return -1;
			}
			mdelay(1000);
		}
		break;
	case 2:
		while (!(SDR_READ(PESDR2_RCSSTS) & (1 << 8))) {
			if (!(attempts--)) {
				printf("PCIE2: BMEN is  not active\n");
				return -1;
			}
			mdelay(1000);
		}
		break;
	}
	printf("PCIE:%d successfully set as endpoint\n",port);

	return 0;
}
#endif /* CONFIG_PCI */
#endif /* CONFIG_440SPE */
