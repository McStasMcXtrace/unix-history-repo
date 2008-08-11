/**************************************************************************

Copyright (c) 2007-2008, Chelsio Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

 2. Neither the name of the Chelsio Corporation nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#ifdef CONFIG_DEFINED
#include <cxgb_include.h>
#else
#include <dev/cxgb/cxgb_include.h>
#endif

#undef msleep
#define msleep t3_os_sleep

enum {
	PMD_RSD     = 10,   /* PMA/PMD receive signal detect register */
	PCS_STAT1_X = 24,   /* 10GBASE-X PCS status 1 register */
	PCS_STAT1_R = 32,   /* 10GBASE-R PCS status 1 register */
	XS_LN_STAT  = 24    /* XS lane status register */
};

enum {
	AEL100X_TX_DISABLE  = 9,
	AEL100X_TX_CONFIG1  = 0xc002,
	AEL1002_PWR_DOWN_HI = 0xc011,
	AEL1002_PWR_DOWN_LO = 0xc012,
	AEL1002_XFI_EQL     = 0xc015,
	AEL1002_LB_EN       = 0xc017,
	AEL_OPT_SETTINGS    = 0xc017,
	AEL_I2C_CTRL        = 0xc30a,
	AEL_I2C_DATA        = 0xc30b,
	AEL_I2C_STAT        = 0xc30c,
	AEL2005_GPIO_CTRL   = 0xc214,
	AEL2005_GPIO_STAT   = 0xc215,
};

enum { edc_none, edc_sr, edc_twinax };

/* PHY module I2C device address */
#define MODULE_DEV_ADDR 0xa0

#define AEL2005_MODDET_IRQ 4

struct reg_val {
	unsigned short mmd_addr;
	unsigned short reg_addr;
	unsigned short clear_bits;
	unsigned short set_bits;
};

static int set_phy_regs(struct cphy *phy, const struct reg_val *rv)
{
	int err;

	for (err = 0; rv->mmd_addr && !err; rv++) {
		if (rv->clear_bits == 0xffff)
			err = mdio_write(phy, rv->mmd_addr, rv->reg_addr,
					 rv->set_bits);
		else
			err = t3_mdio_change_bits(phy, rv->mmd_addr,
						  rv->reg_addr, rv->clear_bits,
						  rv->set_bits);
	}
	return err;
}

static void ael100x_txon(struct cphy *phy)
{
	int tx_on_gpio = phy->addr == 0 ? F_GPIO7_OUT_VAL : F_GPIO2_OUT_VAL;

	msleep(100);
	t3_set_reg_field(phy->adapter, A_T3DBG_GPIO_EN, 0, tx_on_gpio);
	msleep(30);
}

static int ael1002_power_down(struct cphy *phy, int enable)
{
	int err;

	err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL100X_TX_DISABLE, !!enable);
	if (!err)
		err = t3_mdio_change_bits(phy, MDIO_DEV_PMA_PMD, MII_BMCR,
					  BMCR_PDOWN, enable ? BMCR_PDOWN : 0);
	return err;
}

static int ael1002_reset(struct cphy *phy, int wait)
{
	int err;

	if ((err = ael1002_power_down(phy, 0)) ||
	    (err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL100X_TX_CONFIG1, 1)) ||
	    (err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL1002_PWR_DOWN_HI, 0)) ||
	    (err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL1002_PWR_DOWN_LO, 0)) ||
	    (err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL1002_XFI_EQL, 0x18)) ||
	    (err = t3_mdio_change_bits(phy, MDIO_DEV_PMA_PMD, AEL1002_LB_EN,
				       0, 1 << 5)))
		return err;
	return 0;
}

static int ael1002_intr_noop(struct cphy *phy)
{
	return 0;
}

/*
 * Get link status for a 10GBASE-R device.
 */
static int get_link_status_r(struct cphy *phy, int *link_ok, int *speed,
			     int *duplex, int *fc)
{
	if (link_ok) {
		unsigned int stat0, stat1, stat2;
		int err = mdio_read(phy, MDIO_DEV_PMA_PMD, PMD_RSD, &stat0);

		if (!err)
			err = mdio_read(phy, MDIO_DEV_PCS, PCS_STAT1_R, &stat1);
		if (!err)
			err = mdio_read(phy, MDIO_DEV_XGXS, XS_LN_STAT, &stat2);
		if (err)
			return err;
		*link_ok = (stat0 & stat1 & (stat2 >> 12)) & 1;
	}
	if (speed)
		*speed = SPEED_10000;
	if (duplex)
		*duplex = DUPLEX_FULL;
	return 0;
}

#ifdef C99_NOT_SUPPORTED
static struct cphy_ops ael1002_ops = {
	ael1002_reset,
	ael1002_intr_noop,
	ael1002_intr_noop,
	ael1002_intr_noop,
	ael1002_intr_noop,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	get_link_status_r,
	ael1002_power_down,
};
#else
static struct cphy_ops ael1002_ops = {
	.reset           = ael1002_reset,
	.intr_enable     = ael1002_intr_noop,
	.intr_disable    = ael1002_intr_noop,
	.intr_clear      = ael1002_intr_noop,
	.intr_handler    = ael1002_intr_noop,
	.get_link_status = get_link_status_r,
	.power_down      = ael1002_power_down,
};
#endif

int t3_ael1002_phy_prep(struct cphy *phy, adapter_t *adapter, int phy_addr,
			const struct mdio_ops *mdio_ops)
{
	cphy_init(phy, adapter, phy_addr, &ael1002_ops, mdio_ops,
		  SUPPORTED_10000baseT_Full | SUPPORTED_AUI | SUPPORTED_FIBRE,
		  "10GBASE-R");
	ael100x_txon(phy);
	return 0;
}

