/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * Configuration settings for the ATSTK1002 CPU daughterboard
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
#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_AVR32			1
#define CONFIG_AT32AP			1
#define CONFIG_AT32AP7000		1
#define CONFIG_ATSTK1002		1
#define CONFIG_ATSTK1000		1

#define CONFIG_ATSTK1000_EXT_FLASH	1

/*
 * Timer clock frequency. We're using the CPU-internal COUNT register
 * for this, so this is equivalent to the CPU core clock frequency
 */
#define CFG_HZ				1000

/*
 * Set up the PLL to run at 199.5 MHz, the CPU to run at 1/2 the PLL
 * frequency and the peripherals to run at 1/4 the PLL frequency.
 */
#define CONFIG_PLL			1
#define CFG_POWER_MANAGER		1
#define CFG_OSC0_HZ			20000000
#define CFG_PLL0_DIV			1
#define CFG_PLL0_MUL			7
#define CFG_PLL0_SUPPRESS_CYCLES	16
#define CFG_CLKDIV_CPU			0
#define CFG_CLKDIV_HSB			1
#define CFG_CLKDIV_PBA			2
#define CFG_CLKDIV_PBB			1

/*
 * The PLLOPT register controls the PLL like this:
 *   icp = PLLOPT<2>
 *   ivco = PLLOPT<1:0>
 *
 * We want icp=1 (default) and ivco=0 (80-160 MHz) or ivco=2 (150-240MHz).
 */
#define CFG_PLL0_OPT			0x04

#define CFG_USART1			1

#define CFG_CONSOLE_UART_DEV		DEVICE_USART1

/* User serviceable stuff */
#define CONFIG_CMDLINE_TAG		1
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#define CONFIG_STACKSIZE		(2048)

#define CONFIG_BAUDRATE			115200
#define CONFIG_BOOTARGS							\
	"console=ttyUS0 root=/dev/mtdblock1 fbmem=600k"

#define CONFIG_COMMANDS			(CFG_CMD_BDI			\
					 | CFG_CMD_LOADS		\
					 | CFG_CMD_LOADB		\
					 /* | CFG_CMD_IMI */		\
					 /* | CFG_CMD_CACHE */		\
					 | CFG_CMD_FLASH		\
					 | CFG_CMD_MEMORY		\
					 /* | CFG_CMD_NET */		\
					 | CFG_CMD_ENV			\
					 /* | CFG_CMD_IRQ */		\
					 | CFG_CMD_BOOTD		\
					 | CFG_CMD_CONSOLE		\
					 /* | CFG_CMD_EEPROM */		\
					 | CFG_CMD_ASKENV		\
					 | CFG_CMD_RUN			\
					 | CFG_CMD_ECHO			\
					 /* | CFG_CMD_I2C */		\
					 | CFG_CMD_REGINFO		\
					 /* | CFG_CMD_DATE */		\
					 /* | CFG_CMD_DHCP */		\
					 /* | CFG_CMD_AUTOSCRIPT */	\
					 /* | CFG_CMD_MII */		\
					 | CFG_CMD_MISC			\
					 /* | CFG_CMD_SDRAM */		\
					 /* | CFG_CMD_DIAG */		\
					 /* | CFG_CMD_HWFLOW */		\
					 /* | CFG_CMD_SAVES */		\
					 /* | CFG_CMD_SPI */		\
					 /* | CFG_CMD_PING */		\
					 /* | CFG_CMD_MMC */		\
					 /* | CFG_CMD_FAT */		\
					 /* | CFG_CMD_IMLS */		\
					 /* | CFG_CMD_ITEST */		\
					 /* | CFG_CMD_EXT2 */		\
		)

#include <cmd_confdefs.h>

#define CONFIG_ATMEL_USART		1
#define CONFIG_PIO2			1
#define CFG_NR_PIOS			5
#define CFG_HSDRAMC			1

#define CFG_DCACHE_LINESZ		32
#define CFG_ICACHE_LINESZ		32

#define CONFIG_NR_DRAM_BANKS		1

/* External flash on STK1000 */
#if 0
#define CFG_FLASH_CFI			1
#define CFG_FLASH_CFI_DRIVER		1
#endif

#define CFG_FLASH_BASE			0x00000000
#define CFG_FLASH_SIZE			0x800000
#define CFG_MAX_FLASH_BANKS		1
#define CFG_MAX_FLASH_SECT		135

#define CFG_MONITOR_BASE		CFG_FLASH_BASE

#define CFG_INTRAM_BASE			0x24000000
#define CFG_INTRAM_SIZE			0x8000

#define CFG_SDRAM_BASE			0x10000000

#define CFG_ENV_IS_IN_FLASH		1
#define CFG_ENV_SIZE			65536
#define CFG_ENV_ADDR			(CFG_FLASH_BASE + CFG_FLASH_SIZE - CFG_ENV_SIZE)

#define CFG_INIT_SP_ADDR		(CFG_INTRAM_BASE + CFG_INTRAM_SIZE)

#define CFG_MALLOC_LEN			(256*1024)
#define CFG_MALLOC_END							\
	({								\
		DECLARE_GLOBAL_DATA_PTR;				\
		CFG_SDRAM_BASE + gd->sdram_size;			\
	})
#define CFG_MALLOC_START		(CFG_MALLOC_END - CFG_MALLOC_LEN)

#define CFG_DMA_ALLOC_LEN		(16384)
#define CFG_DMA_ALLOC_END		(CFG_MALLOC_START)
#define CFG_DMA_ALLOC_START		(CFG_DMA_ALLOC_END - CFG_DMA_ALLOC_LEN)
/* Allow 2MB for the kernel run-time image */
#define CFG_LOAD_ADDR			(CFG_SDRAM_BASE + 0x00200000)
#define CFG_BOOTPARAMS_LEN		(16 * 1024)

/* Other configuration settings that shouldn't have to change all that often */
#define CFG_PROMPT			"Uboot> "
#define CFG_CBSIZE			256
#define CFG_MAXARGS			8
#define CFG_PBSIZE			(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_LONGHELP			1

#define CFG_MEMTEST_START						\
	({ DECLARE_GLOBAL_DATA_PTR; gd->bd->bi_dram[0].start; })
#define CFG_MEMTEST_END							\
	({								\
		DECLARE_GLOBAL_DATA_PTR;				\
		gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size;	\
	})
#define CFG_BAUDRATE_TABLE { 115200, 38400, 19200, 9600, 2400 }

#endif /* __CONFIG_H */
