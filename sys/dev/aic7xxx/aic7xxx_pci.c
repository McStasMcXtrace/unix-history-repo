/*
 * Product specific probe and attach routines for:
 *      3940, 2940, aic7895, aic7890, aic7880,
 *	aic7870, aic7860 and aic7850 SCSI controllers
 *
 * Copyright (c) 1995-2000 Justin T. Gibbs
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU Public License ("GPL").
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: //depot/src/aic7xxx/aic7xxx_pci.c#24 $
 *
 * $FreeBSD$
 */

#include <dev/aic7xxx/aic7xxx_freebsd.h>
#include <dev/aic7xxx/aic7xxx_inline.h>
#include <dev/aic7xxx/aic7xxx_93cx6.h>

#define AHC_PCI_IOADDR	PCIR_MAPS	/* I/O Address */
#define AHC_PCI_MEMADDR	(PCIR_MAPS + 4)	/* Mem I/O Address */

static __inline uint64_t
ahc_compose_id(u_int device, u_int vendor, u_int subdevice, u_int subvendor)
{
	uint64_t id;

	id = subvendor
	   | (subdevice << 16)
	   | ((uint64_t)vendor << 32)
	   | ((uint64_t)device << 48);

	return (id);
}

#define ID_ALL_MASK			0xFFFFFFFFFFFFFFFFull
#define ID_DEV_VENDOR_MASK		0xFFFFFFFF00000000ull
#define ID_9005_GENERIC_MASK		0xFFF0FFFF00000000ull
#define ID_9005_SISL_MASK		0x000FFFFF00000000ull
#define ID_9005_SISL_ID			0x0005900500000000ull
#define ID_AIC7850			0x5078900400000000ull
#define ID_AHA_2902_04_10_15_20_30C	0x5078900478509004ull
#define ID_AIC7855			0x5578900400000000ull
#define ID_AIC7859			0x3860900400000000ull
#define ID_AHA_2930CU			0x3860900438699004ull
#define ID_AIC7860			0x6078900400000000ull
#define ID_AIC7860C			0x6078900478609004ull
#define ID_AHA_1480A			0x6075900400000000ull
#define ID_AHA_2940AU_0			0x6178900400000000ull
#define ID_AHA_2940AU_1			0x6178900478619004ull
#define ID_AHA_2940AU_CN		0x2178900478219004ull
#define ID_AHA_2930C_VAR		0x6038900438689004ull

#define ID_AIC7870			0x7078900400000000ull
#define ID_AHA_2940			0x7178900400000000ull
#define ID_AHA_3940			0x7278900400000000ull
#define ID_AHA_398X			0x7378900400000000ull
#define ID_AHA_2944			0x7478900400000000ull
#define ID_AHA_3944			0x7578900400000000ull
#define ID_AHA_4944			0x7678900400000000ull

#define ID_AIC7880			0x8078900400000000ull
#define ID_AIC7880_B			0x8078900478809004ull
#define ID_AHA_2940U			0x8178900400000000ull
#define ID_AHA_3940U			0x8278900400000000ull
#define ID_AHA_2944U			0x8478900400000000ull
#define ID_AHA_3944U			0x8578900400000000ull
#define ID_AHA_398XU			0x8378900400000000ull
#define ID_AHA_4944U			0x8678900400000000ull
#define ID_AHA_2940UB			0x8178900478819004ull
#define ID_AHA_2930U			0x8878900478889004ull
#define ID_AHA_2940U_PRO		0x8778900478879004ull
#define ID_AHA_2940U_CN			0x0078900478009004ull

#define ID_AIC7895			0x7895900478959004ull
#define ID_AIC7895_ARO			0x7890900478939004ull
#define ID_AIC7895_ARO_MASK		0xFFF0FFFFFFFFFFFFull
#define ID_AHA_2940U_DUAL		0x7895900478919004ull
#define ID_AHA_3940AU			0x7895900478929004ull
#define ID_AHA_3944AU			0x7895900478949004ull

#define ID_AIC7890			0x001F9005000F9005ull
#define ID_AIC7890_ARO			0x00139005000F9005ull
#define ID_AAA_131U2			0x0013900500039005ull
#define ID_AHA_2930U2			0x0011900501819005ull
#define ID_AHA_2940U2B			0x00109005A1009005ull
#define ID_AHA_2940U2_OEM		0x0010900521809005ull
#define ID_AHA_2940U2			0x00109005A1809005ull
#define ID_AHA_2950U2B			0x00109005E1009005ull

#define ID_AIC7892			0x008F9005FFFF9005ull
#define ID_AIC7892_ARO			0x00839005FFFF9005ull
#define ID_AHA_29160			0x00809005E2A09005ull
#define ID_AHA_29160_CPQ		0x00809005E2A00E11ull
#define ID_AHA_29160N			0x0080900562A09005ull
#define ID_AHA_29160C			0x0080900562209005ull
#define ID_AHA_29160B			0x00809005E2209005ull
#define ID_AHA_19160B			0x0081900562A19005ull

#define ID_AIC7896			0x005F9005FFFF9005ull
#define ID_AIC7896_ARO			0x00539005FFFF9005ull
#define ID_AHA_3950U2B_0		0x00509005FFFF9005ull
#define ID_AHA_3950U2B_1		0x00509005F5009005ull
#define ID_AHA_3950U2D_0		0x00519005FFFF9005ull
#define ID_AHA_3950U2D_1		0x00519005B5009005ull

#define ID_AIC7899			0x00CF9005FFFF9005ull
#define ID_AIC7899_ARO			0x00C39005FFFF9005ull
#define ID_AHA_3960D			0x00C09005F6209005ull
#define ID_AHA_3960D_CPQ		0x00C09005F6200E11ull

#define ID_AIC7810			0x1078900400000000ull
#define ID_AIC7815			0x7815900400000000ull

#define DEVID_9005_TYPE(id) ((id) & 0xF)
#define		DEVID_9005_TYPE_HBA		0x0	/* Standard Card */
#define		DEVID_9005_TYPE_AAA		0x3	/* RAID Card */
#define		DEVID_9005_TYPE_SISL		0x5	/* Low Cost Card */
#define		DEVID_9005_TYPE_MB		0xF	/* On Motherboard */

#define DEVID_9005_MAXRATE(id) (((id) & 0x30) >> 4)
#define		DEVID_9005_MAXRATE_U160		0x0
#define		DEVID_9005_MAXRATE_ULTRA2	0x1
#define		DEVID_9005_MAXRATE_ULTRA	0x2
#define		DEVID_9005_MAXRATE_FAST		0x3

#define DEVID_9005_MFUNC(id) (((id) & 0x40) >> 6)

#define DEVID_9005_CLASS(id) (((id) & 0xFF00) >> 8)
#define		DEVID_9005_CLASS_SPI		0x0	/* Parallel SCSI */

#define SUBID_9005_TYPE(id) ((id) & 0xF)
#define		SUBID_9005_TYPE_MB		0xF	/* On Motherboard */
#define		SUBID_9005_TYPE_CARD		0x0	/* Standard Card */
#define		SUBID_9005_TYPE_LCCARD		0x1	/* Low Cost Card */
#define		SUBID_9005_TYPE_RAID		0x3	/* Combined with Raid */

#define SUBID_9005_TYPE_KNOWN(id)			\
	  ((((id) & 0xF) == SUBID_9005_TYPE_MB)		\
	|| (((id) & 0xF) == SUBID_9005_TYPE_CARD)	\
	|| (((id) & 0xF) == SUBID_9005_TYPE_LCCARD)	\
	|| (((id) & 0xF) == SUBID_9005_TYPE_RAID))

#define SUBID_9005_MAXRATE(id) (((id) & 0x30) >> 4)
#define		SUBID_9005_MAXRATE_ULTRA2	0x0
#define		SUBID_9005_MAXRATE_ULTRA	0x1
#define		SUBID_9005_MAXRATE_U160		0x2
#define		SUBID_9005_MAXRATE_RESERVED	0x3

#define SUBID_9005_SEEPTYPE(id)						\
	((SUBID_9005_TYPE(id) == SUBID_9005_TYPE_MB)			\
	 ? ((id) & 0xC0) >> 6						\
	 : ((id) & 0x300) >> 8)
#define		SUBID_9005_SEEPTYPE_NONE	0x0
#define		SUBID_9005_SEEPTYPE_1K		0x1
#define		SUBID_9005_SEEPTYPE_2K_4K	0x2
#define		SUBID_9005_SEEPTYPE_RESERVED	0x3
#define SUBID_9005_AUTOTERM(id)						\
	((SUBID_9005_TYPE(id) == SUBID_9005_TYPE_MB)			\
	 ? (((id) & 0x400) >> 10) == 0					\
	 : (((id) & 0x40) >> 6) == 0)