static int ael1006_reset(struct cphy *phy, int wait)
{
	return t3_phy_reset(phy, MDIO_DEV_PMA_PMD, wait);
}

static int ael1006_power_down(struct cphy *phy, int enable)
{
	return t3_mdio_change_bits(phy, MDIO_DEV_PMA_PMD, MII_BMCR,
				   BMCR_PDOWN, enable ? BMCR_PDOWN : 0);
}

#ifdef C99_NOT_SUPPORTED
static struct cphy_ops ael1006_ops = {
	ael1006_reset,
	t3_phy_lasi_intr_enable,
	t3_phy_lasi_intr_disable,
	t3_phy_lasi_intr_clear,
	t3_phy_lasi_intr_handler,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	get_link_status_r,
	ael1006_power_down,
};
#else
static struct cphy_ops ael1006_ops = {
	.reset           = ael1006_reset,
	.intr_enable     = t3_phy_lasi_intr_enable,
	.intr_disable    = t3_phy_lasi_intr_disable,
	.intr_clear      = t3_phy_lasi_intr_clear,
	.intr_handler    = t3_phy_lasi_intr_handler,
	.get_link_status = get_link_status_r,
	.power_down      = ael1006_power_down,
};
#endif

int t3_ael1006_phy_prep(struct cphy *phy, adapter_t *adapter, int phy_addr,
			const struct mdio_ops *mdio_ops)
{
	cphy_init(phy, adapter, phy_addr, &ael1006_ops, mdio_ops,
		  SUPPORTED_10000baseT_Full | SUPPORTED_AUI | SUPPORTED_FIBRE,
		  "10GBASE-SR");
	ael100x_txon(phy);
	return 0;
}

