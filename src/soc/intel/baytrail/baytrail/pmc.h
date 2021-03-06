/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
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

#ifndef _BAYTRAIL_PMC_H_
#define _BAYTRAIL_PMC_H_


#define IOCOM1		0x3f8

/* Memory mapped IO registers behind PMC_BASE_ADDRESS */
#define GEN_PMCONF1	0x20
#	define UART_EN		(1 << 24)

/* IO Mapped registers behind ACPI_BASE_ADDRESS */
#define TCO_RLD			0x60
#define TCO_STS			0x64
#	define SECOND_TO_STS	(1 << 17)
#	define TCO_TIMEOUT	(1 << 3)
#define TCO1_CNT		0x68
#	define TCO_LOCK		(1 << 12)
#	define TCO_TMR_HALT	(1 << 11)
#define TCO_TMR			0x70

#endif /* _BAYTRAIL_PMC_H_ */
