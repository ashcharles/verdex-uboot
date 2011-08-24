/* 
 * Copyright (C) 2004 Gumstix, Inc.
**
**   Gumstix u-boot 1.1.1
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
 *
 * Written by Craig Hughes <craig@gumstix.com> 2004
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_GUMSTIX		/* config for gumstix board	*/
#undef  CONFIG_USE_IRQ		/* don't need use IRQ/FIQ	*/

#ifdef GUMSTIX_VERDEX
#define CONFIG_PXA27X
#define GUMSTIX_CPU "PXA270"
#else
#define GUMSTIX_CPU "PXA255"
#endif
#define CONFIG_PXA250

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234	/* For some reason this is not defined */
#endif
#include "asm/arch/pxa-regs.h"

/*
 * Select serial console configuration FFUART is default on G dev board
 */

#define CONFIG_FFUART
#define CONFIG_BAUDRATE		115200
#define CONFIG_SILENT_CONSOLE

/*
 * Definition of u-boot build in commands. Check out CONFIG_CMD_DFL if
 * neccessary in include/cmd_confdefs.h file. (Un)comment for getting
 * functionality or size of u-boot code.
 */

#define CONFIG_COMMANDS	(  CONFIG_CMD_DFL	\
			& ~CFG_CMD_CDP		\
			& ~CFG_CMD_IMLS		\
			& ~CFG_CMD_IRQ		\
			& ~CFG_CMD_SETGETDCR	\
			| CFG_CMD_AUTOSCRIPT	\
			| CFG_CMD_ASKENV	\
			| CFG_CMD_BDI		\
			| CFG_CMD_BOOTD 	\
			| CFG_CMD_CACHE 	\
			| CFG_CMD_CONSOLE	\
			| CFG_CMD_DHCP		\
			| CFG_CMD_DIAG		\
			| CFG_CMD_ECHO		\
			| CFG_CMD_ELF		\
			| CFG_CMD_ENV		\
			| CFG_CMD_FAT		\
			| CFG_CMD_FLASH		\
			| CFG_CMD_IDE		\
			| CFG_CMD_IMI		\
			| CFG_CMD_ITEST 	\
			| CFG_CMD_JFFS2 	\
			| CFG_CMD_KAT		\
			| CFG_CMD_LOADB		\
			| CFG_CMD_LOADS 	\
			| CFG_CMD_MEMORY	\
			| CFG_CMD_MISC		\
			| CFG_CMD_MMC		\
			| CFG_CMD_NET 		\
			| CFG_CMD_NFS		\
			| CFG_CMD_PCMCIA	\
			| CFG_CMD_PING		\
			| CFG_CMD_REGINFO	\
			| CFG_CMD_RUN		\
			| CFG_CMD_SAVES		\
			| CFG_CMD_SDRAM		\
				)

#define CONFIG_SUPPORT_VFAT
#define CFG_DEVICE_NULLDEV

#include <cmd_confdefs.h>

/*
 * Boot options. Setting delay to -1 stops autostart count down.
 * NOTE: Sending parameters to kernel depends on kernel version and
 * 2.4.19-rmk6-pxa1 patch used while my u-boot coding didn't accept 
 * parameters at all! Do not get confused by them so.
*/

#ifdef CONFIG_GUMSTIX_CPUSPEED_200
#define CFG_CPUSPEED_STR	"200"
/* CPDIS: 0, PPDIS: 0, L: 16, 2N: 2  yields run=13*16=208, turbo=208*2/2=208, clkmem=table 3-7 */
#define CCCR_VAL	0x00000110
#endif
#ifdef CONFIG_GUMSTIX_CPUSPEED_300
#define CFG_CPUSPEED_STR	"300"
/* CPDIS: 0, PPDIS: 0, L: 16, 2N: 3  yields run=13*16=208, turbo=208*3/2=312, clkmem=table 3-7 */
#define CCCR_VAL	0x00000190
#endif
#ifdef CONFIG_GUMSTIX_CPUSPEED_400
#define CFG_CPUSPEED_STR	"400"
/* CPDIS: 0, PPDIS: 0, L: 16, 2N: 4  yields run=13*16=208, turbo=208*4/2=416, clkmem=table 3-7 */
#define CCCR_VAL	0x00000210
#endif
#ifdef CONFIG_GUMSTIX_CPUSPEED_500
#define CFG_CPUSPEED_STR	"500"
/* CPDIS: 0, PPDIS: 0, L: 16, 2N: 5  yields run=13*16=208, turbo=208*5/2=520, clkmem=table 3-7 */
#define CCCR_VAL	0x00000290
#endif
#ifdef CONFIG_GUMSTIX_CPUSPEED_600
#define CFG_CPUSPEED_STR	"600"
/* CPDIS: 0, PPDIS: 0, L: 16, 2N: 6  yields run=13*16=208, turbo=208*6/2=624, clkmem=table 3-7 */
#define CCCR_VAL	0x00000310
#endif

