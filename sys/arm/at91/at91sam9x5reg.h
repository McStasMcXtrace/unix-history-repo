/*-
 * Copyright (c) 2009 Sylvestre Gallon.  All rights reserved.
 * Copyright (c) 2010 Greg Ansley.  All rights reserved.
 * Copyright (c) 2012 M. Warener Losh.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* $FreeBSD$ */

#ifndef AT91SAM9X5REG_H_
#define AT91SAM9X5REG_H_

#ifndef AT91SAM9X25_MASTER_CLOCK
#define AT91SAM9X25_MASTER_CLOCK ((18432000 * 43)/6)
#endif

/* Chip Specific limits */
#define SAM9X25_PLL_A_MIN_IN_FREQ	  2000000 /*   2 Mhz */
#define SAM9X25_PLL_A_MAX_IN_FREQ	 32000000 /*  32 Mhz */
#define SAM9X25_PLL_A_MIN_OUT_FREQ	400000000 /* 400 Mhz */
#define SAM9X25_PLL_A_MAX_OUT_FREQ	800000000 /* 800 Mhz */
#define SAM9X25_PLL_A_MUL_SHIFT 16
#define SAM9X25_PLL_A_MUL_MASK 0xFF 
#define SAM9X25_PLL_A_DIV_SHIFT 0
#define SAM9X25_PLL_A_DIV_MASK 0xFF 

#define SAM9X25_PLL_B_MIN_IN_FREQ	  2000000 /*   2 Mhz */
#define SAM9X25_PLL_B_MAX_IN_FREQ	 32000000 /*  32 Mhz */
#define SAM9X25_PLL_B_MIN_OUT_FREQ	 30000000 /*  30 Mhz */
#define SAM9X25_PLL_B_MAX_OUT_FREQ	100000000 /* 100 Mhz */
#define SAM9X25_PLL_B_MUL_SHIFT 16
#define SAM9X25_PLL_B_MUL_MASK 0x3F 
#define SAM9X25_PLL_B_DIV_SHIFT 0
#define SAM9X25_PLL_B_DIV_MASK 0xFF 

/* 
 * Memory map, from datasheet :
 * 0x00000000 - 0x0ffffffff : Internal Memories
 * 0x10000000 - 0x1ffffffff : Chip Select 0
 * 0x20000000 - 0x2ffffffff : Chip Select 1 DDR2/LPDDR/SDR/LPSDR
 * 0x30000000 - 0x3ffffffff : Chip Select 2 
 * 0x40000000 - 0x4ffffffff : Chip Select 3 NAND Flash
 * 0x50000000 - 0x5ffffffff : Chip Select 4
 * 0x60000000 - 0x6ffffffff : Chip Select 5
 * 0x70000000 - 0xeffffffff : Undefined (Abort)
 * 0xf0000000 - 0xfffffffff : Peripherals
 */

#define AT91_CHIPSELECT_0 0x10000000
#define AT91_CHIPSELECT_1 0x20000000
#define AT91_CHIPSELECT_2 0x30000000
#define AT91_CHIPSELECT_3 0x40000000
#define AT91_CHIPSELECT_4 0x50000000
#define AT91_CHIPSELECT_5 0x60000000

#define AT91SAM9X25_EMAC_SIZE  0x4000
#define AT91SAM9X25_EMAC0_BASE 0x802c000
#define AT91SAM9X25_EMAC0_SIZE AT91SAM9X25_EMAC_SIZE
#define AT91SAM9X25_EMAC1_BASE 0x8030000
#define AT91SAM9X25_EMAC1_SIZE AT91SAM9X25_EMAC_SIZE

#define AT91SAM9X25_RSTC_BASE	0xffffe00
#define AT91SAM9X25_RSTC_SIZE	0x10

/* USART*/

