/*
 * Copyright (C) 2004 Gumstix, Inc.
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
 * Written by Craig Hughes <craig@gumstix.com>
 *
 */

#include <common.h>
#include <asm/arch/pxa-regs.h>

int board_init( void ){
  DECLARE_GLOBAL_DATA_PTR;

  gd->bd->bi_arch_number = 373;	
  gd->bd->bi_boot_params = 0xa0000100;

  return 0;
}

#ifdef CONFIG_PXA27X
static volatile uint32_t *partition_starts[] = {
					0xA0000000,
					0xB0000000,
					0x80000000,
					0x90000000,
				};
static uint32_t partition_probe_max_size = (256*1024*1024);
#else
static volatile uint32_t *partition_starts[] = {
					0xA0000000,
					0xA4000000,
					0xA8000000,
					0xAC000000,
				};
static uint32_t partition_probe_max_size = (64*1024*1024);
#endif

int dram_init( void ){
	DECLARE_GLOBAL_DATA_PTR;

	unsigned i;

	for(i=0; i<CONFIG_NR_DRAM_BANKS; i++)
	{
		uint32_t test_size = 16*1024*1024;

		// First probe if the partition exists at all.  If it doesn't exist, then writes will not take
		debug("Setting %p to %#08x\n",partition_starts[i], 0xdeadbeef);
		*partition_starts[i] = 0xDEADBEEF;
		if(0xDEADBEEF != *partition_starts[i])
		{
			debug("Disabling partition %d after failed to read deadbeef\n",i);
			// Disable partition
			gd->bd->bi_dram[i].start = gd->bd->bi_dram[i].size = 0;
			continue;
		}

		// If we have a valid partition, set its start point
		gd->bd->bi_dram[i].start = partition_starts[i];

		// Now probe for partition size.  We do this by starting off assuming a 16MB partition
		// and attempting to write to a longword just beyond the top of that partition.  If we
		// can read back from the bottom address of our partition that same value, then we know
		// that we've found the actual size of the partition
		while(test_size < partition_probe_max_size)
		{
			// Write to the target address
			debug("Setting %p to %#08x\n",&((partition_starts[i])[test_size >> 2]),0);
			(partition_starts[i])[test_size >> 2] = 0;

			// Now test if the base is 0 and escape the loop if it is
			// in which case test_size is the partition size
			if(0 == *partition_starts[i]) break;

			// If we didn't break out, then double the test size
			test_size <<= 1;
			debug("Read back non-zero so looping.  test_size is now to be %#08x\n",test_size);
		}

		debug("Setting partition %d to size %#08x\n",i,test_size);
		gd->bd->bi_dram[i].size = test_size;
	}
	
  return 0;
}

extern flash_info_t flash_info[];       /* info for FLASH chips */

static u8 system_serial[8];

void get_board_serial(struct tag_serialnr *serialnr)
{
	serialnr->high = (system_serial[0]<<24) + (system_serial[1]<<16) + (system_serial[2]<<8) + system_serial[3];
	serialnr->low  = (system_serial[4]<<24) + (system_serial[5]<<16) + (system_serial[6]<<8) + system_serial[7];
}

/*
 * FNV-1 with 64-bit hash
 * http://isthe.com/chongo/tech/comp/fnv/
 *
 * Algorithm is:
 * 	hash = offset_basis
 * 	for each octet_of_data to be hashed
 * 		hash = hash * FNV_prime
 * 		hash = hash xor octet_of_data
 * 	return hash
 *
 */

static const u64 FNV_prime = (u64)1099511628211ULL;

void gumstix_serial_hash(u8 *buf)
{
	unsigned byte;
	u64 hash = (u64)14695981039346656037ULL;

	// Loop through all 8 bytes
	for(byte = 0; byte < 8; byte++)
	{
		// hash = hash * FNV_prime
		hash *= FNV_prime;

		// hash = hash xor octect_of_data
		hash ^= buf[byte];
	}

	// Copy result out
	for(byte=0;byte<8;byte++) buf[byte]=((u8 *)&hash)[byte];
}

static inline unsigned int is_gumstix_oui(u8 *addr)
{
	return (addr[0] == 0x00 && addr[1] == 0x15 && addr[2] == 0xC9);
}

/**
	* gen_serial_ether_addr - Generate software assigned Ethernet address
	* based on the system_serial number
	* @addr: Pointer to a six-byte array containing the Ethernet address
	*
	* Generate an Ethernet address (MAC) that is not multicast
	* and has the local assigned bit set, keyed on the system_serial
	*/
