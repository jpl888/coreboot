/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2015 Intel Corp.
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
#include <arch/acpi.h>
#include <bootstate.h>
#include <cbmem.h>
#include "chip.h"
#include <console/console.h>
#include <cpu/x86/smm.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <pc80/mc146818rtc.h>
#include <romstage_handoff.h>
#include <soc/iomap.h>
#include <soc/irq.h>
#include <soc/lpc.h>
#include <soc/nvs.h>
#include <soc/pci_devs.h>
#include <soc/pm.h>
#include <soc/ramstage.h>
#include <soc/spi.h>
#include <stdint.h>
#include <uart8250.h>

static inline void
add_mmio_resource(device_t dev, int i, unsigned long addr, unsigned long size)
{
	printk(BIOS_SPEW, "%s/%s ( %s, 0x%016lx, 0x%016lx )\n",
			__FILE__, __func__, dev_name(dev), addr, size);
	mmio_resource(dev, i, addr >> 10, size >> 10);
}

static void sc_add_mmio_resources(device_t dev)
{
	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));
	add_mmio_resource(dev, 0xfeb, ABORT_BASE_ADDRESS, ABORT_BASE_SIZE);
	add_mmio_resource(dev, PBASE, PMC_BASE_ADDRESS, PMC_BASE_SIZE);
	add_mmio_resource(dev, IOBASE, IO_BASE_ADDRESS, IO_BASE_SIZE);
	add_mmio_resource(dev, IBASE, ILB_BASE_ADDRESS, ILB_BASE_SIZE);
	add_mmio_resource(dev, SBASE, SPI_BASE_ADDRESS, SPI_BASE_SIZE);
	add_mmio_resource(dev, MPBASE, MPHY_BASE_ADDRESS, MPHY_BASE_SIZE);
	add_mmio_resource(dev, PUBASE, PUNIT_BASE_ADDRESS, PUNIT_BASE_SIZE);
	add_mmio_resource(dev, RCBA, RCBA_BASE_ADDRESS, RCBA_BASE_SIZE);
}

/* Default IO range claimed by the LPC device. The upper bound is exclusive. */
#define LPC_DEFAULT_IO_RANGE_LOWER 0
#define LPC_DEFAULT_IO_RANGE_UPPER 0x1000

static inline int io_range_in_default(int base, int size)
{
	/* Does it start above the range? */
	if (base >= LPC_DEFAULT_IO_RANGE_UPPER)
		return 0;

	/* Is it entirely contained? */
	if (base >= LPC_DEFAULT_IO_RANGE_LOWER &&
	    (base + size) < LPC_DEFAULT_IO_RANGE_UPPER)
		return 1;

	/* This will return not in range for partial overlaps. */
	return 0;
}

/*
 * Note: this function assumes there is no overlap with the default LPC device's
 * claimed range: LPC_DEFAULT_IO_RANGE_LOWER -> LPC_DEFAULT_IO_RANGE_UPPER.
 */
static void sc_add_io_resource(device_t dev, int base, int size, int index)
{
	struct resource *res;

	printk(BIOS_SPEW, "%s/%s ( %s, 0x%08x, 0x%08x, 0x%08x )\n",
			__FILE__, __func__, dev_name(dev), base, size, index);

	if (io_range_in_default(base, size))
		return;

	res = new_resource(dev, index);
	res->base = base;
	res->size = size;
	res->flags = IORESOURCE_IO | IORESOURCE_ASSIGNED | IORESOURCE_FIXED;
}

static void sc_add_io_resources(device_t dev)
{
	struct resource *res;

	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));

	/* Add the default claimed IO range for the LPC device. */
	res = new_resource(dev, 0);
	res->base = LPC_DEFAULT_IO_RANGE_LOWER;
	res->size = LPC_DEFAULT_IO_RANGE_UPPER - LPC_DEFAULT_IO_RANGE_LOWER;
	res->flags = IORESOURCE_IO | IORESOURCE_ASSIGNED | IORESOURCE_FIXED;

	/* GPIO */
	sc_add_io_resource(dev, GPIO_BASE_ADDRESS, 256, GBASE);

	/* ACPI */
	sc_add_io_resource(dev, ACPI_BASE_ADDRESS, 128, ABASE);
}