#define AT91SAM9X25_USART_SIZE	0x4000
#define AT91SAM9X25_USART0_BASE	0x801c000
#define AT91SAM9X25_USART0_PDC	0x801c100
#define AT91SAM9X25_USART0_SIZE	AT91SAM9X25_USART_SIZE
#define AT91SAM9X25_USART1_BASE	0x8020000
#define AT91SAM9X25_USART1_PDC	0x8020100
#define AT91SAM9X25_USART1_SIZE	AT91SAM9X25_USART_SIZE
#define AT91SAM9X25_USART2_BASE	0x8024000
#define AT91SAM9X25_USART2_PDC	0x8024100
#define AT91SAM9X25_USART2_SIZE	AT91SAM9X25_USART_SIZE
#define AT91SAM9X25_USART3_BASE	0x8028000
#define AT91SAM9X25_USART3_PDC	0x8028100
#define AT91SAM9X25_USART3_SIZE	AT91SAM9X25_USART_SIZE

/*TC*/
#define AT91SAM9X25_TC0_BASE	0x8008000
#define AT91SAM9X25_TC0_SIZE	0x4000
#define AT91SAM9X25_TC0C0_BASE	0x8008000
#define AT91SAM9X25_TC0C1_BASE	0x8008040
#define AT91SAM9X25_TC0C2_BASE	0x8008080

#define AT91SAM9X25_TC1_BASE	0x800c000
#define AT91SAM9X25_TC1_SIZE	0x4000

/*SPI*/

#define AT91SAM9X25_SPI0_BASE	0x0000000

#define AT91SAM9X25_SPI0_SIZE	0x4000

#define AT91SAM9X25_SPI1_BASE	0x0004000
#define AT91SAM9X25_SPI1_SIZE	0x4000

/* System Registers */
#define AT91SAM9X25_SYS_BASE	0xffff000
#define AT91SAM9X25_SYS_SIZE	0x1000

#define AT91SAM9X25_MATRIX_BASE	0xfffde00
#define AT91SAM9X25_MATRIX_SIZE	0x200

#define AT91SAM9X25_DBGU_BASE	0xffff200
#define AT91SAM9X25_DBGU_SIZE	0x200

/*
 * PIO
 */
#define AT91SAM9X25_PIOA_BASE	0xffff400
#define AT91SAM9X25_PIOA_SIZE	0x200
#define AT91SAM9X25_PIOB_BASE	0xffff600
#define AT91SAM9X25_PIOB_SIZE	0x200
#define AT91SAM9X25_PIOC_BASE	0xffff800
#define AT91SAM9X25_PIOC_SIZE	0x200
#define AT91SAM9X25_PIOD_BASE	0xffffa00
#define AT91SAM9X25_PIOD_SIZE	0x200

#define AT91RM92_PMC_BASE	0xffffc00
#define AT91RM92_PMC_SIZE	0x100
/* IRQs :
 * 0: AIC 
 * 1: System peripheral (System timer, RTC, DBGU)
 * 2: PIO Controller A,B
 * 3: PIO Controller C,D
 * 4: SMD Soft Modem
 * 5: USART 0
 * 6: USART 1
 * 7: USART 2
 * 8: USART 3
 * 9: Two-wirte interface
 * 10: Two-wirte interface
 * 11: Two-wirte interface
 * 12: HSMCI Interface
 * 13: SPI 0
 * 14: SPI 1
 * 15: UART0
 * 16: UART1
 * 17: Timer Counter 0,1
 * 18: PWM
 * 19: ADC
 * 20: DMAC 0
 * 21: DMAC 1
 * 22: UHPHS - USB Host controller
 * 23: UDPHS - USB Device Controller
 * 24: EMAC0
 * 25: LCD controller or Image Sensor Interface
 * 26: HSMCI1
 * 27: EMAC1
 * 28: SSC
 * 29: CAN0
 * 30: CAN1
 * 31: AIC IRQ0
 */