#define SUBID_9005_NUMCHAN(id)						\
	((SUBID_9005_TYPE(id) == SUBID_9005_TYPE_MB)			\
	 ? ((id) & 0x300) >> 8						\
	 : ((id) & 0xC00) >> 10)

#define SUBID_9005_LEGACYCONN(id)					\
	((SUBID_9005_TYPE(id) == SUBID_9005_TYPE_MB)			\
	 ? 0								\
	 : ((id) & 0x80) >> 7)

#define SUBID_9005_MFUNCENB(id)						\
	((SUBID_9005_TYPE(id) == SUBID_9005_TYPE_MB)			\
	 ? ((id) & 0x800) >> 11						\
	 : ((id) & 0x1000) >> 12)
/*
 * Informational only. Should use chip register to be
 * ceratian, but may be use in identification strings.
 */
#define SUBID_9005_CARD_SCSIWIDTH_MASK	0x2000
#define SUBID_9005_CARD_PCIWIDTH_MASK	0x4000
#define SUBID_9005_CARD_SEDIFF_MASK	0x8000

static ahc_device_setup_t ahc_aic785X_setup;
static ahc_device_setup_t ahc_aic7860_setup;
static ahc_device_setup_t ahc_apa1480_setup;
static ahc_device_setup_t ahc_aic7870_setup;
static ahc_device_setup_t ahc_aha394X_setup;
static ahc_device_setup_t ahc_aha494X_setup;
static ahc_device_setup_t ahc_aha398X_setup;
static ahc_device_setup_t ahc_aic7880_setup;
static ahc_device_setup_t ahc_aha2940Pro_setup;
static ahc_device_setup_t ahc_aha394XU_setup;
static ahc_device_setup_t ahc_aha398XU_setup;
static ahc_device_setup_t ahc_aic7890_setup;
static ahc_device_setup_t ahc_aic7892_setup;
static ahc_device_setup_t ahc_aic7895_setup;
static ahc_device_setup_t ahc_aic7896_setup;
static ahc_device_setup_t ahc_aic7899_setup;
static ahc_device_setup_t ahc_aha29160C_setup;
static ahc_device_setup_t ahc_raid_setup;
static ahc_device_setup_t ahc_aha394XX_setup;
static ahc_device_setup_t ahc_aha494XX_setup;
static ahc_device_setup_t ahc_aha398XX_setup;

