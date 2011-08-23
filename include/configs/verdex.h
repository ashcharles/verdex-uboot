/*
 * Gumstix Verdex (PXA270) configuration file
 *
 * Copyright (C) 2011 Ash Charles <ashcharles@gmail.com>
 *     Based off vpac270.h
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef	__CONFIG_H
#define	__CONFIG_H

/*
 * High Level Board Configuration Options
 */
#define	CONFIG_PXA27X		1	/* Marvell PXA270 CPU */
#define	CONFIG_VERDEX		1	/* Gumstix Verdex board */
#define	CONFIG_SYS_TEXT_BASE	0x0

/*
 * Environment settings
 */
#define	CONFIG_ENV_OVERWRITE
#define	CONFIG_SYS_MALLOC_LEN		(128*1024)
#define	CONFIG_ARCH_CPU_INIT
#define	CONFIG_BOOTCOMMAND						\
	"if mmc init && fatload mmc 0 0xa2000000 uImage; then "		\
		"bootm 0xa2000000; "					\
	"fi; "								\
	"if ide reset && fatload ide 0 0xa2000000 uImage; then "	\
		"bootm 0xa2000000; "					\
	"fi; "								\
	"bootm 0x40000;"
#define	CONFIG_BOOTARGS			"console=tty0 console=ttyS0,115200 root=1f01 rootfstype=jffs2 reboot=cold,hard"
#define	CONFIG_TIMESTAMP
#define	CONFIG_BOOTDELAY		2	/* Autoboot delay */
#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS

/*
 * Serial Console Configuration
 */
#define	CONFIG_PXA_SERIAL
#define	CONFIG_FFUART			1
#define	CONFIG_BAUDRATE			115200
#define	CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 }

/*
 * Bootloader Components Configuration
 */
#include <config_cmd_default.h>

#undef	CONFIG_CMD_FPGA
#undef	CONFIG_CMD_IMLS
#undef	CONFIG_CMD_SETGETDCR
#undef	CONFIG_CMD_XIMG

#define	CONFIG_CMD_ASKENV
#define	CONFIG_CMD_CACHE
#define	CONFIG_CMD_DIAG
#define	CONFIG_CMD_ELF
#define	CONFIG_CMD_ENV
#define	CONFIG_CMD_FAT
#define	CONFIG_CMD_IDE
#define	CONFIG_CMD_MMC
/*#define	CONFIG_CMD_PCMCIA*/
#define	CONFIG_CMD_REGINFO
#define	CONFIG_CMD_SAVES
#define	CONFIG_CMD_SDRAM

/*
 * Networking Configuration
 *  chip on the Gumstix Verdex expansion boards (netpro et al.)
 */
#ifdef	CONFIG_CMD_NET
#define	CONFIG_CMD_PING
#define	CONFIG_CMD_DHCP

#define	CONFIG_NET_MULTI		1
#define CONFIG_SMC911X			1
#define CONFIG_SMC911X_32_BIT		1
#define	CONFIG_SMC911X_BASE		0x04000000	/* CS1 */
#define CONFIG_NET_RETRY_COUNT		10
#define CONFIG_ETHPRIME			"SMC911X"
#endif

/*
 * PCMCIA Bus Configuration
 */
#ifdef	CONFIG_CMD_PCMCIA
#define CONFIG_PCMCIA_SLOT_A

#define CONFIG_SYS_PCMCIA_IO_ADDR       (0x20000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE       ( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR      (0x24000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE      ( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR    (0x28000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE    ( 64 << 20 )
#define CONFIG_SYS_PCMCIA_MEM_ADDR      (0x2C000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE      ( 64 << 20 )
#endif

/*
 * MMC Card Configuration
 */
#ifdef	CONFIG_CMD_MMC
#define	CONFIG_MMC
#define	CONFIG_PXA_MMC
#define	CONFIG_SYS_MMC_BASE		0xF0000000
#define	CONFIG_DOS_PARTITION
#endif

/*
 * HUSH Shell Configuration
 */
#define	CONFIG_SYS_HUSH_PARSER		1
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define	CONFIG_SYS_LONGHELP
#ifdef	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT		"GUM> "
#else
#define	CONFIG_SYS_PROMPT		"=> "
#endif
#define	CONFIG_SYS_CBSIZE		512
#define	CONFIG_SYS_PBSIZE		\
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define	CONFIG_SYS_MAXARGS		16
#define	CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define	CONFIG_SYS_DEVICE_NULLDEV	1

/*
 * Clock Configuration
 */
#undef	CONFIG_SYS_CLKS_IN_HZ
#define	CONFIG_SYS_HZ			3686400 	/* Timer @ 3250000 Hz */
#define	CONFIG_SYS_CPUSPEED		0x310		/* 624 MHz */

/*
 * Stack sizes
 */
#define	CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#ifdef	CONFIG_USE_IRQ
#define	CONFIG_STACKSIZE_IRQ		(4*1024)	/* IRQ stack */
#define	CONFIG_STACKSIZE_FIQ		(4*1024)	/* FIQ stack */
#endif

/*
 * DRAM Map
 */
#define	CONFIG_NR_DRAM_BANKS		1		/* 4 banks of DRAM */
#define	PHYS_SDRAM_1			0xA0000000	/* SDRAM Bank #1 */
#define	PHYS_SDRAM_1_SIZE		0x08000000	/* 32 MB */

#define	CONFIG_SYS_DRAM_BASE		0xA0000000	/* CS0 */
#define	CONFIG_SYS_DRAM_SIZE		0x08000000	/* 128 MB DRAM */