static int ael2005_setup_sr_edc(struct cphy *phy)
{
	static struct reg_val regs[] = {
		{ MDIO_DEV_PMA_PMD, 0xc003, 0xffff, 0x181 },
		{ MDIO_DEV_PMA_PMD, 0xc010, 0xffff, 0x448a },
		{ MDIO_DEV_PMA_PMD, 0xc04a, 0xffff, 0x5200 },
		{ 0, 0, 0, 0 }
	};
	static u16 sr_edc[] = {
		0xcc00, 0x2ff4,
		0xcc01, 0x3cd4,
		0xcc02, 0x2015,
		0xcc03, 0x3105,
		0xcc04, 0x6524,
		0xcc05, 0x27ff,
		0xcc06, 0x300f,
		0xcc07, 0x2c8b,
		0xcc08, 0x300b,
		0xcc09, 0x4009,
		0xcc0a, 0x400e,
		0xcc0b, 0x2f72,
		0xcc0c, 0x3002,
		0xcc0d, 0x1002,
		0xcc0e, 0x2172,
		0xcc0f, 0x3012,
		0xcc10, 0x1002,
		0xcc11, 0x25d2,
		0xcc12, 0x3012,
		0xcc13, 0x1002,
		0xcc14, 0xd01e,
		0xcc15, 0x27d2,
		0xcc16, 0x3012,
		0xcc17, 0x1002,
		0xcc18, 0x2004,
		0xcc19, 0x3c84,
		0xcc1a, 0x6436,
		0xcc1b, 0x2007,
		0xcc1c, 0x3f87,
		0xcc1d, 0x8676,
		0xcc1e, 0x40b7,
		0xcc1f, 0xa746,
		0xcc20, 0x4047,
		0xcc21, 0x5673,
		0xcc22, 0x2982,
		0xcc23, 0x3002,
		0xcc24, 0x13d2,
		0xcc25, 0x8bbd,
		0xcc26, 0x2862,
		0xcc27, 0x3012,
		0xcc28, 0x1002,
		0xcc29, 0x2092,
		0xcc2a, 0x3012,
		0xcc2b, 0x1002,
		0xcc2c, 0x5cc3,
		0xcc2d, 0x314,
		0xcc2e, 0x2942,
		0xcc2f, 0x3002,
		0xcc30, 0x1002,
		0xcc31, 0xd019,
		0xcc32, 0x2032,
		0xcc33, 0x3012,
		0xcc34, 0x1002,
		0xcc35, 0x2a04,
		0xcc36, 0x3c74,
		0xcc37, 0x6435,
		0xcc38, 0x2fa4,
		0xcc39, 0x3cd4,
		0xcc3a, 0x6624,
		0xcc3b, 0x5563,
		0xcc3c, 0x2d42,
		0xcc3d, 0x3002,
		0xcc3e, 0x13d2,
		0xcc3f, 0x464d,
		0xcc40, 0x2862,
		0xcc41, 0x3012,
		0xcc42, 0x1002,
		0xcc43, 0x2032,
		0xcc44, 0x3012,
		0xcc45, 0x1002,
		0xcc46, 0x2fb4,
		0xcc47, 0x3cd4,
		0xcc48, 0x6624,
		0xcc49, 0x5563,
		0xcc4a, 0x2d42,
		0xcc4b, 0x3002,
		0xcc4c, 0x13d2,
		0xcc4d, 0x2ed2,
		0xcc4e, 0x3002,
		0xcc4f, 0x1002,
		0xcc50, 0x2fd2,
		0xcc51, 0x3002,
		0xcc52, 0x1002,
		0xcc53, 0x004,
		0xcc54, 0x2942,
		0xcc55, 0x3002,
		0xcc56, 0x1002,
		0xcc57, 0x2092,
		0xcc58, 0x3012,
		0xcc59, 0x1002,
		0xcc5a, 0x5cc3,
		0xcc5b, 0x317,
		0xcc5c, 0x2f72,
		0xcc5d, 0x3002,
		0xcc5e, 0x1002,
		0xcc5f, 0x2942,
		0xcc60, 0x3002,
		0xcc61, 0x1002,
		0xcc62, 0x22cd,
		0xcc63, 0x301d,
		0xcc64, 0x2862,
		0xcc65, 0x3012,
		0xcc66, 0x1002,
		0xcc67, 0x2ed2,
		0xcc68, 0x3002,
		0xcc69, 0x1002,
		0xcc6a, 0x2d72,
		0xcc6b, 0x3002,
		0xcc6c, 0x1002,
		0xcc6d, 0x628f,
		0xcc6e, 0x2112,
		0xcc6f, 0x3012,
		0xcc70, 0x1002,
		0xcc71, 0x5aa3,
		0xcc72, 0x2dc2,
		0xcc73, 0x3002,
		0xcc74, 0x1312,
		0xcc75, 0x6f72,
		0xcc76, 0x1002,
		0xcc77, 0x2807,
		0xcc78, 0x31a7,
		0xcc79, 0x20c4,
		0xcc7a, 0x3c24,
		0xcc7b, 0x6724,
		0xcc7c, 0x1002,
		0xcc7d, 0x2807,
		0xcc7e, 0x3187,
		0xcc7f, 0x20c4,
		0xcc80, 0x3c24,
		0xcc81, 0x6724,
		0xcc82, 0x1002,
		0xcc83, 0x2514,
		0xcc84, 0x3c64,
		0xcc85, 0x6436,
		0xcc86, 0xdff4,
		0xcc87, 0x6436,
		0xcc88, 0x1002,
		0xcc89, 0x40a4,
		0xcc8a, 0x643c,
		0xcc8b, 0x4016,
		0xcc8c, 0x8c6c,
		0xcc8d, 0x2b24,
		0xcc8e, 0x3c24,
		0xcc8f, 0x6435,
		0xcc90, 0x1002,
		0xcc91, 0x2b24,
		0xcc92, 0x3c24,
		0xcc93, 0x643a,
		0xcc94, 0x4025,
		0xcc95, 0x8a5a,
		0xcc96, 0x1002,
		0xcc97, 0x2731,
		0xcc98, 0x3011,
		0xcc99, 0x1001,
		0xcc9a, 0xc7a0,
		0xcc9b, 0x100,
		0xcc9c, 0xc502,
		0xcc9d, 0x53ac,
		0xcc9e, 0xc503,
		0xcc9f, 0xd5d5,
		0xcca0, 0xc600,
		0xcca1, 0x2a6d,
		0xcca2, 0xc601,
		0xcca3, 0x2a4c,
		0xcca4, 0xc602,
		0xcca5, 0x111,
		0xcca6, 0xc60c,
		0xcca7, 0x5900,
		0xcca8, 0xc710,
		0xcca9, 0x700,
		0xccaa, 0xc718,
		0xccab, 0x700,
		0xccac, 0xc720,
		0xccad, 0x4700,
		0xccae, 0xc801,
		0xccaf, 0x7f50,
		0xccb0, 0xc802,
		0xccb1, 0x7760,
		0xccb2, 0xc803,
		0xccb3, 0x7fce,
		0xccb4, 0xc804,
		0xccb5, 0x5700,
		0xccb6, 0xc805,
		0xccb7, 0x5f11,
		0xccb8, 0xc806,
		0xccb9, 0x4751,
		0xccba, 0xc807,
		0xccbb, 0x57e1,
		0xccbc, 0xc808,
		0xccbd, 0x2700,
		0xccbe, 0xc809,
		0xccbf, 0x000,
		0xccc0, 0xc821,
		0xccc1, 0x002,
		0xccc2, 0xc822,
		0xccc3, 0x014,
		0xccc4, 0xc832,
		0xccc5, 0x1186,
		0xccc6, 0xc847,
		0xccc7, 0x1e02,
		0xccc8, 0xc013,
		0xccc9, 0xf341,
		0xccca, 0xc01a,
		0xcccb, 0x446,
		0xcccc, 0xc024,
		0xcccd, 0x1000,
		0xccce, 0xc025,
		0xcccf, 0xa00,
		0xccd0, 0xc026,
		0xccd1, 0xc0c,
		0xccd2, 0xc027,
		0xccd3, 0xc0c,
		0xccd4, 0xc029,
		0xccd5, 0x0a0,
		0xccd6, 0xc030,
		0xccd7, 0xa00,
		0xccd8, 0xc03c,
		0xccd9, 0x01c,
		0xccda, 0xc005,
		0xccdb, 0x7a06,
		0xccdc, 0x000,
		0xccdd, 0x2731,
		0xccde, 0x3011,
		0xccdf, 0x1001,
		0xcce0, 0xc620,
		0xcce1, 0x000,
		0xcce2, 0xc621,
		0xcce3, 0x03f,
		0xcce4, 0xc622,
		0xcce5, 0x000,
		0xcce6, 0xc623,
		0xcce7, 0x000,
		0xcce8, 0xc624,
		0xcce9, 0x000,
		0xccea, 0xc625,
		0xcceb, 0x000,
		0xccec, 0xc627,
		0xcced, 0x000,
		0xccee, 0xc628,
		0xccef, 0x000,
		0xccf0, 0xc62c,
		0xccf1, 0x000,
		0xccf2, 0x000,
		0xccf3, 0x2806,
		0xccf4, 0x3cb6,
		0xccf5, 0xc161,
		0xccf6, 0x6134,
		0xccf7, 0x6135,
		0xccf8, 0x5443,
		0xccf9, 0x303,
		0xccfa, 0x6524,
		0xccfb, 0x00b,
		0xccfc, 0x1002,
		0xccfd, 0x2104,
		0xccfe, 0x3c24,
		0xccff, 0x2105,
		0xcd00, 0x3805,
		0xcd01, 0x6524,
		0xcd02, 0xdff4,
		0xcd03, 0x4005,
		0xcd04, 0x6524,
		0xcd05, 0x1002,
		0xcd06, 0x5dd3,
		0xcd07, 0x306,
		0xcd08, 0x2ff7,
		0xcd09, 0x38f7,
		0xcd0a, 0x60b7,
		0xcd0b, 0xdffd,
		0xcd0c, 0x00a,
		0xcd0d, 0x1002,
		0xcd0e, 0
	};
	int i, err;

	err = set_phy_regs(phy, regs);
	if (err)
		return err;

	msleep(50);

	for (i = 0; i < ARRAY_SIZE(sr_edc) && !err; i += 2)
		err = mdio_write(phy, MDIO_DEV_PMA_PMD, sr_edc[i],
				 sr_edc[i + 1]);
	if (!err)
		phy->priv = edc_sr;
	return err;
}