struct ahc_pci_identity ahc_pci_ident_table [] =
{
	/* aic7850 based controllers */
	{
		ID_AHA_2902_04_10_15_20_30C,
		ID_ALL_MASK,
		"Adaptec 2902/04/10/15/20/30C SCSI adapter",
		ahc_aic785X_setup
	},
	/* aic7860 based controllers */
	{
		ID_AHA_2930CU,
		ID_ALL_MASK,
		"Adaptec 2930CU SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AHA_1480A & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 1480A Ultra SCSI adapter",
		ahc_apa1480_setup
	},
	{
		ID_AHA_2940AU_0 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940A Ultra SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AHA_2940AU_CN & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940A/CN Ultra SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AHA_2930C_VAR & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2930C Ultra SCSI adapter (VAR)",
		ahc_aic7860_setup
	},
	/* aic7870 based controllers */
	{
		ID_AHA_2940,
		ID_ALL_MASK,
		"Adaptec 2940 SCSI adapter",
		ahc_aic7870_setup
	},
	{
		ID_AHA_3940,
		ID_ALL_MASK,
		"Adaptec 3940 SCSI adapter",
		ahc_aha394X_setup
	},
	{
		ID_AHA_398X,
		ID_ALL_MASK,
		"Adaptec 398X SCSI RAID adapter",
		ahc_aha398X_setup
	},
	{
		ID_AHA_2944,
		ID_ALL_MASK,
		"Adaptec 2944 SCSI adapter",
		ahc_aic7870_setup
	},
	{
		ID_AHA_3944,
		ID_ALL_MASK,
		"Adaptec 3944 SCSI adapter",
		ahc_aha394X_setup
	},
	{
		ID_AHA_4944,
		ID_ALL_MASK,
		"Adaptec 4944 SCSI adapter",
		ahc_aha494X_setup
	},
	/* aic7880 based controllers */
	{
		ID_AHA_2940U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_3940U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 3940 Ultra SCSI adapter",
		ahc_aha394XU_setup
	},
	{
		ID_AHA_2944U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2944 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_3944U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 3944 Ultra SCSI adapter",
		ahc_aha394XU_setup
	},
	{
		ID_AHA_398XU & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 398X Ultra SCSI RAID adapter",
		ahc_aha398XU_setup
	},
	{
		/*
		 * XXX Don't know the slot numbers
		 * so we can't identify channels
		 */
		ID_AHA_4944U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 4944 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_2930U & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2930 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AHA_2940U_PRO & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940 Pro Ultra SCSI adapter",
		ahc_aha2940Pro_setup
	},
	{
		ID_AHA_2940U_CN & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec 2940/CN Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	/* Ignore all SISL (AAC on MB) based controllers. */
	{
		ID_9005_SISL_ID,
		ID_9005_SISL_MASK,
		NULL,
		NULL
	},
	/* aic7890 based controllers */
	{
		ID_AHA_2930U2,
		ID_ALL_MASK,
		"Adaptec 2930 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2940U2B,
		ID_ALL_MASK,
		"Adaptec 2940B Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2940U2_OEM,
		ID_ALL_MASK,
		"Adaptec 2940 Ultra2 SCSI adapter (OEM)",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2940U2,
		ID_ALL_MASK,
		"Adaptec 2940 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AHA_2950U2B,
		ID_ALL_MASK,
		"Adaptec 2950 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AIC7890_ARO,
		ID_ALL_MASK,
		"Adaptec aic7890/91 Ultra2 SCSI adapter (ARO)",
		ahc_aic7890_setup
	},
	{
		ID_AAA_131U2,
		ID_ALL_MASK,
		"Adaptec AAA-131 Ultra2 RAID adapter",
		ahc_aic7890_setup
	},
	/* aic7892 based controllers */
	{
		ID_AHA_29160,
		ID_ALL_MASK,
		"Adaptec 29160 Ultra160 SCSI adapter",
		ahc_aic7892_setup
	},
	{
		ID_AHA_29160_CPQ,
		ID_ALL_MASK,
		"Adaptec (Compaq OEM) 29160 Ultra160 SCSI adapter",
		ahc_aic7892_setup
	},
	{
		ID_AHA_29160N,
		ID_ALL_MASK,
		"Adaptec 29160N Ultra160 SCSI adapter",
		ahc_aic7892_setup
	},
	{
		ID_AHA_29160C,
		ID_ALL_MASK,
		"Adaptec 29160C Ultra160 SCSI adapter",
		ahc_aha29160C_setup
	},
	{
		ID_AHA_29160B,
		ID_ALL_MASK,
		"Adaptec 29160B Ultra160 SCSI adapter",
		ahc_aic7892_setup
	},
	{
		ID_AHA_19160B,
		ID_ALL_MASK,
		"Adaptec 19160B Ultra160 SCSI adapter",
		ahc_aic7892_setup
	},
	{
		ID_AIC7892_ARO,
		ID_ALL_MASK,
		"Adaptec aic7892 Ultra160 SCSI adapter (ARO)",
		ahc_aic7892_setup
	},
	/* aic7895 based controllers */	
	{
		ID_AHA_2940U_DUAL,
		ID_ALL_MASK,
		"Adaptec 2940/DUAL Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AHA_3940AU,
		ID_ALL_MASK,
		"Adaptec 3940A Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AHA_3944AU,
		ID_ALL_MASK,
		"Adaptec 3944A Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AIC7895_ARO,
		ID_AIC7895_ARO_MASK,
		"Adaptec aic7895 Ultra SCSI adapter (ARO)",
		ahc_aic7895_setup
	},
	/* aic7896/97 based controllers */	
	{
		ID_AHA_3950U2B_0,
		ID_ALL_MASK,
		"Adaptec 3950B Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AHA_3950U2B_1,
		ID_ALL_MASK,
		"Adaptec 3950B Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AHA_3950U2D_0,
		ID_ALL_MASK,
		"Adaptec 3950D Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AHA_3950U2D_1,
		ID_ALL_MASK,
		"Adaptec 3950D Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AIC7896_ARO,
		ID_ALL_MASK,
		"Adaptec aic7896/97 Ultra2 SCSI adapter (ARO)",
		ahc_aic7896_setup
	},
	/* aic7899 based controllers */	
	{
		ID_AHA_3960D,
		ID_ALL_MASK,
		"Adaptec 3960D Ultra160 SCSI adapter",
		ahc_aic7899_setup
	},
	{
		ID_AHA_3960D_CPQ,
		ID_ALL_MASK,
		"Adaptec (Compaq OEM) 3960D Ultra160 SCSI adapter",
		ahc_aic7899_setup
	},
	{
		ID_AIC7899_ARO,
		ID_ALL_MASK,
		"Adaptec aic7899 Ultra160 SCSI adapter (ARO)",
		ahc_aic7899_setup
	},
	/* Generic chip probes for devices we don't know 'exactly' */
	{
		ID_AIC7850 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7850 SCSI adapter",
		ahc_aic785X_setup
	},
	{
		ID_AIC7855 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7855 SCSI adapter",
		ahc_aic785X_setup
	},
	{
		ID_AIC7859 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7859 SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AIC7860 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7860 Ultra SCSI adapter",
		ahc_aic7860_setup
	},
	{
		ID_AIC7870 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7870 SCSI adapter",
		ahc_aic7870_setup
	},
	{
		ID_AIC7880 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7880 Ultra SCSI adapter",
		ahc_aic7880_setup
	},
	{
		ID_AIC7890 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7890/91 Ultra2 SCSI adapter",
		ahc_aic7890_setup
	},
	{
		ID_AIC7892 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7892 Ultra160 SCSI adapter",
		ahc_aic7892_setup
	},
	{
		ID_AIC7895 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7895 Ultra SCSI adapter",
		ahc_aic7895_setup
	},
	{
		ID_AIC7896 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7896/97 Ultra2 SCSI adapter",
		ahc_aic7896_setup
	},
	{
		ID_AIC7899 & ID_9005_GENERIC_MASK,
		ID_9005_GENERIC_MASK,
		"Adaptec aic7899 Ultra160 SCSI adapter",
		ahc_aic7899_setup
	},
	{
		ID_AIC7810 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7810 RAID memory controller",
		ahc_raid_setup
	},
	{
		ID_AIC7815 & ID_DEV_VENDOR_MASK,
		ID_DEV_VENDOR_MASK,
		"Adaptec aic7815 RAID memory controller",
		ahc_raid_setup
	}
};

const u_int ahc_num_pci_devs = NUM_ELEMENTS(ahc_pci_ident_table);
		
#define AHC_394X_SLOT_CHANNEL_A	4
#define AHC_394X_SLOT_CHANNEL_B	5

#define AHC_398X_SLOT_CHANNEL_A	4
#define AHC_398X_SLOT_CHANNEL_B	8
#define AHC_398X_SLOT_CHANNEL_C	12

#define AHC_494X_SLOT_CHANNEL_A	4
#define AHC_494X_SLOT_CHANNEL_B	5
#define AHC_494X_SLOT_CHANNEL_C	6
#define AHC_494X_SLOT_CHANNEL_D	7

#define	DEVCONFIG		0x40
#define		SCBSIZE32	0x00010000ul	/* aic789X only */
#define		REXTVALID	0x00001000ul	/* ultra cards only */
#define		MPORTMODE	0x00000400ul	/* aic7870 only */
#define		RAMPSM		0x00000200ul	/* aic7870 only */
#define		VOLSENSE	0x00000100ul
#define		SCBRAMSEL	0x00000080ul
#define		MRDCEN		0x00000040ul
#define		EXTSCBTIME	0x00000020ul	/* aic7870 only */
#define		EXTSCBPEN	0x00000010ul	/* aic7870 only */
#define		BERREN		0x00000008ul
#define		DACEN		0x00000004ul
#define		STPWLEVEL	0x00000002ul
#define		DIFACTNEGEN	0x00000001ul	/* aic7870 only */

#define	CSIZE_LATTIME		0x0c
#define		CACHESIZE	0x0000003ful	/* only 5 bits */
#define		LATTIME		0x0000ff00ul

static int ahc_ext_scbram_present(struct ahc_softc *ahc);
static void ahc_scbram_config(struct ahc_softc *ahc, int enable,
				  int pcheck, int fast, int large);
static void ahc_probe_ext_scbram(struct ahc_softc *ahc);
static void check_extport(struct ahc_softc *ahc, u_int *sxfrctl1);
static void configure_termination(struct ahc_softc *ahc,
				  struct seeprom_descriptor *sd,
				  u_int adapter_control,
	 			  u_int *sxfrctl1);

static void ahc_new_term_detect(struct ahc_softc *ahc,
				int *enableSEC_low,
				int *enableSEC_high,
				int *enablePRI_low,
				int *enablePRI_high,
				int *eeprom_present);
static void aic787X_cable_detect(struct ahc_softc *ahc, int *internal50_present,
				 int *internal68_present,
				 int *externalcable_present,
				 int *eeprom_present);
static void aic785X_cable_detect(struct ahc_softc *ahc, int *internal50_present,
				 int *externalcable_present,
				 int *eeprom_present);
static int acquire_seeprom(struct ahc_softc *ahc,
			   struct seeprom_descriptor *sd);
static void release_seeprom(struct seeprom_descriptor *sd);
static void write_brdctl(struct ahc_softc *ahc, uint8_t value);
static uint8_t read_brdctl(struct ahc_softc *ahc);

struct ahc_pci_identity *
ahc_find_pci_device(ahc_dev_softc_t pci)
{
	uint64_t  full_id;
	uint16_t  device;
	uint16_t  vendor;
	uint16_t  subdevice;
	uint16_t  subvendor;
	struct	  ahc_pci_identity *entry;
	u_int	  i;

	vendor = ahc_pci_read_config(pci, PCIR_DEVVENDOR, /*bytes*/2);
	device = ahc_pci_read_config(pci, PCIR_DEVICE, /*bytes*/2);
	subvendor = ahc_pci_read_config(pci, PCIR_SUBVEND_0, /*bytes*/2);
	subdevice = ahc_pci_read_config(pci, PCIR_SUBDEV_0, /*bytes*/2);
	full_id = ahc_compose_id(device,
				 vendor,
				 subdevice,
				 subvendor);

	/* If the second function is not hooked up, ignore it. */
	if (ahc_get_pci_function(pci) > 0
	 && subvendor == 0x9005
	 && SUBID_9005_TYPE_KNOWN(subdevice) != 0
	 && SUBID_9005_MFUNCENB(subdevice) == 0)
		return (NULL);

	for (i = 0; i < ahc_num_pci_devs; i++) {
		entry = &ahc_pci_ident_table[i];
		if (entry->full_id == (full_id & entry->id_mask)) {
			/* Honor exclusion entries. */
			if (entry->name == NULL)
				return (NULL);
			return (entry);
		}
	}
	return (NULL);
}

int
ahc_pci_config(struct ahc_softc *ahc, struct ahc_pci_identity *entry)
{
	struct		 ahc_probe_config probe_config;
	struct scb_data *shared_scb_data;
	u_int		 command;
	u_int		 our_id = 0;
	u_int		 sxfrctl1;
	u_int		 scsiseq;
	u_int		 dscommand0;
	int		 error;
	uint8_t		 sblkctl;

	shared_scb_data = NULL;
	ahc_init_probe_config(&probe_config);
	error = entry->setup(ahc->dev_softc, &probe_config);
	if (error != 0)
		return (error);
	probe_config.chip |= AHC_PCI;
	probe_config.description = entry->name;

	error = ahc_pci_map_registers(ahc);
	if (error != 0)
		return (error);

	ahc_power_state_change(ahc, AHC_POWER_STATE_D0);

	/* Ensure busmastering is enabled */
	command = ahc_pci_read_config(ahc->dev_softc, PCIR_COMMAND, /*bytes*/1);
	command |= PCIM_CMD_BUSMASTEREN;
	ahc_pci_write_config(ahc->dev_softc, PCIR_COMMAND, command, /*bytes*/1);

	/* On all PCI adapters, we allow SCB paging */
	probe_config.flags |= AHC_PAGESCBS;

	error = ahc_softc_init(ahc, &probe_config);
	if (error != 0)
		return (error);

	ahc->bus_intr = ahc_pci_intr;

	/* Remeber how the card was setup in case there is no SEEPROM */
	if ((ahc_inb(ahc, HCNTRL) & POWRDN) == 0) {
		ahc_pause(ahc);
		if ((ahc->features & AHC_ULTRA2) != 0)
			our_id = ahc_inb(ahc, SCSIID_ULTRA2) & OID;
		else
			our_id = ahc_inb(ahc, SCSIID) & OID;
		sxfrctl1 = ahc_inb(ahc, SXFRCTL1) & STPWEN;
		scsiseq = ahc_inb(ahc, SCSISEQ);
	} else {
		sxfrctl1 = STPWEN;
		our_id = 7;
		scsiseq = 0;
	}

	error = ahc_reset(ahc);
	if (error != 0)
		return (ENXIO);

	if ((ahc->features & AHC_DT) != 0) {
		u_int sfunct;

		/* Perform ALT-Mode Setup */
		sfunct = ahc_inb(ahc, SFUNCT) & ~ALT_MODE;
		ahc_outb(ahc, SFUNCT, sfunct | ALT_MODE);
		ahc_outb(ahc, OPTIONMODE,
			 OPTIONMODE_DEFAULTS|AUTOACKEN|BUSFREEREV|EXPPHASEDIS);
		ahc_outb(ahc, SFUNCT, sfunct);

		/* Normal mode setup */
		ahc_outb(ahc, CRCCONTROL1, CRCVALCHKEN|CRCENDCHKEN|CRCREQCHKEN
					  |TARGCRCENDEN);
	}

	dscommand0 = ahc_inb(ahc, DSCOMMAND0);
	dscommand0 |= MPARCKEN|CACHETHEN;
	if ((ahc->features & AHC_ULTRA2) != 0) {

		/*
		 * DPARCKEN doesn't work correctly on
		 * some MBs so don't use it.
		 */
		dscommand0 &= ~DPARCKEN;
	}

	/*
	 * Handle chips that must have cache line
	 * streaming (dis/en)abled.
	 */
	if ((ahc->bugs & AHC_CACHETHEN_DIS_BUG) != 0)
		dscommand0 |= CACHETHEN;

	if ((ahc->bugs & AHC_CACHETHEN_BUG) != 0)
		dscommand0 &= ~CACHETHEN;

	ahc_outb(ahc, DSCOMMAND0, dscommand0);

	ahc->pci_cachesize =
	    ahc_pci_read_config(ahc->dev_softc, CSIZE_LATTIME,
				/*bytes*/1) & CACHESIZE;
	ahc->pci_cachesize *= 4;

	/*
	 * We cannot perform ULTRA speeds without the presense
	 * of the external precision resistor.
	 */
	if ((ahc->features & AHC_ULTRA) != 0) {
		uint32_t devconfig;

		devconfig = ahc_pci_read_config(ahc->dev_softc,
						DEVCONFIG, /*bytes*/4);
		if ((devconfig & REXTVALID) == 0)
			ahc->features &= ~AHC_ULTRA;
	}

	/* See if we have a SEEPROM and perform auto-term */
	check_extport(ahc, &sxfrctl1);

	/*
	 * Take the LED out of diagnostic mode
	 */
	sblkctl = ahc_inb(ahc, SBLKCTL);
	ahc_outb(ahc, SBLKCTL, (sblkctl & ~(DIAGLEDEN|DIAGLEDON)));

	if ((ahc->features & AHC_ULTRA2) != 0) {
		ahc_outb(ahc, DFF_THRSH, RD_DFTHRSH_MAX|WR_DFTHRSH_MAX);
	} else {
		ahc_outb(ahc, DSPCISTATUS, DFTHRSH_100);
	}

	if (ahc->flags & AHC_USEDEFAULTS) {
		/*
		 * PCI Adapter default setup
		 * Should only be used if the adapter does not have
		 * a SEEPROM.
		 */
		/* See if someone else set us up already */
		if (scsiseq != 0) {
			printf("%s: Using left over BIOS settings\n",
				ahc_name(ahc));
			ahc->flags &= ~AHC_USEDEFAULTS;
			ahc->flags |= AHC_BIOS_ENABLED;
		} else {
			/*
			 * Assume only one connector and always turn
			 * on termination.
			 */
 			our_id = 0x07;
			sxfrctl1 = STPWEN;
		}
		ahc_outb(ahc, SCSICONF, our_id|ENSPCHK|RESET_SCSI);

		ahc->our_id = our_id;
	}

	/*
	 * Take a look to see if we have external SRAM.
	 * We currently do not attempt to use SRAM that is
	 * shared among multiple controllers.
	 */
	ahc_probe_ext_scbram(ahc);

	/*
	 * Record our termination setting for the
	 * generic initialization routine.
	 */
	if ((sxfrctl1 & STPWEN) != 0)
		ahc->flags |= AHC_TERM_ENB_A;

	/* Core initialization */
	error = ahc_init(ahc);
	if (error != 0)
		return (error);

	/*
	 * Link this softc in with all other ahc instances.
	 */
	ahc_softc_insert(ahc);

	/*
	 * Allow interrupts now that we are completely setup.
	 */
	error = ahc_pci_map_int(ahc);
	if (error != 0)
		return (error);

	ahc_intr_enable(ahc, TRUE);

	return (0);
}

/*
 * Test for the presense of external sram in an
 * "unshared" configuration.
 */
static int
ahc_ext_scbram_present(struct ahc_softc *ahc)
{
	u_int chip;
	int ramps;
	int single_user;
	uint32_t devconfig;

	chip = ahc->chip & AHC_CHIPID_MASK;
	devconfig = ahc_pci_read_config(ahc->dev_softc,
					DEVCONFIG, /*bytes*/4);
	single_user = (devconfig & MPORTMODE) != 0;

	if ((ahc->features & AHC_ULTRA2) != 0)
		ramps = (ahc_inb(ahc, DSCOMMAND0) & RAMPS) != 0;
	else if (chip >= AHC_AIC7870)
		ramps = (devconfig & RAMPSM) != 0;
	else
		ramps = 0;

	if (ramps && single_user)
		return (1);
	return (0);
}

/*
 * Enable external scbram.
 */
static void
ahc_scbram_config(struct ahc_softc *ahc, int enable, int pcheck,
		  int fast, int large)
{
	uint32_t devconfig;

	if (ahc->features & AHC_MULTI_FUNC) {
		/*
		 * Set the SCB Base addr (highest address bit)
		 * depending on which channel we are.
		 */
		ahc_outb(ahc, SCBBADDR, ahc_get_pci_function(ahc->dev_softc));
	}

	devconfig = ahc_pci_read_config(ahc->dev_softc, DEVCONFIG, /*bytes*/4);
	if ((ahc->features & AHC_ULTRA2) != 0) {
		u_int dscommand0;

		dscommand0 = ahc_inb(ahc, DSCOMMAND0);
		if (enable)
			dscommand0 &= ~INTSCBRAMSEL;
		else
			dscommand0 |= INTSCBRAMSEL;
		if (large)
			dscommand0 &= ~USCBSIZE32;
		else
			dscommand0 |= USCBSIZE32;
		ahc_outb(ahc, DSCOMMAND0, dscommand0);
	} else {
		if (fast)
			devconfig &= ~EXTSCBTIME;
		else
			devconfig |= EXTSCBTIME;
		if (enable)
			devconfig &= ~SCBRAMSEL;
		else
			devconfig |= SCBRAMSEL;
		if (large)
			devconfig &= ~SCBSIZE32;
		else
			devconfig |= SCBSIZE32;
	}
	if (pcheck)
		devconfig |= EXTSCBPEN;
	else
		devconfig &= ~EXTSCBPEN;

	ahc_pci_write_config(ahc->dev_softc, DEVCONFIG, devconfig, /*bytes*/4);
}

/*
 * Take a look to see if we have external SRAM.
 * We currently do not attempt to use SRAM that is
 * shared among multiple controllers.
 */
static void
ahc_probe_ext_scbram(struct ahc_softc *ahc)
{
	int num_scbs;
	int test_num_scbs;
	int enable;
	int pcheck;
	int fast;
	int large;

	enable = FALSE;
	pcheck = FALSE;
	fast = FALSE;
	large = FALSE;
	num_scbs = 0;
	
	if (ahc_ext_scbram_present(ahc) == 0)
		goto done;

	/*
	 * Probe for the best parameters to use.
	 */
	ahc_scbram_config(ahc, /*enable*/TRUE, pcheck, fast, large);
	num_scbs = ahc_probe_scbs(ahc);
	if (num_scbs == 0) {
		/* The SRAM wasn't really present. */
		goto done;
	}
	enable = TRUE;

	/*
	 * Clear any outstanding parity error
	 * and ensure that parity error reporting
	 * is enabled.
	 */
	ahc_outb(ahc, SEQCTL, 0);
	ahc_outb(ahc, CLRINT, CLRPARERR);
	ahc_outb(ahc, CLRINT, CLRBRKADRINT);

	/* Now see if we can do parity */
	ahc_scbram_config(ahc, enable, /*pcheck*/TRUE, fast, large);
	num_scbs = ahc_probe_scbs(ahc);
	if ((ahc_inb(ahc, INTSTAT) & BRKADRINT) == 0
	 || (ahc_inb(ahc, ERROR) & MPARERR) == 0)
		pcheck = TRUE;

	/* Clear any resulting parity error */
	ahc_outb(ahc, CLRINT, CLRPARERR);
	ahc_outb(ahc, CLRINT, CLRBRKADRINT);

	/* Now see if we can do fast timing */
	ahc_scbram_config(ahc, enable, pcheck, /*fast*/TRUE, large);
	test_num_scbs = ahc_probe_scbs(ahc);
	if (test_num_scbs == num_scbs
	 && ((ahc_inb(ahc, INTSTAT) & BRKADRINT) == 0
	  || (ahc_inb(ahc, ERROR) & MPARERR) == 0))
		fast = TRUE;

	/*
	 * See if we can use large SCBs and still maintain
	 * the same overall count of SCBs.
	 */
	if ((ahc->features & AHC_LARGE_SCBS) != 0) {
		ahc_scbram_config(ahc, enable, pcheck, fast, /*large*/TRUE);
		test_num_scbs = ahc_probe_scbs(ahc);
		if (test_num_scbs >= num_scbs) {
			large = TRUE;
			num_scbs = test_num_scbs;
	 		if (num_scbs >= 64) {
				/*
				 * We have enough space to move the
				 * "busy targets table" into SCB space
				 * and make it qualify all the way to the
				 * lun level.
				 */
				ahc->flags |= AHC_SCB_BTT;
			}
		}
	}
done:
	/*
	 * Disable parity error reporting until we
	 * can load instruction ram.
	 */
	ahc_outb(ahc, SEQCTL, PERRORDIS|FAILDIS);
	/* Clear any latched parity error */
	ahc_outb(ahc, CLRINT, CLRPARERR);
	ahc_outb(ahc, CLRINT, CLRBRKADRINT);
	if (bootverbose && enable) {
		printf("%s: External SRAM, %s access%s, %dbytes/SCB\n",
		       ahc_name(ahc), fast ? "fast" : "slow", 
		       pcheck ? ", parity checking enabled" : "",
		       large ? 64 : 32);
	}
	ahc_scbram_config(ahc, enable, pcheck, fast, large);
}

/*
 * Check the external port logic for a serial eeprom
 * and termination/cable detection contrls.
 */
static void
check_extport(struct ahc_softc *ahc, u_int *sxfrctl1)
{
	struct	seeprom_descriptor sd;
	struct	seeprom_config sc;
	u_int	scsi_conf;
	u_int	adapter_control;
	int	have_seeprom;
	int	have_autoterm;

	sd.sd_ahc = ahc;
	sd.sd_control_offset = SEECTL;		
	sd.sd_status_offset = SEECTL;		
	sd.sd_dataout_offset = SEECTL;		

	/*
	 * For some multi-channel devices, the c46 is simply too
	 * small to work.  For the other controller types, we can
	 * get our information from either SEEPROM type.  Set the
	 * type to start our probe with accordingly.
	 */
	if (ahc->flags & AHC_LARGE_SEEPROM)
		sd.sd_chip = C56_66;
	else
		sd.sd_chip = C46;

	sd.sd_MS = SEEMS;
	sd.sd_RDY = SEERDY;
	sd.sd_CS = SEECS;
	sd.sd_CK = SEECK;
	sd.sd_DO = SEEDO;
	sd.sd_DI = SEEDI;

	have_seeprom = acquire_seeprom(ahc, &sd);
	if (have_seeprom) {

		if (bootverbose) 
			printf("%s: Reading SEEPROM...", ahc_name(ahc));

		for (;;) {
			u_int start_addr;

			start_addr = 32 * (ahc->channel - 'A');

			have_seeprom = read_seeprom(&sd, (uint16_t *)&sc,
						    start_addr, sizeof(sc)/2);

			if (have_seeprom)
				have_seeprom = verify_cksum(&sc);

			if (have_seeprom != 0 || sd.sd_chip == C56_66) {
				if (bootverbose) {
					if (have_seeprom == 0)
						printf ("checksum error\n");
					else
						printf ("done.\n");
				}
				break;
			}
			sd.sd_chip = C56_66;
		}
		release_seeprom(&sd);
	}

	if (!have_seeprom) {
		/*
		 * Pull scratch ram settings and treat them as
		 * if they are the contents of an seeprom if
		 * the 'ADPT' signature is found in SCB2.
		 * We manually compose the data as 16bit values
		 * to avoid endian issues.
		 */
		ahc_outb(ahc, SCBPTR, 2);
		if (ahc_inb(ahc, SCB_BASE) == 'A'
		 && ahc_inb(ahc, SCB_BASE + 1) == 'D'
		 && ahc_inb(ahc, SCB_BASE + 2) == 'P'
		 && ahc_inb(ahc, SCB_BASE + 3) == 'T') {
			uint16_t *sc_data;
			int	  i;

			sc_data = (uint16_t *)&sc;
			for (i = 0; i < 32; i++) {
				uint16_t val;
				int	 j;

				j = i * 2;
				val = ahc_inb(ahc, SRAM_BASE + j)
				    | ahc_inb(ahc, SRAM_BASE + j + 1) << 8;
			}
			have_seeprom = verify_cksum(&sc);
		}
	}

	if (!have_seeprom) {
		if (bootverbose)
			printf("%s: No SEEPROM available.\n", ahc_name(ahc));
		ahc->flags |= AHC_USEDEFAULTS;
	} else {
		/*
		 * Put the data we've collected down into SRAM
		 * where ahc_init will find it.
		 */
		int i;
		int max_targ = sc.max_targets & CFMAXTARG;
		uint16_t discenable;
		uint16_t ultraenb;

		discenable = 0;
		ultraenb = 0;
		if ((sc.adapter_control & CFULTRAEN) != 0) {
			/*
			 * Determine if this adapter has a "newstyle"
			 * SEEPROM format.
			 */
			for (i = 0; i < max_targ; i++) {
				if ((sc.device_flags[i] & CFSYNCHISULTRA) != 0){
					ahc->flags |= AHC_NEWEEPROM_FMT;
					break;
				}
			}
		}

		for (i = 0; i < max_targ; i++) {
			u_int     scsirate;
			uint16_t target_mask;

			target_mask = 0x01 << i;
			if (sc.device_flags[i] & CFDISC)
				discenable |= target_mask;
			if ((ahc->flags & AHC_NEWEEPROM_FMT) != 0) {
				if ((sc.device_flags[i] & CFSYNCHISULTRA) != 0)
					ultraenb |= target_mask;
			} else if ((sc.adapter_control & CFULTRAEN) != 0) {
				ultraenb |= target_mask;
			}
			if ((sc.device_flags[i] & CFXFER) == 0x04
			 && (ultraenb & target_mask) != 0) {
				/* Treat 10MHz as a non-ultra speed */
				sc.device_flags[i] &= ~CFXFER;
			 	ultraenb &= ~target_mask;
			}
			if ((ahc->features & AHC_ULTRA2) != 0) {
				u_int offset;

				if (sc.device_flags[i] & CFSYNCH)
					offset = MAX_OFFSET_ULTRA2;
				else 
					offset = 0;
				ahc_outb(ahc, TARG_OFFSET + i, offset);

				/*
				 * The ultra enable bits contain the
				 * high bit of the ultra2 sync rate
				 * field.
				 */
				scsirate = (sc.device_flags[i] & CFXFER)
					 | ((ultraenb & target_mask)
					    ? 0x8 : 0x0);
				if (sc.device_flags[i] & CFWIDEB)
					scsirate |= WIDEXFER;
			} else {
				scsirate = (sc.device_flags[i] & CFXFER) << 4;
				if (sc.device_flags[i] & CFSYNCH)
					scsirate |= SOFS;
				if (sc.device_flags[i] & CFWIDEB)
					scsirate |= WIDEXFER;
			}
			ahc_outb(ahc, TARG_SCSIRATE + i, scsirate);
		}
		ahc->our_id = sc.brtime_id & CFSCSIID;

		scsi_conf = (ahc->our_id & 0x7);
		if (sc.adapter_control & CFSPARITY)
			scsi_conf |= ENSPCHK;
		if (sc.adapter_control & CFRESETB)
			scsi_conf |= RESET_SCSI;

		ahc->flags |=
		    (sc.adapter_control & CFBOOTCHAN) >> CFBOOTCHANSHIFT;

		if (sc.bios_control & CFEXTEND)
			ahc->flags |= AHC_EXTENDED_TRANS_A;

		if (sc.bios_control & CFBIOSEN)
			ahc->flags |= AHC_BIOS_ENABLED;
		if (ahc->features & AHC_ULTRA
		 && (ahc->flags & AHC_NEWEEPROM_FMT) == 0) {
			/* Should we enable Ultra mode? */
			if (!(sc.adapter_control & CFULTRAEN))
				/* Treat us as a non-ultra card */
				ultraenb = 0;
		}

		if (sc.signature == CFSIGNATURE
		 || sc.signature == CFSIGNATURE2) {
			uint32_t devconfig;

			/* Honor the STPWLEVEL settings */
			devconfig = ahc_pci_read_config(ahc->dev_softc,
							DEVCONFIG, /*bytes*/4);
			devconfig &= ~STPWLEVEL;
			if ((sc.bios_control & CFSTPWLEVEL) != 0)
				devconfig |= STPWLEVEL;
			ahc_pci_write_config(ahc->dev_softc, DEVCONFIG,
					     devconfig, /*bytes*/4);
		}
		/* Set SCSICONF info */
		ahc_outb(ahc, SCSICONF, scsi_conf);
		ahc_outb(ahc, DISC_DSB, ~(discenable & 0xff));
		ahc_outb(ahc, DISC_DSB + 1, ~((discenable >> 8) & 0xff));
		ahc_outb(ahc, ULTRA_ENB, ultraenb & 0xff);
		ahc_outb(ahc, ULTRA_ENB + 1, (ultraenb >> 8) & 0xff);
	}

	/*
	 * Cards that have the external logic necessary to talk to
	 * a SEEPROM, are almost certain to have the remaining logic
	 * necessary for auto-termination control.  This assumption
	 * hasn't failed yet...
	 */
	have_autoterm = have_seeprom;
	if (have_seeprom)
		adapter_control = sc.adapter_control;
	else
		adapter_control = CFAUTOTERM;

	/*
	 * Some low-cost chips have SEEPROM and auto-term control built
	 * in, instead of using a GAL.  They can tell us directly
	 * if the termination logic is enabled.
	 */
	if ((ahc->features & AHC_SPIOCAP) != 0) {
		if ((ahc_inb(ahc, SPIOCAP) & SSPIOCPS) != 0)
			have_autoterm = TRUE;
		else
			have_autoterm = FALSE;
	}

	if (have_autoterm) {
		acquire_seeprom(ahc, &sd);
		configure_termination(ahc, &sd, adapter_control, sxfrctl1);
		release_seeprom(&sd);
	}
}

static void
configure_termination(struct ahc_softc *ahc,
		      struct seeprom_descriptor *sd,
		      u_int adapter_control,
		      u_int *sxfrctl1)
{
	uint8_t brddat;
	
	brddat = 0;

	/*
	 * Update the settings in sxfrctl1 to match the
	 * termination settings 
	 */
	*sxfrctl1 = 0;
	
	/*
	 * SEECS must be on for the GALS to latch
	 * the data properly.  Be sure to leave MS
	 * on or we will release the seeprom.
	 */
	SEEPROM_OUTB(sd, sd->sd_MS | sd->sd_CS);
	if ((adapter_control & CFAUTOTERM) != 0
	 || (ahc->features & AHC_NEW_TERMCTL) != 0) {
		int internal50_present;
		int internal68_present;
		int externalcable_present;
		int eeprom_present;
		int enableSEC_low;
		int enableSEC_high;
		int enablePRI_low;
		int enablePRI_high;
		int sum;

		enableSEC_low = 0;
		enableSEC_high = 0;
		enablePRI_low = 0;
		enablePRI_high = 0;
		if ((ahc->features & AHC_NEW_TERMCTL) != 0) {
			ahc_new_term_detect(ahc, &enableSEC_low,
					       &enableSEC_high,
					       &enablePRI_low,
					       &enablePRI_high,
					       &eeprom_present);
			if ((adapter_control & CFSEAUTOTERM) == 0) {
				if (bootverbose)
					printf("%s: Manual SE Termination\n",
					       ahc_name(ahc));
				enableSEC_low = (adapter_control & CFSELOWTERM);
				enableSEC_high =
				    (adapter_control & CFSEHIGHTERM);
			}
			if ((adapter_control & CFAUTOTERM) == 0) {
				if (bootverbose)
					printf("%s: Manual LVD Termination\n",
					       ahc_name(ahc));
				enablePRI_low = (adapter_control & CFSTERM);
				enablePRI_high = (adapter_control & CFWSTERM);
			}
			/* Make the table calculations below happy */
			internal50_present = 0;
			internal68_present = 1;
			externalcable_present = 1;
		} else if ((ahc->features & AHC_SPIOCAP) != 0) {
			aic785X_cable_detect(ahc, &internal50_present,
					     &externalcable_present,
					     &eeprom_present);
		} else {
			aic787X_cable_detect(ahc, &internal50_present,
					     &internal68_present,
					     &externalcable_present,
					     &eeprom_present);
		}

		if ((ahc->features & AHC_WIDE) == 0)
			internal68_present = 0;

		if (bootverbose
		 && (ahc->features & AHC_ULTRA2) == 0) {
			printf("%s: internal 50 cable %s present",
			       ahc_name(ahc),
			       internal50_present ? "is":"not");

			if ((ahc->features & AHC_WIDE) != 0)
				printf(", internal 68 cable %s present",
				       internal68_present ? "is":"not");
			printf("\n%s: external cable %s present\n",
			       ahc_name(ahc),
			       externalcable_present ? "is":"not");
		}
		if (bootverbose)
			printf("%s: BIOS eeprom %s present\n",
			       ahc_name(ahc), eeprom_present ? "is" : "not");

		if ((ahc->flags & AHC_INT50_SPEEDFLEX) != 0) {
			/*
			 * The 50 pin connector is a separate bus,
			 * so force it to always be terminated.
			 * In the future, perform current sensing
			 * to determine if we are in the middle of
			 * a properly terminated bus.
			 */
			internal50_present = 0;
		}

		/*
		 * Now set the termination based on what
		 * we found.
		 * Flash Enable = BRDDAT7
		 * Secondary High Term Enable = BRDDAT6
		 * Secondary Low Term Enable = BRDDAT5 (7890)
		 * Primary High Term Enable = BRDDAT4 (7890)
		 */
		if ((ahc->features & AHC_ULTRA2) == 0
		 && (internal50_present != 0)
		 && (internal68_present != 0)
		 && (externalcable_present != 0)) {
			printf("%s: Illegal cable configuration!!. "
			       "Only two connectors on the "
			       "adapter may be used at a "
			       "time!\n", ahc_name(ahc));
		}

		if ((ahc->features & AHC_WIDE) != 0
		 && ((externalcable_present == 0)
		  || (internal68_present == 0)
		  || (enableSEC_high != 0))) {
			brddat |= BRDDAT6;
			if (bootverbose) {
				if ((ahc->flags & AHC_INT50_SPEEDFLEX) != 0)
					printf("%s: 68 pin termination "
					       "Enabled\n", ahc_name(ahc));
				else
					printf("%s: %sHigh byte termination "
					       "Enabled\n", ahc_name(ahc),
					       enableSEC_high ? "Secondary "
							      : "");
			}
		}

		sum = internal50_present + internal68_present
		    + externalcable_present;
		if (sum < 2 || (enableSEC_low != 0)) {
			if ((ahc->features & AHC_ULTRA2) != 0)
				brddat |= BRDDAT5;
			else
				*sxfrctl1 |= STPWEN;
			if (bootverbose) {
				if ((ahc->flags & AHC_INT50_SPEEDFLEX) != 0)
					printf("%s: 50 pin termination "
					       "Enabled\n", ahc_name(ahc));
				else
					printf("%s: %sLow byte termination "
					       "Enabled\n", ahc_name(ahc),
					       enableSEC_low ? "Secondary "
							     : "");
			}
		}

		if (enablePRI_low != 0) {
			*sxfrctl1 |= STPWEN;
			if (bootverbose)
				printf("%s: Primary Low Byte termination "
				       "Enabled\n", ahc_name(ahc));
		}

		/*
		 * Setup STPWEN before setting up the rest of
		 * the termination per the tech note on the U160 cards.
		 */
		ahc_outb(ahc, SXFRCTL1, *sxfrctl1);

		if (enablePRI_high != 0) {
			brddat |= BRDDAT4;
			if (bootverbose)
				printf("%s: Primary High Byte "
				       "termination Enabled\n",
				       ahc_name(ahc));
		}
		
		write_brdctl(ahc, brddat);

	} else {
		if ((adapter_control & CFSTERM) != 0) {
			*sxfrctl1 |= STPWEN;

			if (bootverbose)
				printf("%s: %sLow byte termination Enabled\n",
				       ahc_name(ahc),
				       (ahc->features & AHC_ULTRA2) ? "Primary "
								    : "");
		}

		if ((adapter_control & CFWSTERM) != 0
		 && (ahc->features & AHC_WIDE) != 0) {
			brddat |= BRDDAT6;
			if (bootverbose)
				printf("%s: %sHigh byte termination Enabled\n",
				       ahc_name(ahc),
				       (ahc->features & AHC_ULTRA2)
				     ? "Secondary " : "");
		}

		/*
		 * Setup STPWEN before setting up the rest of
		 * the termination per the tech note on the U160 cards.
		 */
		ahc_outb(ahc, SXFRCTL1, *sxfrctl1);

		if ((ahc->features & AHC_WIDE) != 0)
			write_brdctl(ahc, brddat);
	}
	SEEPROM_OUTB(sd, sd->sd_MS); /* Clear CS */
}

static void
ahc_new_term_detect(struct ahc_softc *ahc, int *enableSEC_low,
		    int *enableSEC_high, int *enablePRI_low,
		    int *enablePRI_high, int *eeprom_present)
{
	uint8_t brdctl;

	/*
	 * BRDDAT7 = Eeprom
	 * BRDDAT6 = Enable Secondary High Byte termination
	 * BRDDAT5 = Enable Secondary Low Byte termination
	 * BRDDAT4 = Enable Primary high byte termination
	 * BRDDAT3 = Enable Primary low byte termination
	 */
	brdctl = read_brdctl(ahc);
	*eeprom_present = brdctl & BRDDAT7;
	*enableSEC_high = (brdctl & BRDDAT6);
	*enableSEC_low = (brdctl & BRDDAT5);
	*enablePRI_high = (brdctl & BRDDAT4);
	*enablePRI_low = (brdctl & BRDDAT3);
}

static void
aic787X_cable_detect(struct ahc_softc *ahc, int *internal50_present,
		     int *internal68_present, int *externalcable_present,
		     int *eeprom_present)
{
	uint8_t brdctl;

	/*
	 * First read the status of our cables.
	 * Set the rom bank to 0 since the
	 * bank setting serves as a multiplexor
	 * for the cable detection logic.
	 * BRDDAT5 controls the bank switch.
	 */
	write_brdctl(ahc, 0);

	/*
	 * Now read the state of the internal
	 * connectors.  BRDDAT6 is INT50 and
	 * BRDDAT7 is INT68.
	 */
	brdctl = read_brdctl(ahc);
	*internal50_present = (brdctl & BRDDAT6) ? 0 : 1;
	*internal68_present = (brdctl & BRDDAT7) ? 0 : 1;

	/*
	 * Set the rom bank to 1 and determine
	 * the other signals.
	 */
	write_brdctl(ahc, BRDDAT5);

	/*
	 * Now read the state of the external
	 * connectors.  BRDDAT6 is EXT68 and
	 * BRDDAT7 is EPROMPS.
	 */
	brdctl = read_brdctl(ahc);
	*externalcable_present = (brdctl & BRDDAT6) ? 0 : 1;
	*eeprom_present = (brdctl & BRDDAT7) ? 1 : 0;
}

static void
aic785X_cable_detect(struct ahc_softc *ahc, int *internal50_present,
		     int *externalcable_present, int *eeprom_present)
{
	uint8_t brdctl;

	ahc_outb(ahc, BRDCTL, BRDRW|BRDCS);
	ahc_outb(ahc, BRDCTL, 0);
	brdctl = ahc_inb(ahc, BRDCTL);
	*internal50_present = (brdctl & BRDDAT5) ? 0 : 1;
	*externalcable_present = (brdctl & BRDDAT6) ? 0 : 1;

	*eeprom_present = (ahc_inb(ahc, SPIOCAP) & EEPROM) ? 1 : 0;
}
	
static int
acquire_seeprom(struct ahc_softc *ahc, struct seeprom_descriptor *sd)
{
	int wait;

	if ((ahc->features & AHC_SPIOCAP) != 0
	 && (ahc_inb(ahc, SPIOCAP) & SEEPROM) == 0)
		return (0);

	/*
	 * Request access of the memory port.  When access is
	 * granted, SEERDY will go high.  We use a 1 second
	 * timeout which should be near 1 second more than
	 * is needed.  Reason: after the chip reset, there
	 * should be no contention.
	 */
	SEEPROM_OUTB(sd, sd->sd_MS);
	wait = 1000;  /* 1 second timeout in msec */
	while (--wait && ((SEEPROM_STATUS_INB(sd) & sd->sd_RDY) == 0)) {
		ahc_delay(1000);  /* delay 1 msec */
	}
	if ((SEEPROM_STATUS_INB(sd) & sd->sd_RDY) == 0) {
		SEEPROM_OUTB(sd, 0); 
		return (0);
	}
	return(1);
}

static void
release_seeprom(struct seeprom_descriptor *sd)
{
	/* Release access to the memory port and the serial EEPROM. */
	SEEPROM_OUTB(sd, 0);
}

static void
write_brdctl(struct ahc_softc *ahc, uint8_t value)
{
	uint8_t brdctl;

	if ((ahc->chip & AHC_CHIPID_MASK) == AHC_AIC7895) {
		brdctl = BRDSTB;
	 	if (ahc->channel == 'B')
			brdctl |= BRDCS;
	} else if ((ahc->features & AHC_ULTRA2) != 0) {
		brdctl = 0;
	} else {
		brdctl = BRDSTB|BRDCS;
	}
	ahc_outb(ahc, BRDCTL, brdctl);
	ahc_flush_device_writes(ahc);
	brdctl |= value;
	ahc_outb(ahc, BRDCTL, brdctl);
	ahc_flush_device_writes(ahc);
	if ((ahc->features & AHC_ULTRA2) != 0)
		brdctl |= BRDSTB_ULTRA2;
	else
		brdctl &= ~BRDSTB;
	ahc_outb(ahc, BRDCTL, brdctl);
	ahc_flush_device_writes(ahc);
	if ((ahc->features & AHC_ULTRA2) != 0)
		brdctl = 0;
	else
		brdctl &= ~BRDCS;
	ahc_outb(ahc, BRDCTL, brdctl);
}

static uint8_t
read_brdctl(ahc)
	struct 	ahc_softc *ahc;
{
	uint8_t brdctl;
	uint8_t value;

