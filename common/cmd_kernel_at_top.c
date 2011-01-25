/*
 * Load kernel at offset from top of flash to RAM
 *
 * Based on u-boot mem routines
 */

#include "common.h"
#include "command.h"
#include "flash.h"

#if (CONFIG_COMMANDS & (CFG_CMD_KAT))

extern flash_info_t flash_info[];

int do_katload ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	*addr, *dest, count;
	char	*s;

	if (argc != 2 && argc != 3) {
		printf (" %d Usage:\n%s\n", argc, cmdtp->usage);
		return 1;
	}

	if(argc == 3) {
		dest = (ulong *)simple_strtoul(argv[2], NULL, 16);
	} else {
		if ((s = getenv("loadaddr")) != NULL) {
			dest = (ulong *)simple_strtoul(s, NULL, 16);
		} else {
			dest = CFG_LOAD_ADDR;
		}
	}

	count = simple_strtoul(argv[1], NULL, 16);

	addr = (ulong *)(flash_info[0].size - count);

	if (count == 0) {
		puts ("Zero length ???\n");
		return 1;
	}

	printf("Copying kernel to 0x%08x from 0x%08x (length 0x%08x)...", dest, addr, count);

	count /= 4;

	while (count-- > 0) {
		*(dest++) = *(addr++);
	}

	printf("done\n");
	return 0;
}

extern int flash_sect_erase (ulong addr_first, ulong addr_last);
int do_katinstall ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	*addr, *src, count;
	char	*s;

	if (argc !=2 && argc != 3) {
		printf ("%d Usage:\n%s\n", argc, cmdtp->usage);
		return 1;
	}

	if(argc == 3) {
		src = (ulong *)simple_strtoul(argv[2], NULL, 16);
	} else {
		if ((s = getenv("loadaddr")) != NULL) {
			src = (ulong *)simple_strtoul(s, NULL, 16);
		} else {
			src = CFG_LOAD_ADDR;
		}
	}

	count = simple_strtoul(argv[1], NULL, 16);

	addr = (ulong *)(flash_info[0].size - count);

	if (count == 0) {
		puts ("Zero length ???\n");
		return 1;
	}

	printf("Copying kernel from 0x%08x to 0x%08x (length 0x%08x)...", src, addr, count);

	printf("Erasing...");
	flash_sect_erase(addr,flash_info[0].size-1);
	printf("Writing...");
	flash_write(src, addr, count);

	printf("done\n");
	return 0;
}

/**************************************************/
U_BOOT_CMD(
	katload,     3,     0,      do_katload,
	"katload      - Load kernel to RAM from offset at top of flash\n",
	" offset [address]\n    - Kernel image is at (TOP_OF_FLASH - offset) and will be copied to address\n"
);

U_BOOT_CMD(
	katinstall,     3,     0,      do_katinstall,
	"katinstall      - Load kernel to RAM from offset at top of flash\n",
	" offset [address]\n    - Kernel image is at address in RAM and will install to (TOP_OF_FLASH - offset)\n"
);

#endif	/* CFG_CMD_KAT */
