/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2012 Google Inc.
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

#include <stdint.h>
#include <string.h>
#include <soc/gpio.h>
#include <soc/pei_data.h>
#include <soc/pei_wrapper.h>

void mainboard_fill_pei_data(struct pei_data *pei_data)
{
	/* DQ byte map for Samus board */
	const u8 dq_map[2][6][2] = {
		{ { 0x0F, 0xF0 }, { 0x00, 0xF0 }, { 0x0F, 0xF0 },
		  { 0x0F, 0x00 }, { 0xFF, 0x00 }, { 0xFF, 0x00 } },
		{ { 0x0F, 0xF0 }, { 0x00, 0xF0 }, { 0x0F, 0xF0 },
		  { 0x0F, 0x00 }, { 0xFF, 0x00 }, { 0xFF, 0x00 } } };
	/* DQS CPU<>DRAM map for Samus board */
	const u8 dqs_map[2][8] = {
		{ 2, 0, 3, 1, 5, 7, 6, 4 },
		{ 3, 1, 2, 0, 6, 4, 5, 7 } };

	/* One installed DIMM per channel */
	pei_data->dimm_channel0_disabled = 2;
	pei_data->dimm_channel1_disabled = 2;

	pei_data->spd_addresses[0] = 0xa2;
	pei_data->spd_addresses[2] = 0xa2;

	memcpy(pei_data->dq_map, dq_map, sizeof(dq_map));
	memcpy(pei_data->dqs_map, dqs_map, sizeof(dqs_map));

	pei_data_usb2_port(pei_data, 0, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);
	pei_data_usb2_port(pei_data, 1, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);
	pei_data_usb2_port(pei_data, 2, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);
	pei_data_usb2_port(pei_data, 3, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);
	pei_data_usb2_port(pei_data, 4, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);
	pei_data_usb2_port(pei_data, 5, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);
	pei_data_usb2_port(pei_data, 6, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);
	pei_data_usb2_port(pei_data, 7, 0x40, 1, USB_OC_PIN_SKIP,
			   USB_PORT_FRONT_PANEL);

	pei_data_usb3_port(pei_data, 0, 1, USB_OC_PIN_SKIP, 0);
	pei_data_usb3_port(pei_data, 1, 1, USB_OC_PIN_SKIP, 0);
	pei_data_usb3_port(pei_data, 2, 1, USB_OC_PIN_SKIP, 0);
	pei_data_usb3_port(pei_data, 3, 1, USB_OC_PIN_SKIP, 0);
}
