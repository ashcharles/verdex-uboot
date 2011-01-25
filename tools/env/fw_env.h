/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * To build the utility with the run-time configuration
 * uncomment the next line.
 * See included "fw_env.config" sample file (TRAB board)
 * for notes on configuration.
 */
//#define CONFIG_FILE     "/etc/fw_env.config"

//#define HAVE_REDUND /* For systems with 2 env sectors */
#define DEVICE1_NAME      "/dev/mtd0"
//#define DEVICE2_NAME      "/dev/mtd2"
#define DEVICE1_OFFSET    CFG_ENV_ADDR
#define ENV1_SIZE         CFG_ENV_SIZE
#define DEVICE1_ESIZE     CFG_ENV_SECT_SIZE
//#define DEVICE2_OFFSET    0x0000
//#define ENV2_SIZE         0x4000
//#define DEVICE2_ESIZE     0x4000

extern		void  fw_printenv(int argc, char *argv[]);
extern unsigned char *fw_getenv  (unsigned char *name);
extern		int   fw_setenv  (int argc, char *argv[]);

extern unsigned	long  crc32	 (unsigned long, const unsigned char *, unsigned);