static void sc_read_resources(device_t dev)
{
	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));

	/* Get the normal PCI resources of this device. */
	pci_dev_read_resources(dev);

	/* Add non-standard MMIO resources. */
	sc_add_mmio_resources(dev);

	/* Add IO resources. */
	sc_add_io_resources(dev);
}

static void sc_rtc_init(void)
{
	uint32_t gen_pmcon1;
	int rtc_fail;
	struct chipset_power_state *ps = cbmem_find(CBMEM_ID_POWER_STATE);

	printk(BIOS_SPEW, "%s/%s\n",
			__FILE__, __func__);
	if (ps != NULL)
		gen_pmcon1 = ps->gen_pmcon1;
	else
		gen_pmcon1 = read32((void *)(PMC_BASE_ADDRESS + GEN_PMCON1));

	rtc_fail = !!(gen_pmcon1 & RPS);

	if (rtc_fail)
		printk(BIOS_DEBUG, "RTC failure.\n");

	cmos_init(rtc_fail);
}

/*
 * The UART hardware loses power while in suspend. Because of this the kernel
 * can hang because it doesn't re-initialize serial ports it is using for
 * consoles at resume time. The following function configures the UART
 * if the hardware is enabled though it may not be the correct baud rate
 * or configuration.  This is definitely a hack, but it helps the kernel
 * along.
 */
static void com1_configure_resume(device_t dev)
{
	const uint16_t port = 0x3f8;

	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));

	/* Is the UART I/O port eanbled? */
	if (!(pci_read_config32(dev, UART_CONT) & 1))
		return;

	/* Disable interrupts */
	outb(0x0, port + UART8250_IER);

	/* Enable FIFOs */
	outb(UART8250_FCR_FIFO_EN, port + UART8250_FCR);

	/* assert DTR and RTS so the other end is happy */
	outb(UART8250_MCR_DTR | UART8250_MCR_RTS, port + UART8250_MCR);

	/* DLAB on */
	outb(UART8250_LCR_DLAB | 3, port + UART8250_LCR);

	/* Set Baud Rate Divisor. 1 ==> 115200 Baud */
	outb(1, port + UART8250_DLL);
	outb(0, port + UART8250_DLM);

	/* Set to 3 for 8N1 */
	outb(3, port + UART8250_LCR);
}

static void sc_init(device_t dev)
{
	int i;
	const unsigned long pr_base = ILB_BASE_ADDRESS + 0x08;
	const unsigned long ir_base = ILB_BASE_ADDRESS + 0x20;
	void *gen_pmcon1 = (void *)(PMC_BASE_ADDRESS + GEN_PMCON1);
	void *actl = (void *)(ILB_BASE_ADDRESS + ACTL);
	const struct braswell_irq_route *ir = &global_braswell_irq_route;
	struct soc_intel_braswell_config *config = dev->chip_info;

	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));

	/* Set up the PIRQ PIC routing based on static config. */
	for (i = 0; i < NUM_PIRQS; i++)
		write8((void *)(pr_base + i*sizeof(ir->pic[i])),
			ir->pic[i]);

	/* Set up the per device PIRQ routing base on static config. */
	for (i = 0; i < NUM_IR_DEVS; i++)
		write16((void *)(ir_base + i*sizeof(ir->pcidev[i])),
			ir->pcidev[i]);

	/* Route SCI to IRQ9 */
	write32(actl, (read32(actl) & ~SCIS_MASK) | SCIS_IRQ9);

	sc_rtc_init();

	if (config->disable_slp_x_stretch_sus_fail) {
		printk(BIOS_DEBUG, "Disabling slp_x stretching.\n");
		write32(gen_pmcon1,
			read32(gen_pmcon1) | DIS_SLP_X_STRCH_SUS_UP);
	} else {
		write32(gen_pmcon1,
			read32(gen_pmcon1) & ~DIS_SLP_X_STRCH_SUS_UP);
	}

	if (acpi_slp_type == 3)
		com1_configure_resume(dev);
}

/*
 * Common code for the south cluster devices.
 */