	if ((ahc->chip & AHC_CHIPID_MASK) == AHC_AIC7895) {
		brdctl = BRDRW;
	 	if (ahc->channel == 'B')
			brdctl |= BRDCS;
	} else if ((ahc->features & AHC_ULTRA2) != 0) {
		brdctl = BRDRW_ULTRA2;
	} else {
		brdctl = BRDRW|BRDCS;
	}
	ahc_outb(ahc, BRDCTL, brdctl);
	ahc_flush_device_writes(ahc);
	value = ahc_inb(ahc, BRDCTL);
	ahc_outb(ahc, BRDCTL, 0);
	return (value);
}

#define	DPE	0x80
#define SSE	0x40
#define	RMA	0x20
#define	RTA	0x10
#define STA	0x08
#define DPR	0x01

void
ahc_pci_intr(struct ahc_softc *ahc)
{
	u_int error;
	u_int status1;

	error = ahc_inb(ahc, ERROR);
	if ((error & PCIERRSTAT) == 0)
		return;

	status1 = ahc_pci_read_config(ahc->dev_softc,
				      PCIR_STATUS + 1, /*bytes*/1);

	printf("%s: PCI error Interrupt at seqaddr = 0x%x\n",
	      ahc_name(ahc),
	      ahc_inb(ahc, SEQADDR0) | (ahc_inb(ahc, SEQADDR1) << 8));

	if (status1 & DPE) {
		printf("%s: Data Parity Error Detected during address "
		       "or write data phase\n", ahc_name(ahc));
	}
	if (status1 & SSE) {
		printf("%s: Signal System Error Detected\n", ahc_name(ahc));
	}
	if (status1 & RMA) {
		printf("%s: Received a Master Abort\n", ahc_name(ahc));
	}
	if (status1 & RTA) {
		printf("%s: Received a Target Abort\n", ahc_name(ahc));
	}
	if (status1 & STA) {
		printf("%s: Signaled a Target Abort\n", ahc_name(ahc));
	}
	if (status1 & DPR) {
		printf("%s: Data Parity Error has been reported via PERR#\n",
		       ahc_name(ahc));
	}
	if ((status1 & (DPE|SSE|RMA|RTA|STA|DPR)) == 0) {
		printf("%s: Latched PCIERR interrupt with "
		       "no status bits set\n", ahc_name(ahc)); 
	}
	ahc_pci_write_config(ahc->dev_softc, PCIR_STATUS + 1,
			     status1, /*bytes*/1);

	if (status1 & (DPR|RMA|RTA)) {
		ahc_outb(ahc, CLRINT, CLRPARERR);
	}

	ahc_unpause(ahc);
}

static int
ahc_aic785X_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	uint8_t rev;