static int ael2005_setup_twinax_edc(struct cphy *phy, int modtype)
{
	static struct reg_val regs[] = {
		{ MDIO_DEV_PMA_PMD, 0xc04a, 0xffff, 0x5a00 },
		{ 0, 0, 0, 0 }
	};
	static struct reg_val preemphasis[] = {
		{ MDIO_DEV_PMA_PMD, 0xc014, 0xffff, 0xfe16 },
		{ MDIO_DEV_PMA_PMD, 0xc015, 0xffff, 0xa000 },
		{ 0, 0, 0, 0 }
	};
	static u16 twinax_edc[] = {
		0xcc00, 0x4009,
		0xcc01, 0x27ff,
		0xcc02, 0x300f,
		0xcc03, 0x40aa,
		0xcc04, 0x401c,
		0xcc05, 0x401e,
		0xcc06, 0x2ff4,
		0xcc07, 0x3cd4,
		0xcc08, 0x2035,
		0xcc09, 0x3145,
		0xcc0a, 0x6524,
		0xcc0b, 0x26a2,
		0xcc0c, 0x3012,
		0xcc0d, 0x1002,
		0xcc0e, 0x29c2,
		0xcc0f, 0x3002,
		0xcc10, 0x1002,
		0xcc11, 0x2072,
		0xcc12, 0x3012,
		0xcc13, 0x1002,
		0xcc14, 0x22cd,
		0xcc15, 0x301d,
		0xcc16, 0x2e52,
		0xcc17, 0x3012,
		0xcc18, 0x1002,
		0xcc19, 0x28e2,
		0xcc1a, 0x3002,
		0xcc1b, 0x1002,
		0xcc1c, 0x628f,
		0xcc1d, 0x2ac2,
		0xcc1e, 0x3012,
		0xcc1f, 0x1002,
		0xcc20, 0x5553,
		0xcc21, 0x2ae2,
		0xcc22, 0x3002,
		0xcc23, 0x1302,
		0xcc24, 0x401e,
		0xcc25, 0x2be2,
		0xcc26, 0x3012,
		0xcc27, 0x1002,
		0xcc28, 0x2da2,
		0xcc29, 0x3012,
		0xcc2a, 0x1002,
		0xcc2b, 0x2ba2,
		0xcc2c, 0x3002,
		0xcc2d, 0x1002,
		0xcc2e, 0x5ee3,
		0xcc2f, 0x305,
		0xcc30, 0x400e,
		0xcc31, 0x2bc2,
		0xcc32, 0x3002,
		0xcc33, 0x1002,
		0xcc34, 0x2b82,
		0xcc35, 0x3012,
		0xcc36, 0x1002,
		0xcc37, 0x5663,
		0xcc38, 0x302,
		0xcc39, 0x401e,
		0xcc3a, 0x6f72,
		0xcc3b, 0x1002,
		0xcc3c, 0x628f,
		0xcc3d, 0x2be2,
		0xcc3e, 0x3012,
		0xcc3f, 0x1002,
		0xcc40, 0x22cd,
		0xcc41, 0x301d,
		0xcc42, 0x2e52,
		0xcc43, 0x3012,
		0xcc44, 0x1002,
		0xcc45, 0x2522,
		0xcc46, 0x3012,
		0xcc47, 0x1002,
		0xcc48, 0x2da2,
		0xcc49, 0x3012,
		0xcc4a, 0x1002,
		0xcc4b, 0x2ca2,
		0xcc4c, 0x3012,
		0xcc4d, 0x1002,
		0xcc4e, 0x2fa4,
		0xcc4f, 0x3cd4,
		0xcc50, 0x6624,
		0xcc51, 0x410b,
		0xcc52, 0x56b3,
		0xcc53, 0x3c4,
		0xcc54, 0x2fb2,
		0xcc55, 0x3002,
		0xcc56, 0x1002,
		0xcc57, 0x220b,
		0xcc58, 0x303b,
		0xcc59, 0x56b3,
		0xcc5a, 0x3c3,
		0xcc5b, 0x866b,
		0xcc5c, 0x400c,
		0xcc5d, 0x23a2,
		0xcc5e, 0x3012,
		0xcc5f, 0x1002,
		0xcc60, 0x2da2,
		0xcc61, 0x3012,
		0xcc62, 0x1002,
		0xcc63, 0x2ca2,
		0xcc64, 0x3012,
		0xcc65, 0x1002,
		0xcc66, 0x2fb4,
		0xcc67, 0x3cd4,
		0xcc68, 0x6624,
		0xcc69, 0x56b3,
		0xcc6a, 0x3c3,
		0xcc6b, 0x866b,
		0xcc6c, 0x401c,
		0xcc6d, 0x2205,
		0xcc6e, 0x3035,
		0xcc6f, 0x5b53,
		0xcc70, 0x2c52,
		0xcc71, 0x3002,
		0xcc72, 0x13c2,
		0xcc73, 0x5cc3,
		0xcc74, 0x317,
		0xcc75, 0x2522,
		0xcc76, 0x3012,
		0xcc77, 0x1002,
		0xcc78, 0x2da2,
		0xcc79, 0x3012,
		0xcc7a, 0x1002,
		0xcc7b, 0x2b82,
		0xcc7c, 0x3012,
		0xcc7d, 0x1002,
		0xcc7e, 0x5663,
		0xcc7f, 0x303,
		0xcc80, 0x401e,
		0xcc81, 0x004,
		0xcc82, 0x2c42,
		0xcc83, 0x3012,
		0xcc84, 0x1002,
		0xcc85, 0x6f72,
		0xcc86, 0x1002,
		0xcc87, 0x628f,
		0xcc88, 0x2304,
		0xcc89, 0x3c84,
		0xcc8a, 0x6436,
		0xcc8b, 0xdff4,
		0xcc8c, 0x6436,
		0xcc8d, 0x2ff5,
		0xcc8e, 0x3005,
		0xcc8f, 0x8656,
		0xcc90, 0xdfba,
		0xcc91, 0x56a3,
		0xcc92, 0xd05a,
		0xcc93, 0x21c2,
		0xcc94, 0x3012,
		0xcc95, 0x1392,
		0xcc96, 0xd05a,
		0xcc97, 0x56a3,
		0xcc98, 0xdfba,
		0xcc99, 0x383,
		0xcc9a, 0x6f72,
		0xcc9b, 0x1002,
		0xcc9c, 0x28c5,
		0xcc9d, 0x3005,
		0xcc9e, 0x4178,
		0xcc9f, 0x5653,
		0xcca0, 0x384,
		0xcca1, 0x22b2,
		0xcca2, 0x3012,
		0xcca3, 0x1002,
		0xcca4, 0x2be5,
		0xcca5, 0x3005,
		0xcca6, 0x41e8,
		0xcca7, 0x5653,
		0xcca8, 0x382,
		0xcca9, 0x002,
		0xccaa, 0x4258,
		0xccab, 0x2474,
		0xccac, 0x3c84,
		0xccad, 0x6437,
		0xccae, 0xdff4,
		0xccaf, 0x6437,
		0xccb0, 0x2ff5,
		0xccb1, 0x3c05,
		0xccb2, 0x8757,
		0xccb3, 0xb888,
		0xccb4, 0x9787,
		0xccb5, 0xdff4,
		0xccb6, 0x6724,
		0xccb7, 0x866a,
		0xccb8, 0x6f72,
		0xccb9, 0x1002,
		0xccba, 0x2d01,
		0xccbb, 0x3011,
		0xccbc, 0x1001,
		0xccbd, 0xc620,
		0xccbe, 0x14e5,
		0xccbf, 0xc621,
		0xccc0, 0xc53d,
		0xccc1, 0xc622,
		0xccc2, 0x3cbe,
		0xccc3, 0xc623,
		0xccc4, 0x4452,
		0xccc5, 0xc624,
		0xccc6, 0xc5c5,
		0xccc7, 0xc625,
		0xccc8, 0xe01e,
		0xccc9, 0xc627,
		0xccca, 0x000,
		0xcccb, 0xc628,
		0xcccc, 0x000,
		0xcccd, 0xc62b,
		0xccce, 0x000,
		0xcccf, 0xc62c,
		0xccd0, 0x000,
		0xccd1, 0x000,
		0xccd2, 0x2d01,
		0xccd3, 0x3011,
		0xccd4, 0x1001,
		0xccd5, 0xc620,
		0xccd6, 0x000,
		0xccd7, 0xc621,
		0xccd8, 0x000,
		0xccd9, 0xc622,
		0xccda, 0x0ce,
		0xccdb, 0xc623,
		0xccdc, 0x07f,
		0xccdd, 0xc624,
		0xccde, 0x032,
		0xccdf, 0xc625,
		0xcce0, 0x000,
		0xcce1, 0xc627,
		0xcce2, 0x000,
		0xcce3, 0xc628,
		0xcce4, 0x000,
		0xcce5, 0xc62b,
		0xcce6, 0x000,
		0xcce7, 0xc62c,
		0xcce8, 0x000,
		0xcce9, 0x000,
		0xccea, 0x2d01,
		0xcceb, 0x3011,
		0xccec, 0x1001,
		0xcced, 0xc502,
		0xccee, 0x609f,
		0xccef, 0xc600,
		0xccf0, 0x2a6e,
		0xccf1, 0xc601,
		0xccf2, 0x2a2c,
		0xccf3, 0xc60c,
		0xccf4, 0x5400,
		0xccf5, 0xc710,
		0xccf6, 0x700,
		0xccf7, 0xc718,
		0xccf8, 0x700,
		0xccf9, 0xc720,
		0xccfa, 0x4700,
		0xccfb, 0xc728,
		0xccfc, 0x700,
		0xccfd, 0xc729,
		0xccfe, 0x1207,
		0xccff, 0xc801,
		0xcd00, 0x7f50,
		0xcd01, 0xc802,
		0xcd02, 0x7760,
		0xcd03, 0xc803,
		0xcd04, 0x7fce,
		0xcd05, 0xc804,
		0xcd06, 0x520e,
		0xcd07, 0xc805,
		0xcd08, 0x5c11,
		0xcd09, 0xc806,
		0xcd0a, 0x3c51,
		0xcd0b, 0xc807,
		0xcd0c, 0x4061,
		0xcd0d, 0xc808,
		0xcd0e, 0x49c1,
		0xcd0f, 0xc809,
		0xcd10, 0x3840,
		0xcd11, 0xc80a,
		0xcd12, 0x000,
		0xcd13, 0xc821,
		0xcd14, 0x002,
		0xcd15, 0xc822,
		0xcd16, 0x046,
		0xcd17, 0xc844,
		0xcd18, 0x182f,
		0xcd19, 0xc013,
		0xcd1a, 0xf341,
		0xcd1b, 0xc01a,
		0xcd1c, 0x446,
		0xcd1d, 0xc024,
		0xcd1e, 0x1000,
		0xcd1f, 0xc025,
		0xcd20, 0xa00,
		0xcd21, 0xc026,
		0xcd22, 0xc0c,
		0xcd23, 0xc027,
		0xcd24, 0xc0c,
		0xcd25, 0xc029,
		0xcd26, 0x0a0,
		0xcd27, 0xc030,
		0xcd28, 0xa00,
		0xcd29, 0xc03c,
		0xcd2a, 0x01c,
		0xcd2b, 0x000,
		0xcd2c, 0x2b84,
		0xcd2d, 0x3c74,
		0xcd2e, 0x6435,
		0xcd2f, 0xdff4,
		0xcd30, 0x6435,
		0xcd31, 0x2806,
		0xcd32, 0x3006,
		0xcd33, 0x8565,
		0xcd34, 0x2b24,
		0xcd35, 0x3c24,
		0xcd36, 0x6436,
		0xcd37, 0x1002,
		0xcd38, 0x2b24,
		0xcd39, 0x3c24,
		0xcd3a, 0x6436,
		0xcd3b, 0x4045,
		0xcd3c, 0x8656,
		0xcd3d, 0x1002,
		0xcd3e, 0x2807,
		0xcd3f, 0x31a7,
		0xcd40, 0x20c4,
		0xcd41, 0x3c24,
		0xcd42, 0x6724,
		0xcd43, 0x1002,
		0xcd44, 0x2807,
		0xcd45, 0x3187,
		0xcd46, 0x20c4,
		0xcd47, 0x3c24,
		0xcd48, 0x6724,
		0xcd49, 0x1002,
		0xcd4a, 0x2514,
		0xcd4b, 0x3c64,
		0xcd4c, 0x6436,
		0xcd4d, 0xdff4,
		0xcd4e, 0x6436,
		0xcd4f, 0x1002,
		0xcd50, 0x2806,
		0xcd51, 0x3cb6,
		0xcd52, 0xc161,
		0xcd53, 0x6134,
		0xcd54, 0x6135,
		0xcd55, 0x5443,
		0xcd56, 0x303,
		0xcd57, 0x6524,
		0xcd58, 0x00b,
		0xcd59, 0x1002,
		0xcd5a, 0xd019,
		0xcd5b, 0x2104,
		0xcd5c, 0x3c24,
		0xcd5d, 0x2105,
		0xcd5e, 0x3805,
		0xcd5f, 0x6524,
		0xcd60, 0xdff4,
		0xcd61, 0x4005,
		0xcd62, 0x6524,
		0xcd63, 0x2e8d,
		0xcd64, 0x303d,
		0xcd65, 0x5dd3,
		0xcd66, 0x306,
		0xcd67, 0x2ff7,
		0xcd68, 0x38f7,
		0xcd69, 0x60b7,
		0xcd6a, 0xdffd,
		0xcd6b, 0x00a,
		0xcd6c, 0x1002,
		0xcd6d, 0
	};
	int i, err;

	err = set_phy_regs(phy, regs);
	if (!err && modtype == phy_modtype_twinax_long)
		err = set_phy_regs(phy, preemphasis);
	if (err)
		return err;

	msleep(50);

	for (i = 0; i < ARRAY_SIZE(twinax_edc) && !err; i += 2)
		err = mdio_write(phy, MDIO_DEV_PMA_PMD, twinax_edc[i],
				 twinax_edc[i + 1]);
	if (!err)
		phy->priv = edc_twinax;
	return err;
}