static inline void gen_serial_ether_addr(u8 *addr)
{
	static u8 ether_serial_digit = 0;
	unsigned int i;
	for(i=0; i<5; i++) addr[i] = system_serial[i+2];

	addr [5] = (system_serial[7] & 0xc0) |		/* top bits are from system serial */
		(1 << 4) |				/* 2 bits identify interface type 1=ether, 2=usb, 3&4 undef */
		((ether_serial_digit++) & 0x0f);        /* 15 possible interfaces of each type */

	if(!is_gumstix_oui(addr))
	{
		addr [0] &= 0xfe;               /* clear multicast bit */
		addr [0] |= 0x02;               /* set local assignment bit (IEEE802) */
	}
}

inline u8 hex_digit_value(u8 digit)
{
	return (digit >= '0' && digit <= '9') ? (digit-'0') :
		(digit >= 'A' && digit <= 'F') ? (10+(digit-'A')) :
		(digit >= 'a' && digit <= 'f') ? (10+(digit-'a')) :
		0;
}

int misc_init_r(void)
{
	unsigned char serial[17], *old_serial;
	u8 addr[6];

	old_serial = getenv("serial#");
	if(NULL != old_serial)
	{
		system_serial[0] = hex_digit_value(old_serial[0]) * 16 + hex_digit_value(old_serial[1]);
		system_serial[1] = hex_digit_value(old_serial[2]) * 16 + hex_digit_value(old_serial[3]);
		system_serial[2] = hex_digit_value(old_serial[4]) * 16 + hex_digit_value(old_serial[5]);
		system_serial[3] = hex_digit_value(old_serial[6]) * 16 + hex_digit_value(old_serial[7]);
		system_serial[4] = hex_digit_value(old_serial[8]) * 16 + hex_digit_value(old_serial[9]);
		system_serial[5] = hex_digit_value(old_serial[10]) * 16 + hex_digit_value(old_serial[11]);
		system_serial[6] = hex_digit_value(old_serial[12]) * 16 + hex_digit_value(old_serial[13]);
		system_serial[7] = hex_digit_value(old_serial[14]) * 16 + hex_digit_value(old_serial[15]);
		// Here we need to do the reverse of the bit rotation below
		addr[0] = (system_serial[7] & 0x3f) << 2;
		system_serial[7] >>= 6;
		system_serial[7] |= (system_serial[6] & 0x3f) << 2;
		system_serial[6] >>= 6;
		system_serial[6] |= (system_serial[5] & 0x3f) << 2;
		system_serial[5] >>= 6;
		system_serial[5] |= addr[0];
	} else {
		//flash_read_user_serial(flash_info, (void *)system_serial, 0, 8);
	}

	if(0xff == (system_serial[0] & system_serial[1] & system_serial[2] & system_serial[3] &
		system_serial[4] & system_serial[5] & system_serial[6] & system_serial[7]))
	{
		// User serial number is all ones, so use the factory serial number
		//flash_read_factory_serial(flash_info, (void *)system_serial, 0, 8);
		gumstix_serial_hash(system_serial);
	} else {
		// User serial numbers are most easily programmed sequentially (incrementing by 1).
		// But we want to rotate the bottom bytes of the serial number by 6 bits to the left so that the incrementing bit moves
		// to be in the right place to not get masked out by 0xC0 in the MAC generation code.  To make it easier to not
		// put the wrong OUI in there, we'll only rotate the 6 bits through the bottom 3 bytes; everything above that
		// will remain as it's programmed in the flash chip.  In other words,
		// 0xNN NN 00 15 C9 00 00 01  ->  0xNN NN 00 15 C9 00 00 40
		addr[0] = (system_serial[5] & 0xFC) >> 2;		// save top 6 bits to temp space
		system_serial[5] <<= 6;					// shift up 6 bits
		system_serial[5] |= (system_serial[6] & 0xFC) >> 2;	// Roll in high bits of next byte
		system_serial[6] <<= 6;
		system_serial[6] |= (system_serial[7] & 0xFC) >> 2;
		system_serial[7] <<= 6;
		system_serial[7] |= addr[0];
	}

	sprintf(serial,"%02lX%02lX%02lX%02lX%02lX%02lX%02lX%02lX",system_serial[0],system_serial[1],system_serial[2],system_serial[3],
									system_serial[4],system_serial[5],system_serial[6],system_serial[7]);
	setenv("serial#",serial);
	gen_serial_ether_addr(addr);
	sprintf(serial,"%02lX:%02lX:%02lX:%02lX:%02lX:%02lX",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6]);
	setenv("ethaddr",serial);
	// MAC will actually get set to the card in lib_arm/board.c after misc_init returns

	// Now unprotect the linux part of flash...
//	flash_protect(FLAG_PROTECT_CLEAR, PHYS_FLASH_SECT_SIZE*2, flash_info[0].start[0] + flash_info[0].size - 1, &(flash_info[0]));
}
