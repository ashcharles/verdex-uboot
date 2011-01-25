/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <command.h>
#include <malloc.h>
#include <image.h>
#include <asm/byteorder.h>
#include <usb.h>

#ifdef CFG_HUSH_PARSER
#include <hush.h>
#endif


#ifdef CONFIG_AUTO_UPDATE

#ifndef CONFIG_USB_OHCI
#error "must define CONFIG_USB_OHCI"
#endif

#ifndef CONFIG_USB_STORAGE
#error "must define CONFIG_USB_STORAGE"
#endif

#ifndef CFG_HUSH_PARSER
#error "must define CFG_HUSH_PARSER"
#endif

#if !(CONFIG_COMMANDS & CFG_CMD_FAT)
#error "must define CFG_CMD_FAT"
#endif

/*
 * Check whether a USB memory stick is plugged in.
 * If one is found:
 *	1) if prepare.img ist found load it into memory. If it is
 *		valid then run it.
 *	2) if preinst.img is found load it into memory. If it is
 *		valid then run it. Update the EEPROM.
 *	3) if firmw_01.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	4) if kernl_01.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	5) if app.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	6) if disk.img is found load it into memory. If it is valid,
 *		burn it into FLASH and update the EEPROM.
 *	7) if postinst.img is found load it into memory. If it is
 *		valid then run it. Update the EEPROM.
 */

#undef AU_DEBUG

#undef debug
#ifdef	AU_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif	/* AU_DEBUG */

/* possible names of files on the USB stick. */
#define AU_FIRMWARE	"u-boot.img"
#define AU_KERNEL	"kernel.img"

struct flash_layout {
	long start;
	long end;
};

/* layout of the FLASH. ST = start address, ND = end address. */
#define AU_FL_FIRMWARE_ST	0xfC000000
#define AU_FL_FIRMWARE_ND	0xfC03FFFF
#define AU_FL_KERNEL_ST		0xfC0C0000
#define AU_FL_KERNEL_ND		0xfC1BFFFF

static int au_usb_stor_curr_dev; /* current device */

/* index of each file in the following arrays */
#define IDX_FIRMWARE	0
#define IDX_KERNEL	1

/* max. number of files which could interest us */
#define AU_MAXFILES 2

/* pointers to file names */
char *aufile[AU_MAXFILES];

/* sizes of flash areas for each file */
long ausize[AU_MAXFILES];

/* array of flash areas start and end addresses */
struct flash_layout aufl_layout[AU_MAXFILES] = { \
	{AU_FL_FIRMWARE_ST, AU_FL_FIRMWARE_ND,}, \
	{AU_FL_KERNEL_ST, AU_FL_KERNEL_ND,}, \
};

/* where to load files into memory */
#define LOAD_ADDR ((unsigned char *)0x00200000)

/* the app is the largest image */
#define MAX_LOADSZ ausize[IDX_KERNEL]

/*i2c address of the keypad status*/
#define I2C_PSOC_KEYPAD_ADDR	0x53

/* keypad mask */
#define KEYPAD_ROW	3
#define KEYPAD_COL	3
#define KEYPAD_MASK_LO	((1<<(KEYPAD_COL-1+(KEYPAD_ROW*4-4)))&0xFF)
#define KEYPAD_MASK_HI	((1<<(KEYPAD_COL-1+(KEYPAD_ROW*4-4)))>>8)

/* externals */
extern int fat_register_device(block_dev_desc_t *, int);
extern int file_fat_detectfs(void);
extern long file_fat_read(const char *, void *, unsigned long);
extern int i2c_read (unsigned char, unsigned int, int , unsigned char* , int);
extern int flash_sect_erase(ulong, ulong);
extern int flash_sect_protect (int, ulong, ulong);
extern int flash_write (char *, ulong, ulong);
/* change char* to void* to shutup the compiler */
extern block_dev_desc_t *get_dev (char*, int);
extern int u_boot_hush_start(void);

int au_check_cksum_valid(int idx, long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;

	hdr = (image_header_t *)LOAD_ADDR;

	if (nbytes != (sizeof(*hdr) + ntohl(hdr->ih_size))) {
		printf ("Image %s bad total SIZE\n", aufile[idx]);
		return -1;
	}
	/* check the data CRC */
	checksum = ntohl(hdr->ih_dcrc);

	if (crc32 (0, (uchar *)(LOAD_ADDR + sizeof(*hdr)), ntohl(hdr->ih_size)) != checksum) {
		printf ("Image %s bad data checksum\n", aufile[idx]);
		return -1;
	}
	return 0;
}

