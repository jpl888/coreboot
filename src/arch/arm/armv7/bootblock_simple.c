/*
 * This file is part of the coreboot project.
 *
 * Copyright 2010 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <arch/exception.h>
#include <arch/hlt.h>
#include <arch/stages.h>
#include <bootblock_common.h>
#include <cbfs.h>
#include <console/console.h>
#include <delay.h>
#include <timestamp.h>
#include <vendorcode/google/chromeos/chromeos.h>

__attribute__((weak)) void bootblock_mainboard_early_init(void) { /* no-op */ }
__attribute__((weak)) void bootblock_soc_init(void) { /* do nothing */ }
__attribute__((weak)) void bootblock_mainboard_init(void) { /* do nothing */ }

void main(void)
{
	void *entry = (void *)-1;

	init_timer();
	if (IS_ENABLED(CONFIG_HAS_PRECBMEM_TIMESTAMP_REGION))
		timestamp_early_init(timestamp_get());

	bootblock_mainboard_early_init();

	if (CONFIG_BOOTBLOCK_CONSOLE) {
		console_init();
		exception_init();
	}

	cbfs_set_header_offset(0);

	bootblock_soc_init();
	bootblock_mainboard_init();

	if (IS_ENABLED(CONFIG_VBOOT2_VERIFY_FIRMWARE)) {
		timestamp_add_now(TS_START_COPYVER);
		if (IS_ENABLED(CONFIG_RETURN_FROM_VERSTAGE))
			vboot2_verify_firmware();	/* doesn't return */
		else
			entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
						CONFIG_CBFS_PREFIX "/verstage");
		timestamp_add_now(TS_END_COPYVER);
	} else {
		timestamp_add_now(TS_START_COPYROM);
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/romstage");
		timestamp_add_now(TS_END_COPYROM);
	}

	if (entry != (void *)-1)
		stage_exit(entry);
	hlt();
}