static int ael2005_i2c_rd(struct cphy *phy, int dev_addr, int word_addr)
{
	int i, err;
	unsigned int stat, data;

	err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL_I2C_CTRL,
			 (dev_addr << 8) | (1 << 8) | word_addr);
	if (err)
		return err;

	for (i = 0; i < 5; i++) {
		msleep(1);
		err = mdio_read(phy, MDIO_DEV_PMA_PMD, AEL_I2C_STAT, &stat);
		if (err)
			return err;
		if ((stat & 3) == 1) {
			err = mdio_read(phy, MDIO_DEV_PMA_PMD, AEL_I2C_DATA,
					&data);
			if (err)
				return err;
			return data >> 8;
		}
	}
	CH_WARN(phy->adapter, "PHY %u I2C read of addr %u timed out\n",
		phy->addr, word_addr);
	return -ETIMEDOUT;
}

static int get_module_type(struct cphy *phy, int delay_ms)
{
	int v;
	unsigned int stat;

	v = mdio_read(phy, MDIO_DEV_PMA_PMD, AEL2005_GPIO_CTRL, &stat);
	if (v)
		return v;

	if (stat & (1 << 8))			/* module absent */
		return phy_modtype_none;

	if (delay_ms)
		msleep(delay_ms);

	/* see SFF-8472 for below */
	v = ael2005_i2c_rd(phy, MODULE_DEV_ADDR, 3);
	if (v < 0)
		return v;

	if (v == 0x10)
		return phy_modtype_sr;
	if (v == 0x20)
		return phy_modtype_lr;
	if (v == 0x40)
		return phy_modtype_lrm;

	v = ael2005_i2c_rd(phy, MODULE_DEV_ADDR, 6);
	if (v < 0)
		return v;
	if (v != 4)
		goto unknown;

	v = ael2005_i2c_rd(phy, MODULE_DEV_ADDR, 10);
	if (v < 0)
		return v;

	if (v & 0x80) {
		v = ael2005_i2c_rd(phy, MODULE_DEV_ADDR, 0x12);
		if (v < 0)
			return v;
		return v > 10 ? phy_modtype_twinax_long : phy_modtype_twinax;
	}
unknown:
	return phy_modtype_unknown;
}

