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

#include <stdlib.h>
#include <console/console.h>
#include <soc/rockchip/rk3288/gpio.h>

#include <boardid.h>

uint8_t board_id(void)
{
	static int id = -1;
	static const gpio_t pins[] = {[3] = GPIO(2, A, 7), [2] = GPIO(2, A, 2),
		[1] = GPIO(2, A, 1), [0] = GPIO(2, A, 0)}; /* GPIO2_A0 is LSB */

	if (id < 0) {
		int i;

		id = 0;
		for (i = 0; i < ARRAY_SIZE(pins); i++) {
			gpio_input(pins[i]);
			id |= gpio_get_in_value(pins[i]) << i;
		}
		printk(BIOS_SPEW, "Board ID: %d.\n", id);
	}

	return id;
}
