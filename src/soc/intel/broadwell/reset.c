/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2014 Google Inc.
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

#include <arch/hlt.h>
#include <arch/io.h>
#include <reset.h>
#include <broadwell/reset.h>
#include <romstage_handoff.h>
#include <cbmem.h>

/*
 * Soft reset (INIT# to cpu) - write 0x1 to I/O 0x92
 * Soft reset (INIT# to cpu)- write 0x4 to I/0 0xcf9
 * Cold reset (S0->S5->S0) - write 0xe to I/0 0xcf9
 * Warm reset (PLTRST# assertion) - write 0x6 to I/O 0xcf9
 * Global reset (S0->S5->S0 with ME reset) - write 0x6 or 0xe to 0xcf9 but
 * with ETR[20] set.
 */

void soft_reset(void)
{
        outb(0x04, 0xcf9);
}

void hard_reset(void)
{
#ifdef __PRE_RAM__
	/*
	 * Cold reset will not work until reference code has been
	 * executed, so request a reboot after that step.
	 *
	 * Do a soft_reset() if this is a S3 resume or other case where CBMEM
	 * isn't setup.
	 */
	struct romstage_handoff *handoff = cbmem_find(CBMEM_ID_ROMSTAGE_INFO);
	if (!handoff)
		soft_reset();
	else
		handoff->reboot_required = 1;
#else
        outb(0x06, 0xcf9);
#endif
}

void reset_system(void)
{
	hard_reset();
#ifndef __PRE_RAM__
	/*
	 * If hard_reset() returned in __PRE_RAM__, continue until the
	 * reset can be done. Otherwise, halt here.
	 */
	while (1) {
		hlt();
	}
#endif
}