static int ael2005_intr_enable(struct cphy *phy)
{
	int err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL2005_GPIO_CTRL, 0x200);
	return err ? err : t3_phy_lasi_intr_enable(phy);
}

static int ael2005_intr_disable(struct cphy *phy)
{
	int err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL2005_GPIO_CTRL, 0x100);
	return err ? err : t3_phy_lasi_intr_disable(phy);
}

static int ael2005_intr_clear(struct cphy *phy)
{
	int err = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL2005_GPIO_CTRL, 0xd00);
	return err ? err : t3_phy_lasi_intr_clear(phy);
}

static int ael2005_reset(struct cphy *phy, int wait)
{
	static struct reg_val regs0[] = {
		{ MDIO_DEV_PMA_PMD, 0xc001, 0, 1 << 5 },
		{ MDIO_DEV_PMA_PMD, 0xc017, 0, 1 << 5 },
		{ MDIO_DEV_PMA_PMD, 0xc013, 0xffff, 0xf341 },
		{ MDIO_DEV_PMA_PMD, 0xc210, 0xffff, 0x8000 },
		{ MDIO_DEV_PMA_PMD, 0xc210, 0xffff, 0x8100 },
		{ MDIO_DEV_PMA_PMD, 0xc210, 0xffff, 0x8000 },
		{ MDIO_DEV_PMA_PMD, 0xc210, 0xffff, 0 },
		{ 0, 0, 0, 0 }
	};
	static struct reg_val regs1[] = {
		{ MDIO_DEV_PMA_PMD, 0xca00, 0xffff, 0x0080 },
		{ MDIO_DEV_PMA_PMD, 0xca12, 0xffff, 0 },
		{ 0, 0, 0, 0 }
	};

	int err, lasi_ctrl;

	err = mdio_read(phy, MDIO_DEV_PMA_PMD, LASI_CTRL, &lasi_ctrl);
	if (err)
		return err;

	err = t3_phy_reset(phy, MDIO_DEV_PMA_PMD, 0);
	if (err)
		return err;

	msleep(125);
	phy->priv = edc_none;
	err = set_phy_regs(phy, regs0);
	if (err)
		return err;

	msleep(50);

	err = get_module_type(phy, 0);
	if (err < 0)
		return err;
	phy->modtype = (u8)err;

	if (err == phy_modtype_twinax || err == phy_modtype_twinax_long)
		err = ael2005_setup_twinax_edc(phy, err);
	else
		err = ael2005_setup_sr_edc(phy);
	if (err)
		return err;

	err = set_phy_regs(phy, regs1);
	if (err)
		return err;

	/* reset wipes out interrupts, reenable them if they were on */
	if (lasi_ctrl & 1)
		err = ael2005_intr_enable(phy);
	return err;
}

