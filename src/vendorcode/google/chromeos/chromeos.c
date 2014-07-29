/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
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

#include <stddef.h>
#include <string.h>
#include "chromeos.h"
#if CONFIG_VBOOT_VERIFY_FIRMWARE
#include "vboot_handoff.h"
#endif
#include <boot/coreboot_tables.h>
#include <cbfs.h>
#include <cbmem.h>
#include <console/console.h>

#if CONFIG_VBOOT_VERIFY_FIRMWARE
static int vboot_enable_developer(void)
{
	struct vboot_handoff *vbho;

	vbho = cbmem_find(CBMEM_ID_VBOOT_HANDOFF);

	if (vbho == NULL) {
		printk(BIOS_ERR, "%s: Couldn't find vboot_handoff structure!\n",
		        __func__);
		return 0;
	}

	return !!(vbho->init_params.out_flags & VB_INIT_OUT_ENABLE_DEVELOPER);
}

static int vboot_enable_recovery(void)
{
	struct vboot_handoff *vbho;

	vbho = cbmem_find(CBMEM_ID_VBOOT_HANDOFF);

	if (vbho == NULL)
		return 0;

	return !!(vbho->init_params.out_flags & VB_INIT_OUT_ENABLE_RECOVERY);
}
#else
static inline int vboot_enable_developer(void) { return 0; }
static inline int vboot_enable_recovery(void) { return 0; }
#endif

int developer_mode_enabled(void)
{
	return get_developer_mode_switch() || vboot_enable_developer();
}

int recovery_mode_enabled(void)
{
	/*
	 * This is called in multiple places and has to detect
	 * recovery mode triggered from the EC and via shared
	 * recovery reason set with crossystem.
	 *
	 * If shared recovery reason is set:
	 * - before VbInit then get_recovery_mode_from_vbnv() is true
	 * - after VbInit then vboot_enable_recovery() is true
	 *
	 * Otherwise the mainboard handler for get_recovery_mode_switch()
	 * will detect recovery mode initiated by the EC.
	 */
	return get_recovery_mode_switch() || get_recovery_mode_from_vbnv() ||
		vboot_enable_recovery();
}

int __attribute__((weak)) clear_recovery_mode_switch(void)
{
	// Can be implemented by a mainboard
	return 0;
}

int vboot_skip_display_init(void)
{
#if CONFIG_VBOOT_VERIFY_FIRMWARE
	struct vboot_handoff *vbho;

	vbho = cbmem_find(CBMEM_ID_VBOOT_HANDOFF);

	if (vbho == NULL)
		return 0;

	return !(vbho->init_params.out_flags & VB_INIT_OUT_ENABLE_DISPLAY);
#else
	return 0;
#endif
}

#ifdef __PRE_RAM__
void __attribute__((weak)) save_chromeos_gpios(void)
{
	// Can be implemented by a mainboard
}

int __attribute((weak)) vboot_get_sw_write_protect(void)
{
	// Can be implemented by a platform / mainboard
	return 0;
}
#endif

#if CONFIG_VBOOT_VERIFY_FIRMWARE || CONFIG_VBOOT2_VERIFY_FIRMWARE
void *vboot_get_region(uintptr_t offset_addr, size_t size, void *dest)
{
	if (IS_ENABLED(CONFIG_SPI_FLASH_MEMORY_MAPPED)) {
		if (dest != NULL)
			return memcpy(dest, (void *)offset_addr, size);
		else
			return (void *)offset_addr;
	} else {
		struct cbfs_media default_media, *media = &default_media;
		void *cache;

		init_default_cbfs_media(media);
		media->open(media);
		if (dest != NULL) {
			cache = dest;
			if (media->read(media, dest, offset_addr, size) != size)
				cache = NULL;
		} else {
			cache = media->map(media, offset_addr, size);
			if (cache == CBFS_MEDIA_INVALID_MAP_ADDRESS)
				cache = NULL;
		}
		media->close(media);
		return cache;
	}
}
#endif

#if CONFIG_VBOOT2_VERIFY_FIRMWARE
void *vboot_get_payload(int *len) { return NULL; }
#endif

#if CONFIG_VBOOT_VERIFY_FIRMWARE
void *vboot_get_payload(int *len)
{
	struct vboot_handoff *vboot_handoff;
	struct firmware_component *fwc;

	vboot_handoff = cbmem_find(CBMEM_ID_VBOOT_HANDOFF);

	if (vboot_handoff == NULL)
		return NULL;

	if (CONFIG_VBOOT_BOOT_LOADER_INDEX >= MAX_PARSED_FW_COMPONENTS) {
		printk(BIOS_ERR, "Invalid boot loader index: %d\n",
		       CONFIG_VBOOT_BOOT_LOADER_INDEX);
		return NULL;
	}

	fwc = &vboot_handoff->components[CONFIG_VBOOT_BOOT_LOADER_INDEX];

	/* If payload size is zero fall back to cbfs path. */
	if (fwc->size == 0)
		return NULL;

	if (len != NULL)
		*len = fwc->size;

	printk(BIOS_DEBUG, "Booting 0x%x byte payload at 0x%08x.\n",
	       fwc->size, fwc->address);

	/* This will leak a mapping. */
	return vboot_get_region(fwc->address, fwc->size, NULL);
}

int vboot_get_handoff_info(void **addr, uint32_t *size)
{
	struct vboot_handoff *vboot_handoff;

	vboot_handoff = cbmem_find(CBMEM_ID_VBOOT_HANDOFF);

	if (vboot_handoff == NULL)
		return -1;

	*addr = vboot_handoff;
	*size = sizeof(*vboot_handoff);
	return 0;
}
#endif