/* Set bit in function disble register to hide this device. */
static void sc_disable_devfn(device_t dev)
{
	void *func_dis = (void *)(PMC_BASE_ADDRESS + FUNC_DIS);
	void *func_dis2 = (void *)(PMC_BASE_ADDRESS + FUNC_DIS2);
	uint32_t mask = 0;
	uint32_t mask2 = 0;

	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));

#define SET_DIS_MASK(name_) \
	case PCI_DEVFN(name_ ## _DEV, name_ ## _FUNC): \
		mask |= name_ ## _DIS
#define SET_DIS_MASK2(name_) \
	case PCI_DEVFN(name_ ## _DEV, name_ ## _FUNC): \
		mask2 |= name_ ## _DIS

	switch (dev->path.pci.devfn) {
	SET_DIS_MASK(SDIO);
		break;
	SET_DIS_MASK(SD);
		break;
	SET_DIS_MASK(SATA);
		break;
	SET_DIS_MASK(XHCI);
		/* Disable super speed PHY when XHCI is not available. */
		mask2 |= USH_SS_PHY_DIS;
		break;
	SET_DIS_MASK(LPE);
		break;
	SET_DIS_MASK(MMC);
		break;
	SET_DIS_MASK(SIO_DMA1);
		break;
	SET_DIS_MASK(I2C1);
		break;
	SET_DIS_MASK(I2C2);
		break;
	SET_DIS_MASK(I2C3);
		break;
	SET_DIS_MASK(I2C4);
		break;
	SET_DIS_MASK(I2C5);
		break;
	SET_DIS_MASK(I2C6);
		break;
	SET_DIS_MASK(I2C7);
		break;
	SET_DIS_MASK(TXE);
		break;
	SET_DIS_MASK(HDA);
		break;
	SET_DIS_MASK(PCIE_PORT1);
		break;
	SET_DIS_MASK(PCIE_PORT2);
		break;
	SET_DIS_MASK(PCIE_PORT3);
		break;
	SET_DIS_MASK(PCIE_PORT4);
		break;
	SET_DIS_MASK(SIO_DMA2);
		break;
	SET_DIS_MASK(PWM1);
		break;
	SET_DIS_MASK(PWM2);
		break;
	SET_DIS_MASK(HSUART1);
		break;
	SET_DIS_MASK(HSUART2);
		break;
	SET_DIS_MASK(SPI);
		break;
	SET_DIS_MASK2(SMBUS);
		break;
	}

	if (mask != 0) {
		write32(func_dis, read32(func_dis) | mask);
		/* Ensure posted write hits. */
		read32(func_dis);
	}

	if (mask2 != 0) {
		write32(func_dis2, read32(func_dis2) | mask2);
		/* Ensure posted write hits. */
		read32(func_dis2);
	}
}

static inline void set_d3hot_bits(device_t dev, int offset)
{
	uint32_t reg8;

	printk(BIOS_SPEW, "%s/%s ( %s, 0x%08x )\n",
			__FILE__, __func__, dev_name(dev), offset);
	printk(BIOS_DEBUG, "Power management CAP offset 0x%x.\n", offset);
	reg8 = pci_read_config8(dev, offset + 4);
	reg8 |= 0x3;
	pci_write_config8(dev, offset + 4, reg8);
}

/*
 * Parts of the audio subsystem are powered by the HDA device. Therefore, one
 * cannot put HDA into D3Hot. Instead perform this workaround to make some of
 * the audio paths work for LPE audio.
 */
static void hda_work_around(device_t dev)
{
	void *gctl = (void *)(TEMP_BASE_ADDRESS + 0x8);

	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));

	/* Need to set magic register 0x43 to 0xd7 in config space. */
	pci_write_config8(dev, 0x43, 0xd7);

	/*
	 * Need to set bit 0 of GCTL to take the device out of reset. However,
	 * that requires setting up the 64-bit BAR.
	 */
	pci_write_config32(dev, PCI_BASE_ADDRESS_0, TEMP_BASE_ADDRESS);
	pci_write_config32(dev, PCI_BASE_ADDRESS_1, 0);
	pci_write_config8(dev, PCI_COMMAND, PCI_COMMAND_MEMORY);
	write32(gctl, read32(gctl) | 0x1);
	pci_write_config8(dev, PCI_COMMAND, 0);
	pci_write_config32(dev, PCI_BASE_ADDRESS_0, 0);
}