static int ael2005_intr_handler(struct cphy *phy)
{
	unsigned int stat;
	int ret, edc_needed, cause = 0;

	ret = mdio_read(phy, MDIO_DEV_PMA_PMD, AEL2005_GPIO_STAT, &stat);
	if (ret)
		return ret;

	if (stat & AEL2005_MODDET_IRQ) {
		ret = mdio_write(phy, MDIO_DEV_PMA_PMD, AEL2005_GPIO_CTRL,
				 0xd00);
		if (ret)
			return ret;

		/* modules have max 300 ms init time after hot plug */
		ret = get_module_type(phy, 300);
		if (ret < 0)
			return ret;

		phy->modtype = (u8)ret;
		if (ret == phy_modtype_none)
			edc_needed = phy->priv;       /* on unplug retain EDC */
		else if (ret == phy_modtype_twinax ||
			 ret == phy_modtype_twinax_long)
			edc_needed = edc_twinax;
		else
			edc_needed = edc_sr;

		if (edc_needed != phy->priv) {
			ret = ael2005_reset(phy, 0);
			return ret ? ret : cphy_cause_module_change;
		}
		cause = cphy_cause_module_change;
	}

	ret = t3_phy_lasi_intr_handler(phy);
	return ret < 0 ? ret : ret + cause;
}

#ifdef C99_NOT_SUPPORTED
static struct cphy_ops ael2005_ops = {
	ael2005_reset,
	ael2005_intr_enable,
	ael2005_intr_disable,
	ael2005_intr_clear,
	ael2005_intr_handler,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	get_link_status_r,
	ael1002_power_down,
};
#else
static struct cphy_ops ael2005_ops = {
	.reset           = ael2005_reset,
	.intr_enable     = ael2005_intr_enable,
	.intr_disable    = ael2005_intr_disable,
	.intr_clear      = ael2005_intr_clear,
	.intr_handler    = ael2005_intr_handler,
	.get_link_status = get_link_status_r,
	.power_down      = ael1002_power_down,
};
#endif

