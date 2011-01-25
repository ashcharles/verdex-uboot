/*
 * (C) Copyright 2007
 * Craig Hughes, Gumstix Inc. <craig@gumstix.com>
 *
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
#include <exports.h>

static volatile u8 *const OTP_START_ADDR = (u8 *)0x3FE010A; // Must drive A[MAX:17] high during program for T- flash part
static volatile u8 *const OTP_END_ADDR = (u8 *)0x3FE0111;
static u8 *serial = (u8 *)0xA0000000; // Serial number is in RAM

int gumstix_serial_otp (int argc, char *argv[])
{
	int i;
	volatile u8 *ptr;
	volatile u16 *ptr16;
	int is_set=0;

	app_startup(argv);
	if(get_version() != XF_VERSION) return 1;

	printf("Checking existing OTP serial...");
	for(ptr=OTP_START_ADDR; ptr <= OTP_END_ADDR; ptr = &(ptr[1]))
	{
		*ptr = 0x90;
		if(0xFF != *ptr)
		{
			is_set = 1;
			break;
		}
	}

	if(is_set)
	{
		printf("Found serial 0x");

		for(ptr=OTP_START_ADDR; ptr <= OTP_END_ADDR; ptr = &(ptr[1]))
		{
			printf("%02x", *ptr);
		}

		printf(": Aborting\n");
	} else {
		printf("None present\n");
	}

	*ptr = 0xFF;
	if(is_set) return 1;

	printf("\nOk, I will program: 0x");
	for(i=0; i<8; i++)
	{
		printf("%02x",serial[i]);
	}

	printf ("\nPress 'y' to confirm, or any other key to cancel");
	while (!tstc()) continue;

	if(getc() == 'y')
	{
		printf("\nOK: Programming...");

		for(i=0; i<4; i++)
		{
			ptr16 = &(((u16 *)OTP_START_ADDR)[i]);
			*ptr16 = 0xC0;
			*ptr16 = serial[i*2] | serial[i*2+1] << 8;
			while(!(*ptr16 & 0x80)) continue;
		}

		*ptr16 = 0x50;

		printf("\nConfirmed\n");

		i=7;
		do
		{
			serial[i]++;
		} while(0 == serial[i--] && i >= 0);

		return 0;
	} else {
		printf("\nAborted\n");
		return 1;
	}
}
