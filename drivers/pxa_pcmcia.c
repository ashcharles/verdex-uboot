#include <common.h>
#include <config.h>

#ifdef CONFIG_PXA_PCMCIA

#include <pcmcia.h>
#include <asm/arch/pxa-regs.h>
#include <asm/io.h>

#ifdef CONFIG_GUMSTIX
/*
 * The following are missing from pxa-regs.h
 */
#define GPIO4_nBVD1			4
#define GPIO4_nSTSCHG			GPIO4_nBVD1
#define GPIO8_RESET			8
#define GPIO11_nPCD1			11
#define GPIO22_nINPACK			22
#define GPIO26_PRDY_nBSY0		26
#define GPIO36_nBVD2			36

#define GPIO4_nBVD1_MD			( GPIO4_nBVD1| GPIO_IN )
#define GPIO4_nSTSCHG_MD		( GPIO4_nSTSCHG | GPIO_IN )
#define GPIO8_RESET_MD			( GPIO8_RESET | GPIO_OUT )
#define GPIO11_nPCD1_MD			( GPIO11_nPCD1 | GPIO_IN )
#define GPIO22_nINPACK_MD		( GPIO22_nINPACK | GPIO_IN )
#define GPIO26_PRDY_nBSY0_MD		( GPIO26_PRDY_nBSY0 | GPIO_IN )
#define GPIO36_nBVD2_MD			( GPIO36_nBVD2 | GPIO_IN )
#endif


static inline void msWait(unsigned msVal)
{
	udelay(msVal*1000);
}

int pcmcia_on (void)
{
	unsigned int reg_arr[] = {
#ifdef CONFIG_GUMSTIX
		0x48000014, CFG_MECR_VAL,
#endif
		0x48000028, CFG_MCMEM0_VAL,
		0x4800002c, CFG_MCMEM1_VAL,
		0x48000030, CFG_MCATT0_VAL,
		0x48000034, CFG_MCATT1_VAL,
		0x48000038, CFG_MCIO0_VAL,
		0x4800003c, CFG_MCIO1_VAL,

		0, 0
	};
	int i, rc;

#ifdef CONFIG_EXADRON1
	int cardDetect;
	volatile unsigned int *v_pBCRReg =
			(volatile unsigned int *) 0x08000000;
#endif
#ifdef CONFIG_GUMSTIX
	int cardDetect;
#endif

	debug ("%s\n", __FUNCTION__);

#ifdef CONFIG_GUMSTIX
#ifdef CONFIG_PXA27X
	// Set up GPIOs and then twiddle reset line
	set_GPIO_mode(GPIO104_nPCD1_MD);
	set_GPIO_mode(GPIO111_nBVD1_MD);
	set_GPIO_mode(GPIO109_PRDY_nBSY0_MD);
	set_GPIO_mode(GPIO110_CF_RESET_MD);
	GPSR(GPIO110) = GPIO_bit(GPIO110);
	udelay(50);
	GPCR(GPIO110) = GPIO_bit(GPIO110);
#else
	set_GPIO_mode(GPIO11_nPCD1_MD);
	set_GPIO_mode(GPIO4_nBVD1_MD);
	set_GPIO_mode(GPIO36_nBVD2_MD);
	set_GPIO_mode(GPIO26_PRDY_nBSY0_MD);
	set_GPIO_mode(GPIO8_RESET_MD);
	GPSR(GPIO8_RESET) = GPIO_bit(GPIO8_RESET);
	udelay(50);
	GPCR(GPIO8_RESET) = GPIO_bit(GPIO8_RESET);
#endif

	// Setup default state of GPIO outputs before we enable them as outputs
	GPSR(GPIO48_nPOE) = GPIO_bit(GPIO48_nPOE) |
				GPIO_bit(GPIO49_nPWE) |
				GPIO_bit(GPIO50_nPIOR) |
				GPIO_bit(GPIO51_nPIOW);
#ifdef CONFIG_PXA27X
	GPSR(GPIO102) = GPIO_bit(GPIO102) | GPIO_bit(GPIO105);
#else
	GPSR(GPIO52_nPCE_1) = GPIO_bit(GPIO52_nPCE_1) | GPIO_bit(GPIO53_nPCE_2);
#endif

	set_GPIO_mode(GPIO48_nPOE_MD);
	set_GPIO_mode(GPIO49_nPWE_MD);
	set_GPIO_mode(GPIO50_nPIOR_MD);
	set_GPIO_mode(GPIO51_nPIOW_MD);
#ifdef CONFIG_PXA27X
	set_GPIO_mode(GPIO102_nPCE_1_MD);
	set_GPIO_mode(GPIO105_nPCE_2_MD);
	set_GPIO_mode(GPIO79_pSKTSEL_MD);
#else
	set_GPIO_mode(GPIO52_nPCE_1_MD);
	set_GPIO_mode(GPIO53_nPCE_2_MD);
	set_GPIO_mode(GPIO54_pSKTSEL_MD);
#endif
	set_GPIO_mode(GPIO55_nPREG_MD);
	set_GPIO_mode(GPIO56_nPWAIT_MD);
	set_GPIO_mode(GPIO57_nIOIS16_MD);

	msWait(50);

#ifdef CONFIG_PXA27X
	cardDetect = !(GPLR(104) & GPIO_bit(104));
#else
	cardDetect = !(GPLR(11) & GPIO_bit(11));
#endif
	if(!cardDetect) {
		printf ("No PCMCIA card found: %08x!\n",
#ifdef CONFIG_PXA270
				GPLR(104));
#else
				GPLR(11));
#endif
	}
#endif

	i = 0;
	while (reg_arr[i])
	{
		*((volatile unsigned int *) reg_arr[i]) |= reg_arr[i+1];
		i += 2;
	}
	udelay (1000);

	debug ("%s: programmed mem controller \n", __FUNCTION__);

#ifdef CONFIG_EXADRON1

/*define useful BCR masks */
#define BCR_CF_INIT_VAL  		    0x00007230
#define BCR_CF_PWRON_BUSOFF_RESETOFF_VAL    0x00007231
#define BCR_CF_PWRON_BUSOFF_RESETON_VAL     0x00007233
#define BCR_CF_PWRON_BUSON_RESETON_VAL      0x00007213
#define BCR_CF_PWRON_BUSON_RESETOFF_VAL     0x00007211

	/* we see from the GPIO bit if the card is present */
	cardDetect = !(GPLR0 & GPIO_bit (14));

	if (cardDetect) {
		printf ("No PCMCIA card found!\n");
	}

	/* reset the card via the BCR line */
	*v_pBCRReg = (unsigned) BCR_CF_INIT_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSOFF_RESETOFF_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSOFF_RESETON_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSON_RESETON_VAL;
	msWait (500);

	*v_pBCRReg = (unsigned) BCR_CF_PWRON_BUSON_RESETOFF_VAL;
	msWait (1500);

	/* enable address bus */
	GPCR1 = 0x01;
	/* and the first CF slot */
	MECR = 0x00000002;

#endif /* EXADRON 1 */

	rc = check_ide_device (0);	/* use just slot 0 */
	if(!rc) ide_init();

	return rc;
}

#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA)
int pcmcia_off (void)
{
	return 0;
}
#endif

#endif /* CONFIG_PXA_PCMCIA */