static int place_device_in_d3hot(device_t dev)
{
	unsigned offset;

	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));

	/*
	 * Parts of the HDA block are used for LPE audio as well.
	 * Therefore assume the HDA will never be put into D3Hot.
	 */
	if (dev->path.pci.devfn == PCI_DEVFN(HDA_DEV, HDA_FUNC)) {
		hda_work_around(dev);
		return 0;
	}

	offset = pci_find_capability(dev, PCI_CAP_ID_PM);

	if (offset != 0) {
		set_d3hot_bits(dev, offset);
		return 0;
	}

	/*
	 * For some reason some of the devices don't have the capability
	 * pointer set correctly. Work around this by hard coding the offset.
	 */
#define DEV_CASE(name_) \
	case PCI_DEVFN(name_ ## _DEV, name_ ## _FUNC)

	switch (dev->path.pci.devfn) {
	DEV_CASE(SDIO) :
	DEV_CASE(SD) :
	DEV_CASE(MMC) :
	DEV_CASE(LPE) :
	DEV_CASE(SIO_DMA1) :
	DEV_CASE(I2C1) :
	DEV_CASE(I2C2) :
	DEV_CASE(I2C3) :
	DEV_CASE(I2C4) :
	DEV_CASE(I2C5) :
	DEV_CASE(I2C6) :
	DEV_CASE(I2C7) :
	DEV_CASE(SIO_DMA2) :
	DEV_CASE(PWM1) :
	DEV_CASE(PWM2) :
	DEV_CASE(HSUART1) :
	DEV_CASE(HSUART2) :
	DEV_CASE(SPI) :
		offset = 0x80;
		break;
	DEV_CASE(SATA) :
	DEV_CASE(XHCI) :
		offset = 0x70;
		break;
	DEV_CASE(HDA) :
	DEV_CASE(SMBUS) :
		offset = 0x50;
		break;
	DEV_CASE(TXE) :
		/* TXE cannot be placed in D3Hot. */
		return 0;
		break;
	DEV_CASE(PCIE_PORT1) :
	DEV_CASE(PCIE_PORT2) :
	DEV_CASE(PCIE_PORT3) :
	DEV_CASE(PCIE_PORT4) :
		offset = 0xa0;
		break;
	}

	if (offset != 0) {
		set_d3hot_bits(dev, offset);
		return 0;
	}

	return -1;
}

