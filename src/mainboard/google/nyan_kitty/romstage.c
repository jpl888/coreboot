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
#include <device/device.h>
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
	u32 dram_start = (CONFIG_SYS_SDRAM_BASE >> 20);
	u32 dram_end = sdram_max_addressable_mb();	/* plus one... */
	u32 dram_size = dram_end - dram_start;

#if !CONFIG_VBOOT2_VERIFY_FIRMWARE
	configure_l2_cache();
	mmu_init();
	/* Device memory below DRAM is uncached. */
	mmu_config_range(0, dram_start, DCACHE_OFF);
	/* SRAM is cached. Round the size up to 2MB, the LPAE page size. */
	mmu_config_range(0x40000000 >> 20, 2, DCACHE_WRITEBACK);
	/* The space above DRAM is uncached. */
	if (dram_end < 4096)
		mmu_config_range(dram_end, 4096 - dram_end, DCACHE_OFF);
	mmu_disable_range(0, 1);
	dcache_mmu_enable();
#endif

	/* DRAM is cached. */
	mmu_config_range(dram_start, dram_size, DCACHE_WRITEBACK);
	/* A window for DMA is uncached. */
	mmu_config_range(CONFIG_DRAM_DMA_START >> 20,
			 CONFIG_DRAM_DMA_SIZE >> 20, DCACHE_OFF);

	/*
	 * A watchdog reset only resets part of the system so it ends up in
	 * a funny state. If that happens, we need to reset the whole machine.
	 */
	if (power_reset_status() == POWER_RESET_WATCHDOG) {
		printk(BIOS_INFO, "Watchdog reset detected, rebooting.\n");
		hard_reset();
	}

	/* For quality of the user experience, it's important to get
	 * the video going ASAP. Because there are long delays in some
	 * of the powerup steps, we do some very early setup here in
	 * romstage. The only thing setup_display does is manage
	 * 4 GPIOs, under control of the config struct members.
	 * In general, it is safe to enable panel power, and disable
	 * anything related to the backlight. If we get something wrong,
	 * we can easily fix it in ramstage by further GPIO manipulation,
	 * so we feel it is ok to do some setting at this point.
	 */

	const struct device *soc = dev_find_slot(DEVICE_PATH_CPU_CLUSTER, 0);
	printk(BIOS_SPEW, "s%s: soc is %p\n", __func__, soc);
	if (soc && soc->chip_info) {
		const struct soc_nvidia_tegra124_config *config =
			soc->chip_info;
		setup_display((struct soc_nvidia_tegra124_config *)config);
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