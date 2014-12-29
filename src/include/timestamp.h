/*
 * This file is part of the coreboot project.
 *
 * Copyright 2011 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include <stdint.h>

struct timestamp_entry {
	uint32_t	entry_id;
	uint64_t	entry_stamp;
} __attribute__((packed));

struct timestamp_table {
	uint64_t	base_time;
	uint32_t	max_entries;
	uint32_t	num_entries;
	struct timestamp_entry entries[0]; /* Variable number of entries */
} __attribute__((packed));

enum timestamp_id {
	TS_START_ROMSTAGE = 1,
	TS_BEFORE_INITRAM = 2,
	TS_AFTER_INITRAM = 3,
	TS_END_ROMSTAGE = 4,
	TS_START_VBOOT = 5,
	TS_END_VBOOT = 6,
	TS_START_COPYRAM = 8,
	TS_END_COPYRAM = 9,
	TS_START_RAMSTAGE = 10,
	TS_START_BOOTBLOCK = 11,
	TS_END_BOOTBLOCK = 12,
	TS_DEVICE_ENUMERATE = 30,
	TS_DEVICE_CONFIGURE = 40,
	TS_DEVICE_ENABLE = 50,
	TS_DEVICE_INITIALIZE = 60,
	TS_DEVICE_DONE = 70,
	TS_CBMEM_POST = 75,
	TS_WRITE_TABLES = 80,
	TS_LOAD_PAYLOAD = 90,
	TS_ACPI_WAKE_JUMP = 98,
	TS_SELFBOOT_JUMP = 99,
};

#if CONFIG_COLLECT_TIMESTAMPS
/*
 * Order of usage of timestamp library is:
 * Call timestamp_early_init / timestamp_init to set base time before any
 * timestamp_add function is called. timestamp_early_init also ensures that the
 * cache is valid in _timestamp region.
 * After this, timestamp_add / timestamp_add_now can be used to record
 * timestamps. Sync will be automatically taken care of by cbmem_initialize
 */
/*
 * Initialize the cache to a valid state and set the base time.
 * This function is used before DRAM is setup so that the timestamp cache can
 * be initialized in _timestamp region.
 * Both, timestamp_init and timestamp_early_init reset the cbmem state to
 * timestamp table reset required. Thus, whenever a timestamp_add or
 * timestamp_sync is done to add new entries into the cbmem timestamp table, it
 * first resets the table to 0 entries.
 */
void timestamp_early_init(uint64_t base);
/* Initialize the base time for timestamps and mark cache as valid */
void timestamp_init(uint64_t base);
/*
 * Add a new timestamp. Depending on cbmem is available or not, this timestamp
 * will be stored to cbmem / timestamp cache.
 */
void timestamp_add(enum timestamp_id id, uint64_t ts_time);
/* Calls timestamp_add with current timestamp. */
void timestamp_add_now(enum timestamp_id id);
/*
 * Sync all timestamps from timestamp_cache to cbmem area. Called by
 * cbmem_initialize.
 */
void timestamp_sync(void);
/* Implemented by the architecture code */
uint64_t timestamp_get(void);
#else
#define timestamp_early_init(base)
#define timestamp_init(base)
#define timestamp_add(id, time)
#define timestamp_add_now(id)
#define timestamp_sync()
#endif

#endif