int t3_ael2005_phy_prep(struct cphy *phy, adapter_t *adapter, int phy_addr,
			const struct mdio_ops *mdio_ops)
{
	cphy_init(phy, adapter, phy_addr, &ael2005_ops, mdio_ops,
		  SUPPORTED_10000baseT_Full | SUPPORTED_AUI | SUPPORTED_FIBRE |
		  SUPPORTED_IRQ, "10GBASE-R");
	msleep(125);
	return t3_mdio_change_bits(phy, MDIO_DEV_PMA_PMD, AEL_OPT_SETTINGS, 0,
				   1 << 5);
}

/*
 * Get link status for a 10GBASE-X device.
 */
static int get_link_status_x(struct cphy *phy, int *link_ok, int *speed,
			     int *duplex, int *fc)
{
	if (link_ok) {
		unsigned int stat0, stat1, stat2;
		int err = mdio_read(phy, MDIO_DEV_PMA_PMD, PMD_RSD, &stat0);

		if (!err)
			err = mdio_read(phy, MDIO_DEV_PCS, PCS_STAT1_X, &stat1);
		if (!err)
			err = mdio_read(phy, MDIO_DEV_XGXS, XS_LN_STAT, &stat2);
		if (err)
			return err;
		*link_ok = (stat0 & (stat1 >> 12) & (stat2 >> 12)) & 1;
	}
	if (speed)
		*speed = SPEED_10000;
	if (duplex)
		*duplex = DUPLEX_FULL;
	return 0;
}

#ifdef C99_NOT_SUPPORTED
static struct cphy_ops qt2045_ops = {
	ael1006_reset,
	t3_phy_lasi_intr_enable,
	t3_phy_lasi_intr_disable,
	t3_phy_lasi_intr_clear,
	t3_phy_lasi_intr_handler,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	get_link_status_x,
	ael1006_power_down,
};
#else
static struct cphy_ops qt2045_ops = {
	.reset           = ael1006_reset,
	.intr_enable     = t3_phy_lasi_intr_enable,
	.intr_disable    = t3_phy_lasi_intr_disable,
	.intr_clear      = t3_phy_lasi_intr_clear,
	.intr_handler    = t3_phy_lasi_intr_handler,
	.get_link_status = get_link_status_x,
	.power_down      = ael1006_power_down,
};
#endif

int t3_qt2045_phy_prep(struct cphy *phy, adapter_t *adapter, int phy_addr,
		       const struct mdio_ops *mdio_ops)
{
	unsigned int stat;

	cphy_init(phy, adapter, phy_addr, &qt2045_ops, mdio_ops,
		  SUPPORTED_10000baseT_Full | SUPPORTED_AUI | SUPPORTED_TP,
		  "10GBASE-CX4");

	/*
	 * Some cards where the PHY is supposed to be at address 0 actually
	 * have it at 1.
	 */
	if (!phy_addr && !mdio_read(phy, MDIO_DEV_PMA_PMD, MII_BMSR, &stat) &&
	    stat == 0xffff)
		phy->addr = 1;
	return 0;
}

static int xaui_direct_reset(struct cphy *phy, int wait)
{
	return 0;
}

static int xaui_direct_get_link_status(struct cphy *phy, int *link_ok,
				       int *speed, int *duplex, int *fc)
{
	if (link_ok) {
		unsigned int status;
		
		status = t3_read_reg(phy->adapter,
				     XGM_REG(A_XGM_SERDES_STAT0, phy->addr)) |
			 t3_read_reg(phy->adapter,
				     XGM_REG(A_XGM_SERDES_STAT1, phy->addr)) |
			 t3_read_reg(phy->adapter,
				     XGM_REG(A_XGM_SERDES_STAT2, phy->addr)) |
			 t3_read_reg(phy->adapter,
				     XGM_REG(A_XGM_SERDES_STAT3, phy->addr));
		*link_ok = !(status & F_LOWSIG0);
	}
	if (speed)
		*speed = SPEED_10000;
	if (duplex)
		*duplex = DUPLEX_FULL;
	return 0;
}

static int xaui_direct_power_down(struct cphy *phy, int enable)
{
	return 0;
}

#ifdef C99_NOT_SUPPORTED
static struct cphy_ops xaui_direct_ops = {
	xaui_direct_reset,
	ael1002_intr_noop,
	ael1002_intr_noop,
	ael1002_intr_noop,
	ael1002_intr_noop,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	xaui_direct_get_link_status,
	xaui_direct_power_down,
};
#else
static struct cphy_ops xaui_direct_ops = {
	.reset           = xaui_direct_reset,
	.intr_enable     = ael1002_intr_noop,
	.intr_disable    = ael1002_intr_noop,
	.intr_clear      = ael1002_intr_noop,
	.intr_handler    = ael1002_intr_noop,
	.get_link_status = xaui_direct_get_link_status,
	.power_down      = xaui_direct_power_down,
};
#endif

int t3_xaui_direct_phy_prep(struct cphy *phy, adapter_t *adapter, int phy_addr,
			    const struct mdio_ops *mdio_ops)
{
	cphy_init(phy, adapter, phy_addr, &xaui_direct_ops, mdio_ops,
		  SUPPORTED_10000baseT_Full | SUPPORTED_AUI | SUPPORTED_TP,
		  "10GBASE-CX4");
	return 0;
}