	probe_config->channel = 'A';
	probe_config->chip = AHC_AIC7850;
	probe_config->features = AHC_AIC7850_FE;
	probe_config->bugs |= AHC_TMODE_WIDEODD_BUG|AHC_CACHETHEN_BUG
			   |  AHC_PCI_MWI_BUG;
	rev = ahc_pci_read_config(pci, PCIR_REVID, /*bytes*/1);
	if (rev >= 1)
		probe_config->bugs |= AHC_PCI_2_1_RETRY_BUG;
	return (0);
}

static int
ahc_aic7860_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	uint8_t rev;

	probe_config->channel = 'A';
	probe_config->chip = AHC_AIC7860;
	probe_config->features = AHC_AIC7860_FE;
	probe_config->bugs |= AHC_TMODE_WIDEODD_BUG|AHC_CACHETHEN_BUG
			   |  AHC_PCI_MWI_BUG;
	rev = ahc_pci_read_config(pci, PCIR_REVID, /*bytes*/1);
	if (rev >= 1)
		probe_config->bugs |= AHC_PCI_2_1_RETRY_BUG;
	return (0);
}

static int
ahc_apa1480_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	error = ahc_aic7860_setup(pci, probe_config);
	if (error != 0)
		return (error);
	probe_config->features |= AHC_REMOVABLE;
	return (0);
}

