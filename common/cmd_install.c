/*
 * (C) Copyright 2005
 * Craig Hughes, Gumstix Inc. <craig@gumstix.com>
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
 * Install command to copy compiled-in binary to flash
 *
 */

#include <common.h>
#include <command.h>
#if (CONFIG_COMMANDS & CFG_CMD_INSTALL)

#ifdef CONFIG_GUMSTIX_CPUSPEED_400
#include <u-boot-400.h>
#else
#include <u-boot-200.h>
#endif

int do_install ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rc;
	size_t sect_top;

	if (argc != 1) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	for(sect_top=0; sect_top<sizeof(u_boot_bin_data); sect_top+=PHYS_FLASH_SECT_SIZE) continue;
	sect_top--;
	flash_sect_protect(0, CFG_FLASH_BASE, sect_top);
	flash_sect_erase(CFG_FLASH_BASE, sect_top);

	puts ("Copying to Flash... ");

	rc = flash_write ((uchar *)u_boot_bin_data, CFG_FLASH_BASE, sizeof(u_boot_bin_data));
	if (rc != 0) {
		flash_perror (rc);
		return (1);
	}
	puts ("done\n");
	return 0;
}


/**************************************************/
U_BOOT_CMD(
	install,    1,    1,    do_install,
	"install  - install u-boot to flash\n",
	"copies a u-boot image to begining of flash\n"
);

#endif	/* CFG_CMD_INSTALL */
