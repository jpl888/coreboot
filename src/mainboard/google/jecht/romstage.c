/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2014 Google Inc.
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

#include <cbfs.h>
#include <console/console.h>
#include <string.h>
#include <ec/google/chromeec/ec.h>
#include <soc/gpio.h>
#include <soc/pei_data.h>
#include <soc/pei_wrapper.h>
#include <soc/romstage.h>
#include <mainboard/google/jecht/spd/spd.h>
#include "gpio.h"
#include "superio/ite/it8772f/it8772f.h"
#include "superio/ite/it8772f/early_serial.c"

void mainboard_romstage_entry(struct romstage_params *rp)
{
	struct pei_data pei_data;

	post_code(0x32);

	/* Initialize GPIOs */
	init_gpios(mainboard_gpio_config);

	/* Fill out PEI DATA */
	memset(&pei_data, 0, sizeof(pei_data));
	mainboard_fill_pei_data(&pei_data);
	mainboard_fill_spd_data(&pei_data);
	rp->pei_data = &pei_data;

	/* Call into the real romstage main with this board's attributes. */
	romstage_common(rp);
}

void mainboard_pre_console_init(void)
{
	/* Early SuperIO setup */
	it8772f_kill_watchdog();
	it8772f_ac_resume_southbridge();
	it8772f_enable_serial(PNP_DEV(IT8772F_BASE, IT8772F_SP1),
			      CONFIG_TTYS0_BASE);
}
