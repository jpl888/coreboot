/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright 2012 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <arch/io.h>
#include <console/console.h>
#include <cpu/x86/smm.h>
#include <southbridge/intel/lynxpoint/nvs.h>
#include <southbridge/intel/lynxpoint/pch.h>
#include <southbridge/intel/lynxpoint/me.h>
#include <northbridge/intel/haswell/haswell.h>
#include <cpu/intel/haswell/haswell.h>
#include <elog.h>
#include <superio/ite/it8772f/it8772f.h>

/* GPIO46 controls the WLAN_DISABLE_L signal. */
#define GPIO_WLAN_DISABLE_L 46
#define GPIO_LTE_DISABLE_L  59

int mainboard_io_trap_handler(int smif)
{
	switch (smif) {
	case 0x99:
		printk(BIOS_DEBUG, "Sample\n");
		smm_get_gnvs()->smif = 0;
		break;
	default:
		return 0;
	}

	/* On success, the IO Trap Handler returns 0
	 * On failure, the IO Trap Handler returns a value != 0
	 *
	 * For now, we force the return value to 0 and log all traps to
	 * see what's going on.
	 */
	//gnvs->smif = 0;
	return 1;
}

#if 0
static u8 mainboard_smi_ec(void)
{
	u8 cmd = 0;// google_chromeec_get_event();

#if CONFIG_ELOG_GSMI
	/* Log this event */
	if (cmd)
		elog_add_event_byte(ELOG_TYPE_EC_EVENT, cmd);
#endif

	return cmd;
}
#endif

/* gpi_sts is GPIO 47:32 */
void mainboard_smi_gpi(u32 gpi_sts)
{
#if 0
	if (gpi_sts & (1 << (EC_SMI_GPI - 32))) {
		/* Process all pending events */
		while (mainboard_smi_ec() != 0);
	}
#endif
}

void mainboard_smi_sleep(u8 slp_typ)
{
	/* Disable USB charging if required */
	switch (slp_typ) {
	case 3:
			it8772f_gpio_led(2 /* set */, 0xF7 /* select */,
			0x04 /* polarity: inverting */, 0x04 /* 1=pullup */,
			0x04 /* output */, 0x00, /* 0=Alternate function */
			SIO_GPIO_BLINK_GPIO22, IT8772F_GPIO_BLINK_FREQUENCY_1_HZ);
		break;
	case 5:
			it8772f_gpio_led(2 /* set */, 0xF7 /* select */,
			0x04 /* polarity: inverting */, 0x00 /* 0=pulldown */,
			0x04 /* output */, 0x04 /* 1=Simple IO function */,
			SIO_GPIO_BLINK_GPIO22, IT8772F_GPIO_BLINK_FREQUENCY_1_HZ);
		break;
	default:
		break;
	}
}

#define APMC_FINALIZE 0xcb

static int mainboard_finalized = 0;

int mainboard_smi_apmc(u8 apmc)
{
	switch (apmc) {
	case APMC_FINALIZE:
		if (mainboard_finalized) {
			printk(BIOS_DEBUG, "SMI#: Already finalized\n");
			return 0;
		}

		intel_pch_finalize_smm();
		intel_northbridge_haswell_finalize_smm();
		intel_cpu_haswell_finalize_smm();

		mainboard_finalized = 1;
		break;
	case APM_CNT_ACPI_ENABLE:
		//google_chromeec_set_smi_mask(0);
		/* Clear all pending events */
		//while (google_chromeec_get_event() != 0);
		//google_chromeec_set_sci_mask(MAINBOARD_EC_SCI_EVENTS);
		break;
	case APM_CNT_ACPI_DISABLE:
		//google_chromeec_set_sci_mask(0);
		/* Clear all pending events */
		//while (google_chromeec_get_event() != 0);
		//google_chromeec_set_smi_mask(MAINBOARD_EC_SMI_EVENTS);;
		break;
	}
	return 0;
}