#define CONFIG_IDENT_STRING	" - " GUMSTIX_CPU "@" CFG_CPUSPEED_STR " MHz - " SVN_REVISION "\n\n*** Welcome to Gumstix ***"
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R	/* misc_init_r function in gumstix sets board serial number */

#define CONFIG_BOOTFILE		boot/uImage
#define CONFIG_BOOTARGS		"console=ttyS0,115200n8 root=1f01 rootfstype=jffs2 reboot=cold,hard"
#define CONFIG_BOOTCOMMAND	"icache on; setenv stderr nulldev; setenv stdout nulldev; if pinit on && fatload ide 0 a2000000 gumstix-factory.script; then setenv stdout serial; setenv stderr serial; echo Found gumstix-factory.script on CF...; autoscr; else if mmcinit && fatload mmc 0 a2000000 gumstix-factory.script; then setenv stdout serial; setenv stderr serial; echo Found gumstix-factory.script on MMC...; autoscr; else setenv stdout serial; setenv stderr serial; katload 100000 && bootm; fi; fi"
#define CONFIG_BOOTDELAY	2		/* in seconds */
#define CONFIG_EXTRA_ENV_SETTINGS	"verify=no"
#define CFG_HUSH_PARSER
#define CFG_AUTO_COMPLETE
#define CFG_PROMPT_HUSH_PS2	"> "

/*
 * General options for u-boot. Modify to save memory foot print
 */

#define CFG_LONGHELP				/* undef saves memory		*/
#define CFG_PROMPT		"GUM> "		/* prompt string		*/
#define CFG_CBSIZE		512		/* console I/O buffer		*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* print buffer size	*/
#define CFG_MAXARGS		16		/* max command args		*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* boot args buf size		*/

#define CONFIG_CRC32_VERIFY
#define CONFIG_LOOPW
#define CONFIG_MX_CYCLIC

/*
 *  * SMSC91C111 Network Card
 *   */
#define CONFIG_DRIVER_SMC911X          1
#define CONFIG_SMC911X_BASE            0x04000000 /* chip select 1         */
#define CONFIG_NET_RETRY_COUNT          10         /* # of retries          */
#define CONFIG_ETHPRIME		"SMC911X"

/*
 * CF slot
 */

/*
 * The following are missing from pxa-regs.h
 */
#define CONFIG_PXA_PCMCIA 1
#define CONFIG_PXA_IDE 1
#define CONFIG_PCMCIA_SLOT_A 1

#define CFG_PCMCIA_IO_ADDR      0x20000000
#define CFG_PCMCIA_IO_SIZE      0x04000000
#define CFG_PCMCIA_DMA_ADDR     0x24000000
#define CFG_PCMCIA_DMA_SIZE     0x04000000
#define CFG_PCMCIA_ATTRB_ADDR   0x28000000
#define CFG_PCMCIA_ATTRB_SIZE   0x04000000
#define CFG_PCMCIA_MEM_ADDR     0x2c000000
#define CFG_PCMCIA_MEM_SIZE     0x04000000

#define CFG_MECR_VAL            0x00000002
#define CFG_MCMEM0_VAL          0x0002851f
#define CFG_MCMEM1_VAL          0x0002851f
#define CFG_MCATT0_VAL          0x0002851f
#define CFG_MCATT1_VAL          0x0002851f
#define CFG_MCIO0_VAL           0x00014291
#define CFG_MCIO1_VAL           0x00014291

#undef  CONFIG_IDE_PCCARD
#define CONFIG_IDE_PCMCIA       1
#undef  CONFIG_IDE_LED                  /* LED   for ide not supported  */
#undef  CONFIG_IDE_RESET                /* reset for ide not supported  */
#define CFG_IDE_MAXBUS          1       /* max. 1 IDE bus               */
#define CFG_IDE_MAXDEVICE       1       /* max. 1 drive per IDE bus     */
#define CFG_ATA_IDE0_OFFSET     0x0000
/* it's simple, all regs are in I/O space */
#define CFG_ATA_BASE_ADDR       CFG_PCMCIA_IO_ADDR
/* Offset for data I/O                  */
#define CFG_ATA_DATA_OFFSET     0x1f0
/* Offset for normal register accesses  */
#define CFG_ATA_REG_OFFSET      0x1f0
/* Offset for alternate registers       */
#define CFG_ATA_ALT_OFFSET      0x3f0


#define CFG_MEMTEST_START	0xa1000000	/* memtest test area		*/
#define CFG_MEMTEST_END		0xa2000000


#undef  CFG_CLKS_IN_HZ				/* use HZ for freq. display	*/
#define CFG_HZ			3686400		/* incrementer freq: 3.6864 MHz	*/
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 }

#define CFG_CONSOLE_INFO_QUIET

/*
 * Definitions related to passing arguments to kernel.
 */
#define CONFIG_CMDLINE_TAG		/* send commandline to Kernel		*/
#define CONFIG_SETUP_MEMORY_TAGS	/* send memory definition to kernel	*/
#define CONFIG_INITRD_TAG		/* send initrd params			*/
#define CONFIG_SERIAL_TAG		/* send serialnr from flash 		*/
#undef  CONFIG_VFD			/* do not send framebuffer setup	*/