int au_check_header_valid(int idx, long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;
	unsigned char buf[4];

	hdr = (image_header_t *)LOAD_ADDR;
	/* check the easy ones first */
#undef CHECK_VALID_DEBUG
#ifdef CHECK_VALID_DEBUG
	printf("magic %#x %#x ", ntohl(hdr->ih_magic), IH_MAGIC);
	printf("arch %#x %#x ", hdr->ih_arch, IH_CPU_ARM);
	printf("size %#x %#lx ", ntohl(hdr->ih_size), nbytes);
	printf("type %#x %#x ", hdr->ih_type, IH_TYPE_KERNEL);
#endif
	if (nbytes < sizeof(*hdr)) {
		printf ("Image %s bad header SIZE\n", aufile[idx]);
		return -1;
	}
	if (ntohl(hdr->ih_magic) != IH_MAGIC || hdr->ih_arch != IH_CPU_PPC) {
		printf ("Image %s bad MAGIC or ARCH\n", aufile[idx]);
		return -1;
	}
	/* check the hdr CRC */
	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;

	if (crc32 (0, (uchar *)hdr, sizeof(*hdr)) != checksum) {
		printf ("Image %s bad header checksum\n", aufile[idx]);
		return -1;
	}
	hdr->ih_hcrc = htonl(checksum);
	/* check the type - could do this all in one gigantic if() */
	if ((idx == IDX_FIRMWARE) && (hdr->ih_type != IH_TYPE_FIRMWARE)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		return -1;
	}
	if ((idx == IDX_KERNEL) && (hdr->ih_type != IH_TYPE_KERNEL)) {
		printf ("Image %s wrong type\n", aufile[idx]);
		return -1;
	}
	/* recycle checksum */
	checksum = ntohl(hdr->ih_size);
	/* for kernel and app the image header must also fit into flash */
	if (idx != IDX_FIRMWARE)
		checksum += sizeof(*hdr);
	/* check the size does not exceed space in flash. HUSH scripts */
	/* all have ausize[] set to 0 */
	if ((ausize[idx] != 0) && (ausize[idx] < checksum)) {
		printf ("Image %s is bigger than FLASH\n", aufile[idx]);
		return -1;
	}
	return 0;
}

int au_do_update(int idx, long sz)
{
	image_header_t *hdr;
	char *addr;
	long start, end;
	int off, rc;
	uint nbytes;

	hdr = (image_header_t *)LOAD_ADDR;

	/* execute a script */
	if (hdr->ih_type == IH_TYPE_SCRIPT) {
		addr = (char *)((char *)hdr + sizeof(*hdr));
		/* stick a NULL at the end of the script, otherwise */
		/* parse_string_outer() runs off the end. */
		addr[ntohl(hdr->ih_size)] = 0;
		addr += 8;
		parse_string_outer(addr, FLAG_PARSE_SEMICOLON);
		return 0;
	}

	start = aufl_layout[idx].start;
	end = aufl_layout[idx].end;

	/* unprotect the address range */
	/* this assumes that ONLY the firmware is protected! */
	if (idx == IDX_FIRMWARE) {
#undef AU_UPDATE_TEST
#ifdef AU_UPDATE_TEST
		/* erase it where Linux goes */
		start = aufl_layout[1].start;
		end = aufl_layout[1].end;
#endif
		flash_sect_protect(0, start, end);
	}

	/*
	 * erase the address range.
	 */
	debug ("flash_sect_erase(%lx, %lx);\n", start, end);
	flash_sect_erase(start, end);
	wait_ms(100);
	/* strip the header - except for the kernel and ramdisk */
	if (hdr->ih_type == IH_TYPE_KERNEL) {
		addr = (char *)hdr;
		off = sizeof(*hdr);
		nbytes = sizeof(*hdr) + ntohl(hdr->ih_size);
	} else {
		addr = (char *)((char *)hdr + sizeof(*hdr));
#ifdef AU_UPDATE_TEST
		/* copy it to where Linux goes */
		if (idx == IDX_FIRMWARE)
			start = aufl_layout[1].start;
#endif
		off = 0;
		nbytes = ntohl(hdr->ih_size);
	}

	/* copy the data from RAM to FLASH */
	debug ("flash_write(%p, %lx %x)\n", addr, start, nbytes);
	rc = flash_write(addr, start, nbytes);
	if (rc != 0) {
		printf("Flashing failed due to error %d\n", rc);
		return -1;
	}

	/* check the dcrc of the copy */
	if (crc32 (0, (uchar *)(start + off), ntohl(hdr->ih_size)) != ntohl(hdr->ih_dcrc)) {
		printf ("Image %s Bad Data Checksum After COPY\n", aufile[idx]);
		return -1;
	}

	/* protect the address range */
	/* this assumes that ONLY the firmware is protected! */
	if (idx == IDX_FIRMWARE)
		flash_sect_protect(1, start, end);
	return 0;
}

/*
 * this is called from board_init() after the hardware has been set up
 * and is usable. That seems like a good time to do this.
 * Right now the return value is ignored.
 */
