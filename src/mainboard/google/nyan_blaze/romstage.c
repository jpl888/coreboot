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

#include <arch/cache.h>
#include <arch/cpu.h>
#include <arch/exception.h>
#include <arch/io.h>
#include <arch/stages.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/cbmem_console.h>
#include <console/console.h>
#include <reset.h>
#include <romstage_handoff.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include "sdram_configs.h"
#include <soc/nvidia/tegra/i2c.h>
#include <soc/nvidia/tegra124/cache.h>
#include <soc/nvidia/tegra124/chip.h>
#include <soc/nvidia/tegra124/clk_rst.h>
#include <soc/nvidia/tegra124/early_configs.h>
#include <soc/nvidia/tegra124/power.h>
#include <soc/nvidia/tegra124/sdram.h>
#include <soc/addressmap.h>
#include <soc/clock.h>
#include <soc/display.h>
#include <symbols.h>
#include <timestamp.h>

static void __attribute__((noinline)) romstage(void)
{
	void *entry = NULL;
#if CONFIG_COLLECT_TIMESTAMPS
	uint64_t romstage_start_time = timestamp_get();
#endif

	console_init();
	exception_init();

	sdram_init(get_sdram_config());

	/* used for MMU and CBMEM setup, in MB */
	u32 dram_start_mb = (uintptr_t)_dram/MiB;
	u32 dram_end_mb = sdram_max_addressable_mb();
	u32 dram_size_mb = dram_end_mb - dram_start_mb;

#if !CONFIG_VBOOT2_VERIFY_FIRMWARE
	configure_l2_cache();
	mmu_init();
	/* Device memory below DRAM is uncached. */
	mmu_config_range(0, dram_start_mb, DCACHE_OFF);
	/* SRAM is cached. MMU code will round size up to page size. */
	mmu_config_range((uintptr_t)_sram/MiB, div_round_up(_sram_size, MiB),
			 DCACHE_WRITEBACK);
	/* The space above DRAM is uncached. */
	if (dram_end_mb < 4096)
		mmu_config_range(dram_end_mb, 4096 - dram_end_mb, DCACHE_OFF);
	mmu_disable_range(0, 1);
	dcache_mmu_enable();
#endif

	/* DRAM is cached. */
	mmu_config_range(dram_start_mb, dram_size_mb, DCACHE_WRITEBACK);
	/* A window for DMA is uncached. */
	mmu_config_range((uintptr_t)_dma_coherent/MiB,
			 _dma_coherent_size/MiB, DCACHE_OFF);

	/*
	 * A watchdog reset only resets part of the system so it ends up in
	 * a funny state. If that happens, we need to reset the whole machine.
	 */
	if (power_reset_status() == POWER_RESET_WATCHDOG) {
		printk(BIOS_INFO, "Watchdog reset detected, rebooting.\n");
		hard_reset();
	}

	cbmem_initialize_empty();

	timestamp_init(0);
	timestamp_add(TS_START_ROMSTAGE, romstage_start_time);

#if CONFIG_CONSOLE_CBMEM
	cbmemc_reinit();
#endif

#if CONFIG_VBOOT2_VERIFY_FIRMWARE
	entry = vboot_load_ramstage();
#else
	early_mainboard_init();
	vboot_verify_firmware(romstage_handoff_find_or_add());
#endif

	if (entry == NULL) {
		timestamp_add(TS_START_COPYRAM, timestamp_get());
		entry = cbfs_load_stage(CBFS_DEFAULT_MEDIA,
					CONFIG_CBFS_PREFIX "/ramstage");
		timestamp_add(TS_END_COPYRAM, timestamp_get());
		if (entry == (void *)-1)
			die("failed to load ramstage\n");
	}

	stage_exit(entry);
}

/* Stub to force arm_init_caches to the top, before any stack/memory accesses */
void main(void)
{
#if !CONFIG_VBOOT2_VERIFY_FIRMWARE
	asm volatile ("bl arm_init_caches"
		      ::: "r0","r1","r2","r3","r4","r5","ip");
#endif
	romstage();
}
