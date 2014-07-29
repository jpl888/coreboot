/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
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

#include <console/console.h>
#include <device/device.h>
#include <arch/io.h>
#include <vendorcode/google/chromeos/chromeos.h>
#include <soc/addressmap.h>

static void soc_read_resources(device_t dev)
{
	unsigned long index = 0;
	int i; uintptr_t begin, end, framebuffer_begin;
	size_t size, framebuffer_size;

	printk(BIOS_DEBUG, "%s: entry, device = %p\n", __func__, dev);
	for (i = 0; i < CARVEOUT_NUM; i++) {
		carveout_range(i, &begin, &size);
		if (size == 0)
			continue;
		reserved_ram_resource(dev, index++, begin * KiB, size * KiB);
	}

	framebuffer_begin = framebuffer_attributes(&framebuffer_size);
	//TODO: Frame buffer needs to handled as a carveout from the below_4G

	memory_in_range_below_4gb(&begin, &end);
	size = end - begin;
	ram_resource(dev, index++, begin * KiB, size * KiB);

	memory_in_range_above_4gb(&begin, &end);
	size = end - begin;
	ram_resource(dev, index++, begin * KiB, size * KiB);
}

static void soc_init(device_t dev)
{
	printk(BIOS_INFO, "CPU: Tegra132\n");
}

static void soc_noop(device_t dev)
{
}

static struct device_operations soc_ops = {
	.read_resources   = soc_read_resources,
	.set_resources    = soc_noop,
	.enable_resources = soc_noop,
	.init             = soc_init,
	.scan_bus         = 0,
};

static void enable_tegra132_dev(device_t dev)
{
	dev->ops = &soc_ops;
}

struct chip_operations soc_nvidia_tegra132_ops = {
	CHIP_NAME("SOC Nvidia Tegra132")
	.enable_dev = enable_tegra132_dev,
};