static int
ahc_aic7870_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	probe_config->channel = 'A';
	probe_config->chip = AHC_AIC7870;
	probe_config->features = AHC_AIC7870_FE;
	probe_config->bugs |= AHC_TMODE_WIDEODD_BUG|AHC_CACHETHEN_BUG
			   |  AHC_PCI_MWI_BUG;
	return (0);
}

static int
ahc_aha394X_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	error = ahc_aic7870_setup(pci, probe_config);
	if (error == 0)
		error = ahc_aha394XX_setup(pci, probe_config);
	return (error);
}

static int
ahc_aha398X_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	error = ahc_aic7870_setup(pci, probe_config);
	if (error == 0)
		error = ahc_aha398XX_setup(pci, probe_config);
	return (error);
}

static int
ahc_aha494X_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	error = ahc_aic7870_setup(pci, probe_config);
	if (error == 0)
		error = ahc_aha494XX_setup(pci, probe_config);
	return (error);
}

static int
ahc_aic7880_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	uint8_t rev;

	probe_config->channel = 'A';
	probe_config->chip = AHC_AIC7880;
	probe_config->features = AHC_AIC7880_FE;
	probe_config->bugs |= AHC_TMODE_WIDEODD_BUG;
	rev = ahc_pci_read_config(pci, PCIR_REVID, /*bytes*/1);
	if (rev >= 1) {
		probe_config->bugs |= AHC_PCI_2_1_RETRY_BUG;
	} else {
		probe_config->bugs |= AHC_CACHETHEN_BUG|AHC_PCI_MWI_BUG;
	}
	return (0);
}