#define AT91SAM9X25_IRQ_AIC	0
#define AT91SAM9X25_IRQ_SYSTEM	1
#define AT91SAM9X25_IRQ_PIOAB	2
#define AT91SAM9X25_IRQ_PIOCD	3
#define AT91SAM9X25_IRQ_SMD	4
#define AT91SAM9X25_IRQ_USART0	5
#define AT91SAM9X25_IRQ_USART1	6
#define AT91SAM9X25_IRQ_USART2	7
#define AT91SAM9X25_IRQ_USART3	8
#define AT91SAM9X25_IRQ_TWI0	9
#define AT91SAM9X25_IRQ_TWI1	10
#define AT91SAM9X25_IRQ_TWI2	11
#define AT91SAM9X25_IRQ_HSMCI0	12
#define AT91SAM9X25_IRQ_SPI0	13
#define AT91SAM9X25_IRQ_SPI1	14
#define AT91SAM9X25_IRQ_UART0	15
#define AT91SAM9X25_IRQ_UART1	16
#define AT91SAM9X25_IRQ_TC01	17
#define AT91SAM9X25_IRQ_PWM	18
#define AT91SAM9X25_IRQ_ADC	19
#define AT91SAM9X25_IRQ_DMAC0	20
#define AT91SAM9X25_IRQ_DMAC1	21
#define AT91SAM9X25_IRQ_UHPHS	22
#define AT91SAM9X25_IRQ_UDPHS	23
#define AT91SAM9X25_IRQ_EMAC0	24
#define AT91SAM9X25_IRQ_HSMCI1	26
#define AT91SAM9X25_IRQ_EMAC1	27
#define AT91SAM9X25_IRQ_SSC	28
#define AT91SAM9X25_IRQ_CAN0	29
#define AT91SAM9X25_IRQ_CAN1	30
#define AT91SAM9X25_IRQ_AICBASE	31

/* Alias */
#define AT91SAM9X25_IRQ_DBGU 	AT91SAM9X25_IRQ_SYSTEM
#define AT91SAM9X25_IRQ_PMC 	AT91SAM9X25_IRQ_SYSTEM
#define AT91SAM9X25_IRQ_WDT 	AT91SAM9X25_IRQ_SYSTEM
#define AT91SAM9X25_IRQ_PIT 	AT91SAM9X25_IRQ_SYSTEM
#define AT91SAM9X25_IRQ_RSTC 	AT91SAM9X25_IRQ_SYSTEM
#define AT91SAM9X25_IRQ_OHCI 	AT91SAM9X25_IRQ_UHPHS
#define AT91SAM9X25_IRQ_EHCI 	AT91SAM9X25_IRQ_UHPHS
#define AT91SAM9X25_IRQ_PIOA    AT91SAM9X25_IRQ_PIOAB
#define AT91SAM9X25_IRQ_PIOB    AT91SAM9X25_IRQ_PIOAB
#define AT91SAM9X25_IRQ_PIOC    AT91SAM9X25_IRQ_PIOCD
#define AT91SAM9X25_IRQ_PIOD    AT91SAM9X25_IRQ_PIOCD
#define AT91SAM9X25_IRQ_NAND 	(-1)
#define AT91SAM9X25_IRQ_AIC	(-1)

#define AT91SAM9X25_AIC_BASE	0xffff000
#define AT91SAM9X25_AIC_SIZE	0x200

/* Timer */

#define AT91SAM9X25_WDT_BASE	0xffffd40
#define AT91SAM9X25_WDT_SIZE	0x10

#define AT91SAM9X25_PIT_BASE	0xffffd30
#define AT91SAM9X25_PIT_SIZE	0x10

#define AT91SAM9X25_SMC_BASE	0xfffea00
#define AT91SAM9X25_SMC_SIZE	0x200

#define AT91SAM9X25_PMC_BASE	0xffffc00
#define AT91SAM9X25_PMC_SIZE	0x100

#define AT91SAM9X25_UDPHS_BASE	0x803c000
#define AT91SAM9X25_UDPHS_SIZE	0x4000

#define AT91SAM9X25_HSMCI_SIZE	0x4000
#define AT91SAM9X25_HSMCI0_BASE	0x0008000
#define AT91SAM9X25_HSMCI0_SIZE AT91SAM9X25_HSMCI_SIZE
#define AT91SAM9X25_HSMCI1_BASE	0x000c000
#define AT91SAM9X25_HSMCI1_SIZE AT91SAM9X25_HSMCI_SIZE

#define AT91SAM9X25_TWI_SIZE	0x4000
#define AT91SAM9X25_TWI0_BASE	0xffaC000
#define AT91SAM9X25_TWI0_SIZE	AT91SAM9X25_TWI_SIZE
#define AT91SAM9X25_TWI1_BASE	0xffaC000
#define AT91SAM9X25_TWI1_SIZE	AT91SAM9X25_TWI_SIZE
#define AT91SAM9X25_TWI2_BASE	0xffaC000
#define AT91SAM9X25_TWI2_SIZE	AT91SAM9X25_TWI_SIZE

