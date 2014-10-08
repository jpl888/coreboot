/*
 * This file is part of the coreboot project.
 *
 * Copyright 2014 Rockchip Inc.
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
#include <bootblock_common.h>
#include <console/console.h>
#include <soc/rockchip/rk3288/grf.h>
#include "addressmap.h"
#include "timer.h"
#include "clock.h"

void bootblock_cpu_init(void)
{
	rk3288_init_timer();

	if (IS_ENABLED(CONFIG_CONSOLE_SERIAL_UART)) {
		switch (CONFIG_CONSOLE_SERIAL_UART_ADDRESS) {
		case UART2_BASE:
			writel(IOMUX_UART2, &rk3288_grf->iomux_uart2);
			break;
		default:
			die("TODO: Handle setup for console UART if needed");
		}
		console_init();
	}

	rkclk_init();
}
