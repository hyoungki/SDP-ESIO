/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * sdp_wdt.h - SDP External Watchdog GPIO Character Driver
 *             Shared header: kernel driver + userspace application
 *
 * Hardware (imx6sx):
 *   GPIO1_22  ->  WDT_CTRL  (watchdog enable,  sysfs GPIO 22)
 *   GPIO1_25  ->  WDT_CLR   (keepalive toggle, sysfs GPIO 25)
 *
 * Device node: /dev/SDP_WDT
 *
 * Interface:
 *   write(fd, buf, 1)              -> toggle WDT_CLR
 *   ioctl(fd, SET_WDT_CTRL, 0|1)  -> set WDT_CTRL low/high
 *   ioctl(fd, GET_WDT_STATUS, &s) -> read back both GPIO states
 */

#ifndef _SDP_WDT_H_
#define _SDP_WDT_H_

#include <linux/ioctl.h>

/* -------------------------------------------------------------------------
 * ioctl magic number - choose something unlikely to collide.
 * 'W' is used by kernel's own watchdog; use 0xB2 for SDP private use.
 * ---------------------------------------------------------------------- */
#define SDP_WDT_IOC_MAGIC   0xB2

/*
 * SET_WDT_CTRL  arg: 0 = disable watchdog (WDT_CTRL LOW)
 *                    1 = enable  watchdog (WDT_CTRL HIGH)
 */
#define SDP_WDT_SET_CTRL    _IOW(SDP_WDT_IOC_MAGIC, 0x01, int)

/*
 * GET_WDT_STATUS  arg: pointer to struct sdp_wdt_status
 */
#define SDP_WDT_GET_STATUS  _IOR(SDP_WDT_IOC_MAGIC, 0x02, struct sdp_wdt_status)

/* Status structure filled by GET_WDT_STATUS */
struct sdp_wdt_status {
    int clr_value;   /* current WDT_CLR  GPIO level (0 or 1) */
    int ctrl_value;  /* current WDT_CTRL GPIO level (0 or 1) */
};

#endif /* _SDP_WDT_H_ */