static int
ahc_aha2940Pro_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	probe_config->flags |= AHC_INT50_SPEEDFLEX;
	error = ahc_aic7880_setup(pci, probe_config);
	return (0);
}

static int
ahc_aha394XU_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	error = ahc_aic7880_setup(pci, probe_config);
	if (error == 0)
		error = ahc_aha394XX_setup(pci, probe_config);
	return (error);
}

static int
ahc_aha398XU_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	error = ahc_aic7880_setup(pci, probe_config);
	if (error == 0)
		error = ahc_aha398XX_setup(pci, probe_config);
	return (error);
}

static int
ahc_aic7890_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	uint8_t rev;

	probe_config->channel = 'A';
	probe_config->chip = AHC_AIC7890;
	probe_config->features = AHC_AIC7890_FE;
	probe_config->flags |= AHC_NEWEEPROM_FMT;
	rev = ahc_pci_read_config(pci, PCIR_REVID, /*bytes*/1);
	if (rev == 0)
		probe_config->bugs |= AHC_AUTOFLUSH_BUG|AHC_CACHETHEN_BUG;
	return (0);
}

static int
ahc_aic7892_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	probe_config->channel = 'A';
	probe_config->chip = AHC_AIC7892;
	probe_config->features = AHC_AIC7892_FE;
	probe_config->flags |= AHC_NEWEEPROM_FMT;
	probe_config->bugs |= AHC_SCBCHAN_UPLOAD_BUG;
	return (0);
}

