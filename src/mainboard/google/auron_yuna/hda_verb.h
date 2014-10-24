/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the License.
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

static const u32 mainboard_cim_verb_data[] = {
	/* coreboot specific header */
	0x10ec0283,	// Codec Vendor / Device ID: Realtek ALC283
	0x10ec0283,	// Subsystem ID
	0x00000011,	// Number of jacks (NID entries)

	0x0017ff00,	// Function Reset
	0x0017ff00,	// Double Function Reset
	0x000F0000,	// Pad - get vendor id
	0x000F0002,	// Pad - get revision id

	/* Bits 31:28 - Codec Address */
	/* Bits 27:20 - NID */
	/* Bits 19:8 - Verb ID */
	/* Bits 7:0  - Payload */

	/* NID 0x01, HDA Codec Subsystem ID Verb Table: 0x10ec0283 */
	0x00172083,
	0x00172102,
	0x001722ec,
	0x00172310,

	/* Pin Widget Verb Table */

	/* Pin Complex (NID 0x12) DMIC - Enabled */
	0x01271c30, //
	0x01271d01, //
	0x01271ea6, //
	0x01271f90, //

	/* Pin Complex (NID 0x14) SPKR-OUT - Internal Speakers */
	0x01471c10, // group 1, cap 0
	0x01471d01, // no connector, no jack detect
	0x01471e17, // speaker out, analog
	0x01471f90, // fixed function, internal, Location N/A

	/* Pin Complex (NID 0x17) MONO Out - Enabled */
	0x01771c08, //
	0x01771d00, //
	0x01771e00, //
	0x01771f40, //

	/* Pin Complex (NID 0x18) Disabled */
	0x01871cf0, //
	0x01871d11, //
	0x01871e11, //
	0x01871f41, //

	/* Pin Complex (NID 0x19)  MIC2 - 3.5mm Jack */
	0x01971c20, // group2, cap 0
	0x01971d90, // black, jack detect
	0x01971ea1, // Mic in, 3.5mm Jack
	0x01971f03, // connector, External left panel

	/* Pin Complex (NID 0x1A)  LINE1 - Disabled */
	0x01a71cf0, //
	0x01a71d11, //
	0x01a71e11, //
	0x01a71f41, //

	/* Pin Complex (NID 0x1B)  LINE2 - Disabled */
	0x01b71cf0, //
	0x01b71d11, //
	0x01b71e11, //
	0x01b71f41, //

	/* Pin Complex (NID 0x1D)  PCBeep */
	0x01d71c2d, // eapd low on ex-amp, laptop, custom enable
	0x01d71d81, // mute spkr on hpout
	0x01d71e15, // pcbeep en able, checksum
	0x01d71f40, // no physical, Internal, Location N/A

	/* Pin Complex (NID 0x1E)  SPDIF-OUT - Disabled*/
	0x01e71cf0, //
	0x01e71d11, //
	0x01e71e11, //
	0x01e71f41, //

	/* Pin Complex (NID 0x21) HP-OUT - 3.5mm Jack*/
	0x02171c1f, // group1
	0x02171d10, // black, jack detect
	0x02171e21, // HPOut, 3.5mm Jack
	0x02171f03, // connector, left panel

	/* Undocumented settings from Realtek (needed for beep_gen) */
	/* Widget node 0x20 */
	0x02050038,
	0x02046900,
	0x02050010,
	0x02040C20,
	/* Widget node 0x20 - 1 */
	0x02050019,
	0x02041857,
	0x0205001A,
	0x02044001,
	/* Widget node 0x20 - 2 */
	0x0205001B,
	0x0204140B,
	0x02050025,
	0x0204802A,
	/* Widget node 0x20 - 3 */
	0x02050045,
	0x02045029,
	0x02050046,
	0x02040004,
	/* Widget node 0x20 - 4 */
	0x02050043,
	0x0204A614,
	0x02050043,
	0x0204A614,
};

static const u32 mainboard_pc_beep_verbs[] = {
	0x00170500,	/* power up everything (codec, dac, adc, mixers)  */
	0x01470740,	/* enable speaker out */
	0x01470c02,	/* set speaker EAPD pin */
	0x0143b01f,	/* unmute speaker */
	0x00c37100,	/* unmute mixer nid 0xc input 1 */
	0x00b37410,	/* unmute mixer nid 0xb beep input and set volume */
};

static const u32 mainboard_pc_beep_verbs_size =
	sizeof(mainboard_pc_beep_verbs) / sizeof(mainboard_pc_beep_verbs[0]);