/* XXX Needs to be carfully coordinated with
 * other * soc's so phyical and vm address
 * mapping are unique. XXX
 */
#define AT91SAM9X25_OHCI_BASE	  0xdfc00000 /* SAME as 9c40 */
#define AT91SAM9X25_OHCI_PA_BASE  0x00600000
#define AT91SAM9X25_OHCI_SIZE	  0x00100000

#define AT91SAM9X25_EHCI_BASE	  0xdfd00000
#define AT91SAM9X25_EHCI_PA_BASE  0x00700000
#define AT91SAM9X25_EHCI_SIZE	  0x00100000

#define AT91SAM9X25_NAND_BASE     0xe0000000
#define AT91SAM9X25_NAND_PA_BASE  0x40000000
#define AT91SAM9X25_NAND_SIZE     0x10000000


/* SDRAMC */
#define AT91SAM9X25_SDRAMC_BASE	0xfffea00               /* SAME as SMC? */
#define AT91SAM9X25_SDRAMC_MR	0x00
#define AT91SAM9X25_SDRAMC_MR_MODE_NORMAL	0
#define AT91SAM9X25_SDRAMC_MR_MODE_NOP	1
#define AT91SAM9X25_SDRAMC_MR_MODE_PRECHARGE 2
#define AT91SAM9X25_SDRAMC_MR_MODE_LOAD_MODE_REGISTER 3
#define AT91SAM9X25_SDRAMC_MR_MODE_REFRESH	4
#define AT91SAM9X25_SDRAMC_TR	0x04
#define AT91SAM9X25_SDRAMC_CR	0x08
#define AT91SAM9X25_SDRAMC_CR_NC_8		0x0
#define AT91SAM9X25_SDRAMC_CR_NC_9		0x1
#define AT91SAM9X25_SDRAMC_CR_NC_10	0x2
#define AT91SAM9X25_SDRAMC_CR_NC_11	0x3
#define AT91SAM9X25_SDRAMC_CR_NC_MASK	0x00000003
#define AT91SAM9X25_SDRAMC_CR_NR_11	0x0
#define AT91SAM9X25_SDRAMC_CR_NR_12	0x4
#define AT91SAM9X25_SDRAMC_CR_NR_13	0x8
#define AT91SAM9X25_SDRAMC_CR_NR_RES	0xc
#define AT91SAM9X25_SDRAMC_CR_NR_MASK	0x0000000c
#define AT91SAM9X25_SDRAMC_CR_NB_2		0x00
#define AT91SAM9X25_SDRAMC_CR_NB_4		0x10
#define AT91SAM9X25_SDRAMC_CR_DBW_16		0x80
#define AT91SAM9X25_SDRAMC_CR_NB_MASK	0x00000010
#define AT91SAM9X25_SDRAMC_CR_NCAS_MASK	0x00000060
#define AT91SAM9X25_SDRAMC_CR_TWR_MASK	0x00000780
#define AT91SAM9X25_SDRAMC_CR_TRC_MASK	0x00007800
#define AT91SAM9X25_SDRAMC_CR_TRP_MASK	0x00078000
#define AT91SAM9X25_SDRAMC_CR_TRCD_MASK	0x00780000
#define AT91SAM9X25_SDRAMC_CR_TRAS_MASK	0x07800000
#define AT91SAM9X25_SDRAMC_CR_TXSR_MASK	0x78000000
#define AT91SAM9X25_SDRAMC_HSR	0x0c
#define AT91SAM9X25_SDRAMC_LPR	0x10
#define AT91SAM9X25_SDRAMC_IER	0x14
#define AT91SAM9X25_SDRAMC_IDR	0x18
#define AT91SAM9X25_SDRAMC_IMR	0x1c
#define AT91SAM9X25_SDRAMC_ISR	0x20
#define AT91SAM9X25_SDRAMC_MDR	0x24

#endif /* AT91SAM9X5REG_H_*/