static int
ahc_aic7895_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	uint8_t rev;

	probe_config->channel = ahc_get_pci_function(pci) == 1 ? 'B' : 'A';
	/*
	 * The 'C' revision of the aic7895 has a few additional features.
	 */
	rev = ahc_pci_read_config(pci, PCIR_REVID, /*bytes*/1);
	if (rev >= 4) {
		probe_config->chip = AHC_AIC7895C;
		probe_config->features = AHC_AIC7895C_FE;
	} else  {
		u_int command;

		probe_config->chip = AHC_AIC7895;
		probe_config->features = AHC_AIC7895_FE;

		/*
		 * The BIOS disables the use of MWI transactions
		 * since it does not have the MWI bug work around
		 * we have.  Disabling MWI reduces performance, so
		 * turn it on again.
		 */
		command = ahc_pci_read_config(pci, PCIR_COMMAND, /*bytes*/1);
		command |= PCIM_CMD_MWRICEN;
		ahc_pci_write_config(pci, PCIR_COMMAND, command, /*bytes*/1);
		probe_config->bugs |= AHC_PCI_MWI_BUG;
	}
	/*
	 * XXX Does CACHETHEN really not work???  What about PCI retry?
	 * on C level chips.  Need to test, but for now, play it safe.
	 */
	probe_config->bugs |= AHC_TMODE_WIDEODD_BUG|AHC_PCI_2_1_RETRY_BUG
			   |  AHC_CACHETHEN_BUG;

#if 0
	uint32_t devconfig;

	/*
	 * Cachesize must also be zero due to stray DAC
	 * problem when sitting behind some bridges.
	 */
	ahc_pci_write_config(pci, CSIZE_LATTIME, 0, /*bytes*/1);
	devconfig = ahc_pci_read_config(pci, DEVCONFIG, /*bytes*/1);
	devconfig |= MRDCEN;
	ahc_pci_write_config(pci, DEVCONFIG, devconfig, /*bytes*/1);
#endif
	probe_config->flags |= AHC_NEWEEPROM_FMT;
	return (0);
}

static int
ahc_aic7896_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	probe_config->channel = ahc_get_pci_function(pci) == 1 ? 'B' : 'A';
	probe_config->chip = AHC_AIC7896;
	probe_config->features = AHC_AIC7896_FE;
	probe_config->flags |= AHC_NEWEEPROM_FMT;
	probe_config->bugs |= AHC_CACHETHEN_DIS_BUG;
	return (0);
}

static int
ahc_aic7899_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	probe_config->channel = ahc_get_pci_function(pci) == 1 ? 'B' : 'A';
	probe_config->chip = AHC_AIC7899;
	probe_config->features = AHC_AIC7899_FE;
	probe_config->flags |= AHC_NEWEEPROM_FMT;
	probe_config->bugs |= AHC_SCBCHAN_UPLOAD_BUG;
	return (0);
}

static int
ahc_aha29160C_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	int error;

	error = ahc_aic7899_setup(pci, probe_config);
	if (error != 0)
		return (error);
	probe_config->features |= AHC_REMOVABLE;
	return (0);
}

static int
ahc_raid_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	printf("RAID functionality unsupported\n");
	return (ENXIO);
}

static int
ahc_aha394XX_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	switch (ahc_get_pci_slot(pci)) {
	case AHC_394X_SLOT_CHANNEL_A:
		probe_config->channel = 'A';
		break;
	case AHC_394X_SLOT_CHANNEL_B:
		probe_config->channel = 'B';
		break;
	default:
		printf("adapter at unexpected slot %d\n"
		       "unable to map to a channel\n",
		       ahc_get_pci_slot(pci));
		probe_config->channel = 'A';
	}
	return (0);
}

static int
ahc_aha398XX_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	switch (ahc_get_pci_slot(pci)) {
	case AHC_398X_SLOT_CHANNEL_A:
		probe_config->channel = 'A';
		break;
	case AHC_398X_SLOT_CHANNEL_B:
		probe_config->channel = 'B';
		break;
	case AHC_398X_SLOT_CHANNEL_C:
		probe_config->channel = 'C';
		break;
	default:
		printf("adapter at unexpected slot %d\n"
		       "unable to map to a channel\n",
		       ahc_get_pci_slot(pci));
		probe_config->channel = 'A';
		break;
	}
	probe_config->flags |= AHC_LARGE_SEEPROM;
	return (0);
}

static int
ahc_aha494XX_setup(ahc_dev_softc_t pci, struct ahc_probe_config *probe_config)
{
	switch (ahc_get_pci_slot(pci)) {
	case AHC_494X_SLOT_CHANNEL_A:
		probe_config->channel = 'A';
		break;
	case AHC_494X_SLOT_CHANNEL_B:
		probe_config->channel = 'B';
		break;
	case AHC_494X_SLOT_CHANNEL_C:
		probe_config->channel = 'C';
		break;
	case AHC_494X_SLOT_CHANNEL_D:
		probe_config->channel = 'D';
		break;
	default:
		printf("adapter at unexpected slot %d\n"
		       "unable to map to a channel\n",
		       ahc_get_pci_slot(pci));
		probe_config->channel = 'A';
	}
	probe_config->flags |= AHC_LARGE_SEEPROM;
	return (0);
}