#define	CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on */
#define	CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM */

#define	CONFIG_SYS_LOAD_ADDR		PHYS_SDRAM_1
#define	CONFIG_SYS_IPL_LOAD_ADDR	(0x5c000000)
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		\
	(PHYS_SDRAM_1 + GENERATED_GBL_DATA_SIZE + 2048)

/*
 * NOR FLASH
 */
#define	CONFIG_SYS_MONITOR_BASE		0x0
#define	CONFIG_SYS_MONITOR_LEN		0x40000
#define	CONFIG_ENV_ADDR			\
			(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#define	CONFIG_ENV_SIZE			0x4000

#define	PHYS_FLASH_1			0x00000000	/* Flash Bank #1 */

#define	CONFIG_SYS_FLASH_CFI
#define	CONFIG_FLASH_CFI_DRIVER		1

#define	CONFIG_SYS_MAX_FLASH_SECT	(4 + 255)
#ifdef	CONFIG_RAM_256M
#define	CONFIG_SYS_MAX_FLASH_BANKS	2
#define	CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1, PHYS_FLASH_2 }
#else
#define	CONFIG_SYS_MAX_FLASH_BANKS	1
#define	CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#endif

#define	CONFIG_SYS_FLASH_ERASE_TOUT	(25*CONFIG_SYS_HZ)
#define	CONFIG_SYS_FLASH_WRITE_TOUT	(25*CONFIG_SYS_HZ)

#define	CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
#define	CONFIG_SYS_FLASH_PROTECTION		1

#define	CONFIG_ENV_IS_IN_FLASH		1
#define	CONFIG_ENV_SECT_SIZE		0x20000

/*
 * IDE
 */
#ifdef	CONFIG_CMD_IDE
#undef	CONFIG_IDE_LED
#undef	CONFIG_IDE_RESET

#define	__io

#define	CONFIG_SYS_IDE_MAXBUS		1
#define	CONFIG_SYS_IDE_MAXDEVICE	1

#define	CONFIG_SYS_ATA_BASE_ADDR	0x0c000000
#define	CONFIG_SYS_ATA_IDE0_OFFSET	0x0

#define	CONFIG_SYS_ATA_DATA_OFFSET	0x120
#define	CONFIG_SYS_ATA_REG_OFFSET	0x120
#define	CONFIG_SYS_ATA_ALT_OFFSET	0x120

#define	CONFIG_SYS_ATA_STRIDE		2
#endif

/*
 * GPIO settings
 */
#define	CONFIG_SYS_GPSR0_VAL	0xFFFDEF1B
#define	CONFIG_SYS_GPSR1_VAL	0xFFEFFFFF
#define	CONFIG_SYS_GPSR2_VAL	0xFFFFFDFF
#define	CONFIG_SYS_GPSR3_VAL	0x00000000

#define	CONFIG_SYS_GPCR0_VAL	0x08023000
#define	CONFIG_SYS_GPCR1_VAL	0x00100200
#define	CONFIG_SYS_GPCR2_VAL	0x00010000
#define	CONFIG_SYS_GPCR3_VAL	0x00000800

#define	CONFIG_SYS_GPDR0_VAL	0xC1829218
#define	CONFIG_SYS_GPDR1_VAL	0x0012AAA1
#define	CONFIG_SYS_GPDR2_VAL	0x00010000
#define	CONFIG_SYS_GPDR3_VAL	0x00014800

#define	CONFIG_SYS_GAFR0_L_VAL	0x80040000
#define	CONFIG_SYS_GAFR0_U_VAL	0xA5254010
#define	CONFIG_SYS_GAFR1_L_VAL	0x69908010
#define	CONFIG_SYS_GAFR1_U_VAL	0xAAA5A8AA
#define	CONFIG_SYS_GAFR2_L_VAL	0xAAAAAAAA
#define	CONFIG_SYS_GAFR2_U_VAL	0x00010000
#define	CONFIG_SYS_GAFR3_L_VAL	0x10000000
#define	CONFIG_SYS_GAFR3_U_VAL	0x00000001

#define	CONFIG_SYS_PSSR_VAL	0x20

/*
 * Clock settings
 */
#define	CONFIG_SYS_CCCR		0x00000310
#define	CONFIG_SYS_CKEN		0x00FFFFF0

/*
 * Memory settings
 */
#define	CONFIG_SYS_MDCNFG_VAL	0x8AD00AD0
#define	CONFIG_SYS_MDREFR_VAL	0x2015C001
#define	CONFIG_SYS_MSC0_VAL	0x033c35d8
#define	CONFIG_SYS_MSC1_VAL	0x033c35d8
#define	CONFIG_SYS_MSC2_VAL	0x033c35d8
#define	CONFIG_SYS_MDMRS_VAL	0x00320032
#define	CONFIG_SYS_FLYCNFG_VAL	0x00000000
#define	CONFIG_SYS_SXCNFG_VAL	0x40044004

/*
 * PCMCIA and CF Interfaces
 */
#define	CONFIG_SYS_MECR_VAL	0x00000002
#define	CONFIG_SYS_MCMEM0_VAL	0x0002851f
#define	CONFIG_SYS_MCMEM1_VAL	0x0002851f
#define	CONFIG_SYS_MCATT0_VAL	0x0002851f
#define	CONFIG_SYS_MCATT1_VAL	0x0002851f
#define	CONFIG_SYS_MCIO0_VAL	0x00014291
#define	CONFIG_SYS_MCIO1_VAL	0x00014291

#endif	/* __CONFIG_H */