/* Common PCI device function disable. */
void southcluster_enable_dev(device_t dev)
{
	uint32_t reg32;

	printk(BIOS_SPEW, "%s/%s ( %s )\n",
			__FILE__, __func__, dev_name(dev));
	if (!dev->enabled) {
		int slot = PCI_SLOT(dev->path.pci.devfn);
		int func = PCI_FUNC(dev->path.pci.devfn);
		printk(BIOS_DEBUG, "%s: Disabling device: %02x.%01x\n",
		       dev_path(dev), slot, func);

		/* Ensure memory, io, and bus master are all disabled */
		reg32 = pci_read_config32(dev, PCI_COMMAND);
		reg32 &= ~(PCI_COMMAND_MASTER |
			   PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
		pci_write_config32(dev, PCI_COMMAND, reg32);

		/* Place device in D3Hot */
		if (place_device_in_d3hot(dev) < 0) {
			printk(BIOS_WARNING,
			       "Could not place %02x.%01x into D3Hot. "
			       "Keeping device visible.\n", slot, func);
			return;
		}
		/* Disable this device if possible */
		sc_disable_devfn(dev);
	} else {
		/* Enable SERR */
		reg32 = pci_read_config32(dev, PCI_COMMAND);
		reg32 |= PCI_COMMAND_SERR;
		pci_write_config32(dev, PCI_COMMAND, reg32);
	}
}

static struct device_operations device_ops = {
	.read_resources		= sc_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= NULL,
	.init			= sc_init,
	.enable			= southcluster_enable_dev,
	.scan_bus		= scan_static_bus,
	.ops_pci		= &soc_pci_ops,
};

static const struct pci_driver southcluster __pci_driver = {
	.ops		= &device_ops,
	.vendor		= PCI_VENDOR_ID_INTEL,
	.device		= LPC_DEVID,
};

int __attribute__((weak)) mainboard_get_spi_config(struct spi_config *cfg)
{
	printk(BIOS_SPEW, "%s/%s ( 0x%p )\n",
			__FILE__, __func__, (void *)cfg);
	return -1;
}

static void update_gnvs(device_t dev, int nvs_index, global_nvs_t *gnvs)
{

	struct resource *bar;
	/*
	 * ACPI code doesn't create a BAR1 address space
	 * Save BAR0 to ACPI NVS
	 */
	bar = find_resource(dev, PCI_BASE_ADDRESS_0);
	if (bar)
		gnvs->dev.scc_bar0[nvs_index] = (u32)bar->base;
	/* Device is enabled in ACPI mode */
	gnvs->dev.scc_en[nvs_index] = 1;
}

static void finalize_chipset(void *unused)
{
	void *bcr = (void *)(SPI_BASE_ADDRESS + BCR);
	void *gcs = (void *)(RCBA_BASE_ADDRESS + GCS);
	void *gen_pmcon2 = (void *)(PMC_BASE_ADDRESS + GEN_PMCON2);
	void *etr = (void *)(PMC_BASE_ADDRESS + ETR);
	uint8_t *spi = (uint8_t *)SPI_BASE_ADDRESS;
	struct spi_config cfg;
	global_nvs_t *gnvs;

	printk(BIOS_SPEW, "%s/%s ( 0x%p )\n",
			__FILE__, __func__, unused);
	/* Find ACPI NVS to update BARs */
	gnvs = (global_nvs_t *)cbmem_find(CBMEM_ID_ACPI_GNVS);
	if (!gnvs) {
		printk(BIOS_ERR, "Unable to locate Global NVS\n");
		return;
	}

	device_t dev0 = dev_find_slot(0, PCI_DEVFN(MMC_DEV, 0));
	/* Update the ACPI mode for SCC devices in GNVS */
	update_gnvs(dev0, SCC_NVS_MMC, gnvs);

	device_t dev1 = dev_find_slot(0, PCI_DEVFN(SD_DEV, 0));

	update_gnvs(dev1, SCC_NVS_SD, gnvs);

	/* Set the lock enable on the BIOS control register. */
	write32(bcr, read32(bcr) | BCR_LE);

	/* Set BIOS lock down bit controlling boot block size and swapping. */
	write32(gcs, read32(gcs) | BILD);

	/* Lock sleep stretching policy and set SMI lock. */
	write32(gen_pmcon2, read32(gen_pmcon2) | SLPSX_STR_POL_LOCK | SMI_LOCK);

	/*  Set the CF9 lock. */
	write32(etr, read32(etr) | CF9LOCK);

	if (mainboard_get_spi_config(&cfg) < 0) {
		printk(BIOS_DEBUG, "No SPI lockdown configuration.\n");
	} else {
		write16(spi + PREOP, cfg.preop);
		write16(spi + OPTYPE, cfg.optype);
		write32(spi + OPMENU0, cfg.opmenu[0]);
		write32(spi + OPMENU1, cfg.opmenu[1]);
		write16(spi + HSFSTS, read16(spi + HSFSTS) | FLOCKDN);
		write32(spi + UVSCC, cfg.uvscc);
		write32(spi + LVSCC, cfg.lvscc | VCL);
	}

	printk(BIOS_DEBUG, "Finalizing SMM.\n");
	outb(APM_CNT_FINALIZE, APM_CNT);
}

BOOT_STATE_INIT_ENTRIES(finalize_bscb) = {
	BOOT_STATE_INIT_ENTRY(BS_OS_RESUME, BS_ON_ENTRY,
			      finalize_chipset, NULL),
	BOOT_STATE_INIT_ENTRY(BS_PAYLOAD_LOAD, BS_ON_EXIT,
			      finalize_chipset, NULL),
};