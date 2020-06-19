/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */
#ifndef _DT_BINDINGS_AMLOGIC_MESON8B_RESET_H
#define _DT_BINDINGS_AMLOGIC_MESON8B_RESET_H

/*	RESET0					*/
#define RESET_HIU			0
#define RESET_VLD			1
#define RESET_IQIDCT			2
#define RESET_MC			3
/*					8	*/
#define RESET_VIU			5
#define RESET_AIU			6
#define RESET_MCPU			7
#define RESET_CCPU			8
#define RESET_PMUX			9
#define RESET_VENC			10
#define RESET_ASSIST			11
#define RESET_AFIFO2			12
#define RESET_MDEC			13
#define RESET_VLD_PART			14
#define RESET_VIFIFO			15
/*					16-31	*/
/*	RESET1					*/
/*					32	*/
#define RESET_DEMUX			33
#define RESET_USB_OTG			34
#define RESET_DDR			35
#define RESET_VDAC_1			36
#define RESET_BT656			37
#define RESET_AHB_SRAM			38
#define RESET_AHB_BRIDGE		39
#define RESET_PARSER			40
#define RESET_BLKMV			41
#define RESET_ISA			42
#define RESET_ETHERNET			43
#define RESET_ABUF			44
#define RESET_AHB_DATA			45
#define RESET_AHB_CNTL			46
#define RESET_ROM_BOOT			47
/*					48-63	*/
/*	RESET2					*/
#define RESET_VD_RMEM			64
#define RESET_AUDIN			65
#define RESET_DBLK			66
#define RESET_PIC_DC			67
#define RESET_PSC			68
#define RESET_NAND			69
#define RESET_GE2D			70
#define RESET_PARSER_REG		71
#define RESET_PARSER_FETCH		72
#define RESET_PARSER_CTL		73
#define RESET_PARSER_TOP		74
#define RESET_HDMI_APB			75
#define RESET_AUDIO_APB			76
#define RESET_MEDIA_CPU			77
#define RESET_MALI			78
#define RESET_HDMI_SYSTEM_RESET		79
/*					80-95	*/
/*	RESET3					*/
#define RESET_RING_OSCILLATOR		96
#define RESET_SYS_CPU_0			97
#define RESET_EFUSE			98
#define RESET_SYS_CPU_BVCI		99
#define RESET_AIFIFO			100
#define RESET_AUDIO_PLL_MODULATOR	101
#define RESET_AHB_BRIDGE_CNTL		102
#define RESET_SYS_CPU_1			103
#define RESET_AUDIO_DAC			104
#define RESET_DEMUX_TOP			105
#define RESET_DEMUX_DES			106
#define RESET_DEMUX_S2P_0		107
#define RESET_DEMUX_S2P_1		108
#define RESET_DEMUX_RESET_0		109
#define RESET_DEMUX_RESET_1		110
#define RESET_DEMUX_RESET_2		111
/*					112-127	*/
/*	RESET4					*/
#define RESET_PL310			128
#define RESET_A5_APB			129
#define RESET_A5_AXI			130
#define RESET_A5			131
#define RESET_DVIN			132
#define RESET_RDMA			133
#define RESET_VENCI			134
#define RESET_VENCP			135
#define RESET_VENCT			136
#define RESET_VDAC_4			137
#define RESET_RTC			138
#define RESET_A5_DEBUG			139
#define RESET_VDI6			140
#define RESET_VENCL			141
/*					142-159	*/
/*	RESET5					*/
#define RESET_DDR_PLL			160
#define RESET_MISC_PLL			161
#define RESET_SYS_PLL			162
#define RESET_HPLL_PLL			163
#define RESET_AUDIO_PLL			164
#define RESET_VID2_PLL			165
/*					166-191	*/
/*	RESET6					*/
#define RESET_PERIPHS_GENERAL		192
#define RESET_PERIPHS_IR_REMOTE		193
#define RESET_PERIPHS_SMART_CARD	194
#define RESET_PERIPHS_SAR_ADC		195
#define RESET_PERIPHS_I2C_MASTER_0	196
#define RESET_PERIPHS_I2C_MASTER_1	197
#define RESET_PERIPHS_I2C_SLAVE		198
#define RESET_PERIPHS_STREAM_INTERFACE	199
#define RESET_PERIPHS_SDIO		200
#define RESET_PERIPHS_UART_0		201
#define RESET_PERIPHS_UART_1		202
#define RESET_PERIPHS_ASYNC_0		203
#define RESET_PERIPHS_ASYNC_1		204
#define RESET_PERIPHS_SPI_0		205
#define RESET_PERIPHS_SPI_1		206
#define RESET_PERIPHS_LED_PWM		207
/*					208-223	*/
/*	RESET7					*/
/*					224-255	*/

#endif