int do_auto_update(void)
{
	block_dev_desc_t *stor_dev;
	long sz;
	int i, res, bitmap_first, cnt, old_ctrlc, got_ctrlc;
	char *env;
	long start, end;
	char keypad_status1[2] = {0,0}, keypad_status2[2] = {0,0};

	/*
	 * Read keypad status
	 */
	i2c_read(I2C_PSOC_KEYPAD_ADDR, 0, 0, keypad_status1, 2);
	wait_ms(500);
	i2c_read(I2C_PSOC_KEYPAD_ADDR, 0, 0, keypad_status2, 2);

	/*
	 * Check keypad
	 */
	if ( !(keypad_status1[0] & KEYPAD_MASK_HI) ||
	      (keypad_status1[0] != keypad_status2[0])) {
		return 0;
	}
	if ( !(keypad_status1[1] & KEYPAD_MASK_LO) ||
	      (keypad_status1[1] != keypad_status2[1])) {
		return 0;
	}
	au_usb_stor_curr_dev = -1;
	/* start USB */
	if (usb_stop() < 0) {
		debug ("usb_stop failed\n");
		return -1;
	}
	if (usb_init() < 0) {
		debug ("usb_init failed\n");
		return -1;
	}
	/*
	 * check whether a storage device is attached (assume that it's
	 * a USB memory stick, since nothing else should be attached).
	 */
	au_usb_stor_curr_dev = usb_stor_scan(0);
	if (au_usb_stor_curr_dev == -1) {
		debug ("No device found. Not initialized?\n");
		return -1;
	}
	/* check whether it has a partition table */
	stor_dev = get_dev("usb", 0);
	if (stor_dev == NULL) {
		debug ("uknown device type\n");
		return -1;
	}
	if (fat_register_device(stor_dev, 1) != 0) {
		debug ("Unable to use USB %d:%d for fatls\n",
			au_usb_stor_curr_dev, 1);
		return -1;
	}
	if (file_fat_detectfs() != 0) {
		debug ("file_fat_detectfs failed\n");
	}

	/* initialize the array of file names */
	memset(aufile, 0, sizeof(aufile));
	aufile[IDX_FIRMWARE] = AU_FIRMWARE;
	aufile[IDX_KERNEL] = AU_KERNEL;
	/* initialize the array of flash sizes */
	memset(ausize, 0, sizeof(ausize));
	ausize[IDX_FIRMWARE] = (AU_FL_FIRMWARE_ND + 1) - AU_FL_FIRMWARE_ST;
	ausize[IDX_KERNEL] = (AU_FL_KERNEL_ND + 1) - AU_FL_KERNEL_ST;
	/*
	 * now check whether start and end are defined using environment
	 * variables.
	 */
	start = -1;
	end = 0;
	env = getenv("firmware_st");
	if (env != NULL)
		start = simple_strtoul(env, NULL, 16);
	env = getenv("firmware_nd");
	if (env != NULL)
		end = simple_strtoul(env, NULL, 16);
	if (start >= 0 && end && end > start) {
		ausize[IDX_FIRMWARE] = (end + 1) - start;
		aufl_layout[0].start = start;
		aufl_layout[0].end = end;
	}
	start = -1;
	end = 0;
	env = getenv("kernel_st");
	if (env != NULL)
		start = simple_strtoul(env, NULL, 16);
	env = getenv("kernel_nd");
	if (env != NULL)
		end = simple_strtoul(env, NULL, 16);
	if (start >= 0 && end && end > start) {
		ausize[IDX_KERNEL] = (end + 1) - start;
		aufl_layout[1].start = start;
		aufl_layout[1].end = end;
	}
	/* make certain that HUSH is runnable */
	u_boot_hush_start();
	/* make sure that we see CTRL-C and save the old state */
	old_ctrlc = disable_ctrlc(0);

	bitmap_first = 0;
	/* just loop thru all the possible files */
	for (i = 0; i < AU_MAXFILES; i++) {
		/* just read the header */
		sz = file_fat_read(aufile[i], LOAD_ADDR, sizeof(image_header_t));
		debug ("read %s sz %ld hdr %d\n",
			aufile[i], sz, sizeof(image_header_t));
		if (sz <= 0 || sz < sizeof(image_header_t)) {
			debug ("%s not found\n", aufile[i]);
			continue;
		}
		if (au_check_header_valid(i, sz) < 0) {
			debug ("%s header not valid\n", aufile[i]);
			continue;
		}
		sz = file_fat_read(aufile[i], LOAD_ADDR, MAX_LOADSZ);
		debug ("read %s sz %ld hdr %d\n",
			aufile[i], sz, sizeof(image_header_t));
		if (sz <= 0 || sz <= sizeof(image_header_t)) {
			debug ("%s not found\n", aufile[i]);
			continue;
		}
		if (au_check_cksum_valid(i, sz) < 0) {
			debug ("%s checksum not valid\n", aufile[i]);
			continue;
		}
		/* this is really not a good idea, but it's what the */
		/* customer wants. */
		cnt = 0;
		got_ctrlc = 0;
		do {
			res = au_do_update(i, sz);
			/* let the user break out of the loop */
			if (ctrlc() || had_ctrlc()) {
				clear_ctrlc();
				if (res < 0)
					got_ctrlc = 1;
				break;
			}
			cnt++;
#ifdef AU_TEST_ONLY
		} while (res < 0 && cnt < 3);
		if (cnt < 3)
#else
		} while (res < 0);
#endif
	}
	usb_stop();
	/* restore the old state */
	disable_ctrlc(old_ctrlc);
	return 0;
}
#endif /* CONFIG_AUTO_UPDATE */
