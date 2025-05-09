/****************************************************************************
 * drivers/wireless/ieee80211/bcm43xxx/bcmf_sdio_regs.h
 *
 * SPDX-License-Identifier: ISC
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 ****************************************************************************/

#ifndef __DRIVERS_WIRELESS_IEEE80211_BCM43XXX_BCMF_SDIO_REGS_H
#define __DRIVERS_WIRELESS_IEEE80211_BCM43XXX_BCMF_SDIO_REGS_H

#define SDIO_FUNC_0        0
#define SDIO_FUNC_1        1
#define SDIO_FUNC_2        2

#define SDIOD_FBR_SIZE     0x100

/* io_en */

#define SDIO_FUNC_ENABLE_1 0x02
#define SDIO_FUNC_ENABLE_2 0x04

/* io_rdys */

#define SDIO_FUNC_READY_1  0x02
#define SDIO_FUNC_READY_2  0x04

/* intr_status */

#define INTR_STATUS_FUNC1  0x2
#define INTR_STATUS_FUNC2  0x4

/* Maximum number of I/O funcs */

#define SDIOD_MAX_IOFUNCS  7

/* mask of register map */

#define REG_F0_REG_MASK    0x7FF
#define REG_F1_MISC_MASK   0x1FFFF

/* as of sdiod rev 0, supports 3 functions */

#define SBSDIO_NUM_FUNCTION 3

/* function 0 vendor specific CCCR registers */

#define SDIO_CCCR_BRCM_CARDCAP               0xf0
#define SDIO_CCCR_BRCM_CARDCAP_CMD14_SUPPORT 0x02
#define SDIO_CCCR_BRCM_CARDCAP_CMD14_EXT     0x04
#define SDIO_CCCR_BRCM_CARDCAP_CMD_NODEC     0x08
#define SDIO_CCCR_BRCM_CARDCTRL              0xf1
#define SDIO_CCCR_BRCM_CARDCTRL_WLANRESET    0x02
#define SDIO_CCCR_BRCM_SEPINT                0xf2

#define  SDIO_SEPINT_MASK   0x01
#define  SDIO_SEPINT_OE     0x02
#define  SDIO_SEPINT_ACT_HI 0x04

/* function 1 miscellaneous registers */

/* sprom command and status */

#define SBSDIO_SPROM_CS         0x10000

/* sprom info register */

#define SBSDIO_SPROM_INFO       0x10001

/* sprom indirect access data byte 0 */

#define SBSDIO_SPROM_DATA_LOW   0x10002

/* sprom indirect access data byte 1 */

#define SBSDIO_SPROM_DATA_HIGH  0x10003

/* sprom indirect access addr byte 0 */

#define SBSDIO_SPROM_ADDR_LOW   0x10004

/* sprom indirect access addr byte 0 */

#define SBSDIO_SPROM_ADDR_HIGH  0x10005

/* xtal_pu (gpio) output */

#define SBSDIO_CHIP_CTRL_DATA   0x10006

/* xtal_pu (gpio) enable */

#define SBSDIO_CHIP_CTRL_EN     0x10007

/* rev < 7, watermark for sdio device */

#define SBSDIO_WATERMARK        0x10008

/* control busy signal generation */

#define SBSDIO_DEVICE_CTL       0x10009

/* SB Address Window Low (b15) */

#define SBSDIO_FUNC1_SBADDRLOW  0x1000A

/* SB Address Window Mid (b23:b16) */

#define SBSDIO_FUNC1_SBADDRMID  0x1000B

/* SB Address Window High (b31:b24) */

#define SBSDIO_FUNC1_SBADDRHIGH 0x1000C

/* Frame Control (frame term/abort) */

#define SBSDIO_FUNC1_FRAMECTRL  0x1000D

/* Read Frame Terminate */

#define SFC_RF_TERM  (1 << 0)

/* Write Frame Terminate */

#define SFC_WF_TERM  (1 << 1)

/* CRC error for write out of sync */

#define SFC_CRC4WOOS (1 << 2)

/* Abort all in-progress frames */

#define SFC_ABORTALL (1 << 3)

/* ChipClockCSR (ALP/HT ctl/status) */

#define SBSDIO_FUNC1_CHIPCLKCSR 0x1000E

/* Force ALP request to backplane */

#define SBSDIO_FORCE_ALP           0x01