/*
 * SDRAM Memory Map
 */
#define CFG_DRAM_BASE		0xa0000000
#define CONFIG_NR_DRAM_BANKS	4	/* Max number of DRAM banks		*/
/* Large 1GB model, MA<24:10>, Normal non-SA1110 addressing, tRP=3, CL=3, tRCD=3, tRAS=7, tRC=10, 4 banksx13x10, 32-bits */
#define MDCNFG_VAL_13_10 0x8AD00AD0
//#else
///* Large 1GB model, MA<24:10>, Normal non-SA1110 addressing, tRP=3, CL=3, tRCD=3, tRAS=7, tRC=10, 4 banksx13x9, 32-bits */
#define MDCNFG_VAL_13_9 0x8AC80AC8
//#endif
#define MDCNFG_VAL	MDCNFG_VAL_13_10

/*
 * Configuration for FLASH memory
 */
#define PHYS_FLASH_1		0x00000000	/* Flash Bank #1 */
#define PHYS_FLASH_SECT_SIZE	0x00020000	/* 128 KB sectors */

#define CFG_FLASH_BASE		PHYS_FLASH_1
#define CFG_FLASH_CFI				/* flash is CFI conformant	*/
#define CFG_FLASH_CFI_DRIVER			/* use common cfi driver	*/
#define CFG_FLASH_USE_BUFFER_WRITE 1		/* use buffered writes (20x faster) */
#define CFG_MAX_FLASH_BANKS	1		/* max # of memory banks	*/
#define CFG_MAX_FLASH_SECT	260		/* max # of sectors on one chip	*/
#define CFG_FLASH_PROTECTION			/* use hardware flash protection */

#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SIZE		0x1000		/* 4kB */
#define CFG_ENV_SECT_SIZE	PHYS_FLASH_SECT_SIZE
#define CFG_ENV_ADDR		((CFG_FLASH_BASE + CFG_ENV_SECT_SIZE*2) - CFG_ENV_SIZE)

/*
 * Malloc pool need to host env + 256 Kb reserve for other allocations.
 */
#define MALLOC_DRAM_BASE	0xA3F00000	/* malloc space is below this */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + (256 * 1024) )
#define CONFIG_STACKSIZE	(0)
#define CFG_GBL_DATA_SIZE	512		/* num bytes initial data       */

/*
 * This is setting for JFFS2 support in u-boot.
 * Right now there is no gain for user, but later on booting kernel might be
 * possible. Consider using XIP kernel running from flash to save RAM
 * footprint.
 * NOTE: Enable CFG_CMD_JFFS2 for JFFS2 support.
 */

#define CFG_JFFS2_FIRST_BANK	0
#define CFG_JFFS2_FIRST_SECTOR	2
#define CFG_JFFS2_NUM_BANKS	1

#define CONFIG_MMC		1
#define CFG_MMC_BASE		0xF0000000
#define CONFIG_DOS_PARTITION

/*
 * Environment setup. Definitions of monitor location and size with
 * definition of environment setup ends up in 2 possibilities.
 * 1. Embedded environment - in u-boot code is space for environment
 * 2. Environment is read from predefined sector of flash
 * Right now we support 2. possiblity, but expecting no env placed
 * on mentioned address right now. This also needs to provide whole
 * sector for it - for us 256Kb is really waste of memory. U-boot uses
 * default env. and until kernel parameters could be sent to kernel
 * env. has no sense to us.
 */

#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(PHYS_FLASH_SECT_SIZE*2 - CFG_ENV_SIZE)

#define CFG_GPSR0_VAL		0xFFFDEF1B
#define CFG_GPSR1_VAL		0xFFEFFFFF
#define CFG_GPSR2_VAL		0xFFFFFDFF
#define CFG_GPSR3_VAL		0x00000000

#define CFG_GPCR0_VAL		0x08023000
#define CFG_GPCR1_VAL		0x00100200
#define CFG_GPCR2_VAL		0x00010000
#define CFG_GPCR3_VAL		0x00000800

#define CFG_GPDR0_VAL		0xC1829218
#define CFG_GPDR1_VAL		0x0012AAA1
#define CFG_GPDR2_VAL		0x00010000
#define CFG_GPDR3_VAL		0x00014800

#define CFG_GAFR0_L_VAL		0x80040000
#define CFG_GAFR0_U_VAL		0xA5254010
#define CFG_GAFR1_L_VAL		0x69908010
#define CFG_GAFR1_U_VAL		0xAAA5A8AA
#define CFG_GAFR2_L_VAL		0xAAAAAAAA
#define CFG_GAFR2_U_VAL		0x00010000
#define CFG_GAFR3_L_VAL		0x10000000
#define CFG_GAFR3_U_VAL		0x00000001

#define CFG_PSSR_VAL		0x20

/*
 * Well this has to be defined, but on the other hand it is used differently
 * one may expect. For instance loadb command do not cares :-)
 * So advice is - do not rely on this...
 */

#define CFG_LOAD_ADDR		0xA2000000
#endif  /* __CONFIG_H */
