/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Google Inc.
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

#include <arch/exception.h>
#include <arch/hlt.h>
#include <cbfs.h>
#include <console/console.h>
#include <soc/addressmap.h>
#include <soc/bootblock.h>
#include <soc/clock.h>
#include <soc/nvidia/tegra/apbmisc.h>
#include <arch/stages.h>

#include "power.h"

void __attribute__((weak)) bootblock_mainboard_early_init(void)
{
	/* Empty default implementation. */
}

void main(void)
{
	void *entry;

	// enable pinmux clamp inputs
	clamp_tristate_inputs();

	// enable JTAG at the earliest stage
	enable_jtag();

	clock_early_uart();

	/* Configure mselect clock. */
	clock_configure_source(mselect, PLLP, 102000);

	/* Enable AVP cache, timer, APB dma, and mselect blocks.  */
	clock_enable_clear_reset(CLK_L_CACHE2 | CLK_L_TMR,
				 CLK_H_APBDMA,
				 0, CLK_V_MSELECT, 0, 0);

	bootblock_mainboard_early_init();

	if (CONFIG_BOOTBLOCK_CONSOLE) {
		console_init();
		exception_init();
		printk(BIOS_INFO, "T132: Bootblock here\n");
	}

	clock_init();

	printk(BIOS_INFO, "T132 bootblock: Clock init done\n");

	bootblock_mainboard_init();

	printk(BIOS_INFO, "T132 bootblock: Mainboard bootblock init done\n");

	entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
				CONFIG_CBFS_PREFIX "/romstage");

	if (entry) {
		printk(BIOS_INFO, "T132 bootblock: jumping to romstage\n");
		stage_exit(entry);
	} else {
		printk(BIOS_INFO, "T132 bootblock: fallback/romstage not found\n");
	}

	hlt();
}