/* Force HT request to backplane */

#define SBSDIO_FORCE_HT            0x02

/* Force ILP request to backplane */

#define SBSDIO_FORCE_ILP           0x04

/* Make ALP ready (power up xtal) */

#define SBSDIO_ALP_AVAIL_REQ       0x08

/* Make HT ready (power up PLL) */

#define SBSDIO_HT_AVAIL_REQ        0x10

/* Squelch clock requests from HW */

#define SBSDIO_FORCE_HW_CLKREQ_OFF 0x20

/* Status: ALP is ready */

#define SBSDIO_ALP_AVAIL           0x40

/* Status: HT is ready */

#define SBSDIO_HT_AVAIL            0x80

/* SdioPullUp (on cmd, d0-d2) */

#define SBSDIO_FUNC1_SDIOPULLUP  0x1000F

/* Write Frame Byte Count Low */

#define SBSDIO_FUNC1_WFRAMEBCLO  0x10019

/* Write Frame Byte Count High */

#define SBSDIO_FUNC1_WFRAMEBCHI  0x1001A

/* Read Frame Byte Count Low */

#define SBSDIO_FUNC1_RFRAMEBCLO  0x1001B

/* Read Frame Byte Count High */

#define SBSDIO_FUNC1_RFRAMEBCHI  0x1001C

/* MesBusyCtl (rev 11) */

#define SBSDIO_FUNC1_MESBUSYCTRL 0x1001D

/* Sdio Core Rev 12 */

#define SBSDIO_FUNC1_WAKEUPCTRL  0x1001E
#define SBSDIO_FUNC1_WCTRL_ALPWAIT_MASK  0x1
#define SBSDIO_FUNC1_WCTRL_ALPWAIT_SHIFT 0
#define SBSDIO_FUNC1_WCTRL_HTWAIT_MASK   0x2
#define SBSDIO_FUNC1_WCTRL_HTWAIT_SHIFT  1

#define SBSDIO_FUNC1_SLEEPCSR    0x1001F
#define SBSDIO_FUNC1_SLEEPCSR_KSO_MASK    0x1
#define SBSDIO_FUNC1_SLEEPCSR_KSO_SHIFT   0
#define SBSDIO_FUNC1_SLEEPCSR_KSO_EN      1
#define SBSDIO_FUNC1_SLEEPCSR_DEVON_MASK  0x2
#define SBSDIO_FUNC1_SLEEPCSR_DEVON_SHIFT 1

#define SBSDIO_AVBITS          (SBSDIO_HT_AVAIL | SBSDIO_ALP_AVAIL)
#define SBSDIO_ALPAV(regval)   ((regval) & SBSDIO_AVBITS)
#define SBSDIO_HTAV(regval)    (((regval) & SBSDIO_AVBITS) == SBSDIO_AVBITS)
#define SBSDIO_ALPONLY(regval) (SBSDIO_ALPAV(regval) && !SBSDIO_HTAV(regval))
#define SBSDIO_CLKAV(regval, alponly) \
    (SBSDIO_ALPAV(regval) && (alponly ? 1 : SBSDIO_HTAV(regval)))

#define SBSDIO_FUNC1_MISC_REG_START 0x10000 /* f1 misc register start */
#define SBSDIO_FUNC1_MISC_REG_LIMIT 0x1001F /* f1 misc register end */

/* function 1 OCP space */

/* sb offset addr is <= 15 bits, 32k */

#define SBSDIO_SB_OFT_ADDR_MASK    0x07FFF
#define SBSDIO_SB_OFT_ADDR_LIMIT   0x08000

/* with b15, maps to 32-bit SB access */

#define SBSDIO_SB_ACCESS_2_4B_FLAG 0x08000

/* valid bits in SBSDIO_FUNC1_SBADDRxxx regs */

#define SBSDIO_SBADDRLOW_MASK  0x80  /* Valid bits in SBADDRLOW */
#define SBSDIO_SBADDRMID_MASK  0xff  /* Valid bits in SBADDRMID */
#define SBSDIO_SBADDRHIGH_MASK 0xffU /* Valid bits in SBADDRHIGH */

/* Address bits from SBADDR regs */

#define SBSDIO_SBWINDOW_MASK   0xffff8000

#endif /* __DRIVERS_WIRELESS_IEEE80211_BCM43XXX_BCMF_SDIO_REGS_H */
