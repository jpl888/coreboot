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

#include <types.h>
#include <armv7.h>
#include <cbfs.h>
#include <console/console.h>
#include <arch/stages.h>
#include <cbmem.h>
#include <delay.h>
#include <timestamp.h>
#include <arch/cache.h>
#include <arch/exception.h>
#include <stdlib.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <soc/marvell/bg4cd/sdram.h>
#include <symbols.h>
#include "timer.h"

void main(void)
{
	void *entry;
#if CONFIG_COLLECT_TIMESTAMPS
	uint64_t start_romstage_time;
	uint64_t before_dram_time;
	uint64_t after_dram_time;
	uint64_t base_time = timestamp_get();
	start_romstage_time = timestamp_get();
#endif

	console_init();

#if CONFIG_COLLECT_TIMESTAMPS
	before_dram_time = timestamp_get();
#endif
	sdram_init();
#if CONFIG_COLLECT_TIMESTAMPS
	after_dram_time = timestamp_get();
#endif
	mmu_init();
	mmu_config_range(0, 4096, DCACHE_OFF);
	dcache_mmu_enable();

	cbmem_initialize_empty();
#if CONFIG_COLLECT_TIMESTAMPS
	timestamp_init(base_time);
	timestamp_add(TS_START_ROMSTAGE, start_romstage_time);
	timestamp_add(TS_BEFORE_INITRAM, before_dram_time);
	timestamp_add(TS_AFTER_INITRAM, after_dram_time);
#endif

	entry = vboot_load_ramstage();

	if (entry == NULL) {
		timestamp_add(TS_START_COPYRAM, timestamp_get());
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/ramstage");
		timestamp_add(TS_END_COPYRAM, timestamp_get());
		if (entry == (void *)-1)
			die("failed to load ramstage\n");
	}
#if CONFIG_COLLECT_TIMESTAMPS
	timestamp_add_now(TS_END_ROMSTAGE);
#endif
	stage_exit(entry);
}
