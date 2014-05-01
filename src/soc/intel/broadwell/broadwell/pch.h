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

#ifndef SOUTHBRIDGE_INTEL_LYNXPOINT_PCH_H
#define SOUTHBRIDGE_INTEL_LYNXPOINT_PCH_H

/*
 * Lynx Point PCH PCI Devices:
 *
 * Bus 0:Device 31:Function 0 LPC Controller1
 * Bus 0:Device 31:Function 2 SATA Controller #1
 * Bus 0:Device 31:Function 3 SMBus Controller
 * Bus 0:Device 31:Function 5 SATA Controller #22
 * Bus 0:Device 31:Function 6 Thermal Subsystem
 * Bus 0:Device 29:Function 03 USB EHCI Controller #1
 * Bus 0:Device 26:Function 03 USB EHCI Controller #2
 * Bus 0:Device 28:Function 0 PCI Express* Port 1
 * Bus 0:Device 28:Function 1 PCI Express Port 2
 * Bus 0:Device 28:Function 2 PCI Express Port 3
 * Bus 0:Device 28:Function 3 PCI Express Port 4
 * Bus 0:Device 28:Function 4 PCI Express Port 5
 * Bus 0:Device 28:Function 5 PCI Express Port 6
 * Bus 0:Device 28:Function 6 PCI Express Port 7
 * Bus 0:Device 28:Function 7 PCI Express Port 8
 * Bus 0:Device 27:Function 0 Intel High Definition Audio Controller
 * Bus 0:Device 25:Function 0 Gigabit Ethernet Controller
 * Bus 0:Device 22:Function 0 Intel Management Engine Interface #1
 * Bus 0:Device 22:Function 1 Intel Management Engine Interface #2
 * Bus 0:Device 22:Function 2 IDE-R
 * Bus 0:Device 22:Function 3 KT
 * Bus 0:Device 20:Function 0 xHCI Controller
*/

/* PCH types */
#define PCH_TYPE_LPT		0x8c
#define PCH_TYPE_LPT_LP		0x9c

/* PCH stepping values for LPC device */
#define LPT_H_STEP_B0		0x02
#define LPT_H_STEP_C0		0x03
#define LPT_H_STEP_C1		0x04
#define LPT_H_STEP_C2		0x05
#define LPT_LP_STEP_B0		0x02
#define LPT_LP_STEP_B1		0x03
#define LPT_LP_STEP_B2		0x04

/*
 * It does not matter where we put the SMBus I/O base, as long as we
 * keep it consistent and don't interfere with other devices.  Stage2
 * will relocate this anyways.
 * Our solution is to have SMB initialization move the I/O to SMBUS_IO_BASE
 * again. But handling static BARs is a generic problem that should be
 * solved in the device allocator.
 */
#define SMBUS_IO_BASE		0x0400
#define SMBUS_SLAVE_ADDR	0x24

#if CONFIG_INTEL_LYNXPOINT_LP
#define DEFAULT_PMBASE		0x1000
#define DEFAULT_GPIOBASE	0x1400
#define DEFAULT_GPIOSIZE	0x400
#else
#define DEFAULT_PMBASE		0x500
#define DEFAULT_GPIOBASE	0x480
#define DEFAULT_GPIOSIZE	0x80
#endif

#define HPET_ADDR		0xfed00000
#define DEFAULT_RCBA		0xfed1c000

#ifndef __ACPI__

#if defined (__SMM__) && !defined(__ASSEMBLER__)
void intel_pch_finalize_smm(void);
void usb_ehci_sleep_prepare(device_t dev, u8 slp_typ);
void usb_ehci_disable(device_t dev);
void usb_xhci_sleep_prepare(device_t dev, u8 slp_typ);
void usb_xhci_route_all(void);
#endif

#if !defined(__ASSEMBLER__)
void pch_config_rcba(const struct rcba_config_instruction *rcba_config);
int pch_silicon_revision(void);
int pch_silicon_type(void);
int pch_is_lp(void);
u16 get_pmbase(void);
u16 get_gpiobase(void);

/*
 * get GPIO pin value
 */
int get_gpio(int gpio_num);
/*
 * Get a number comprised of multiple GPIO values. gpio_num_array points to
 * the array of gpio pin numbers to scan, terminated by -1.
 */
unsigned get_gpios(const int *gpio_num_array);
/*
 * Set GPIO pin value.
 */
void set_gpio(int gpio_num, int value);
/* Return non-zero if gpio is set to native function. 0 otherwise. */
int gpio_is_native(int gpio_num);

#if !defined(__PRE_RAM__) && !defined(__SMM__)
#include <device/device.h>
#include <arch/acpi.h>
#include "chip.h"
void pch_enable(device_t dev);
void pch_disable_devfn(device_t dev);
u32 pch_iobp_read(u32 address);
void pch_iobp_write(u32 address, u32 data);
void pch_iobp_update(u32 address, u32 andvalue, u32 orvalue);
#if CONFIG_ELOG
void pch_log_state(void);
#endif
void acpi_create_intel_hpet(acpi_hpet_t * hpet);
void acpi_create_serialio_ssdt(acpi_header_t *ssdt);

/* These helpers are for performing SMM relocation. */
void southbridge_trigger_smi(void);
void southbridge_clear_smi_status(void);
/* The initialization of the southbridge is split into 2 compoments. One is
 * for clearing the state in the SMM registers. The other is for enabling
 * SMIs. They are split so that other work between the 2 actions. */
void southbridge_smm_clear_state(void);
void southbridge_smm_enable_smi(void);
#else
void enable_smbus(void);
void enable_usb_bar(void);
int smbus_read_byte(unsigned device, unsigned address);
int early_spi_read(u32 offset, u32 size, u8 *buffer);
int early_pch_init(const void *gpio_map,
                   const struct rcba_config_instruction *rcba_config);
void pch_enable_lpc(void);
#endif /* !__PRE_RAM__ && !__SMM__ */
#endif /* __ASSEMBLER__ */


#ifndef CONFIG_MAINBOARD_POWER_ON_AFTER_POWER_FAIL
#define CONFIG_MAINBOARD_POWER_ON_AFTER_POWER_FAIL MAINBOARD_POWER_ON
#endif

/* PCI Configuration Space (D30:F0): PCI2PCI */
#define PSTS	0x06
#define SMLT	0x1b
#define SECSTS	0x1e
#define INTR	0x3c
#define BCTRL	0x3e
#define   SBR	(1 << 6)
#define   SEE	(1 << 1)
#define   PERE	(1 << 0)

/* Power Management Control and Status */
#define PCH_PCS			0x84
#define  PCH_PCS_PS_D3HOT	3

#define PCH_EHCI1_DEV		PCI_DEV(0, 0x1d, 0)
#define PCH_EHCI2_DEV		PCI_DEV(0, 0x1a, 0)
#define PCH_XHCI_DEV		PCI_DEV(0, 0x14, 0)
#define PCH_ME_DEV		PCI_DEV(0, 0x16, 0)
#define PCH_PCIE_DEV_SLOT	28

/* PCI Configuration Space (D31:F0): LPC */
#define PCH_LPC_DEV		PCI_DEV(0, 0x1f, 0)
#define SERIRQ_CNTL		0x64

#define GEN_PMCON_1		0xa0
#define GEN_PMCON_2		0xa2
#define GEN_PMCON_3		0xa4
#define PMIR			0xac
#define  PMIR_CF9LOCK		(1 << 31)
#define  PMIR_CF9GR		(1 << 20)

/* GEN_PMCON_3 bits */
#define RTC_BATTERY_DEAD	(1 << 2)
#define RTC_POWER_FAILED	(1 << 1)
#define SLEEP_AFTER_POWER_FAIL	(1 << 0)

#define PMBASE			0x40
#define ACPI_CNTL		0x44
#define BIOS_CNTL		0xDC
#define GPIO_BASE		0x48 /* LPC GPIO Base Address Register */
#define GPIO_CNTL		0x4C /* LPC GPIO Control Register */
#define GPIO_ROUT		0xb8

#define PIRQA_ROUT		0x60
#define PIRQB_ROUT		0x61
#define PIRQC_ROUT		0x62
#define PIRQD_ROUT		0x63
#define PIRQE_ROUT		0x68
#define PIRQF_ROUT		0x69
#define PIRQG_ROUT		0x6A
#define PIRQH_ROUT		0x6B

#define LPC_IO_DEC		0x80 /* IO Decode Ranges Register */
#define LPC_EN			0x82 /* LPC IF Enables Register */
#define  CNF2_LPC_EN		(1 << 13) /* 0x4e/0x4f */
#define  CNF1_LPC_EN		(1 << 12) /* 0x2e/0x2f */
#define  MC_LPC_EN		(1 << 11) /* 0x62/0x66 */
#define  KBC_LPC_EN		(1 << 10) /* 0x60/0x64 */
#define  GAMEH_LPC_EN		(1 << 9)  /* 0x208/0x20f */
#define  GAMEL_LPC_EN		(1 << 8)  /* 0x200/0x207 */
#define  FDD_LPC_EN		(1 << 3)  /* LPC_IO_DEC[12] */
#define  LPT_LPC_EN		(1 << 2)  /* LPC_IO_DEC[9:8] */
#define  COMB_LPC_EN		(1 << 1)  /* LPC_IO_DEC[6:4] */
#define  COMA_LPC_EN		(1 << 0)  /* LPC_IO_DEC[2:0] */
#define LPC_GEN1_DEC		0x84 /* LPC IF Generic Decode Range 1 */
#define LPC_GEN2_DEC		0x88 /* LPC IF Generic Decode Range 2 */
#define LPC_GEN3_DEC		0x8c /* LPC IF Generic Decode Range 3 */
#define LPC_GEN4_DEC		0x90 /* LPC IF Generic Decode Range 4 */
#define LGMR			0x98 /* LPC Generic Memory Range */

/* EHCI PCI Registers */
#define EHCI_PWR_CTL_STS	0x54
#define  PWR_CTL_SET_MASK	0x3
#define  PWR_CTL_SET_D0		0x0
#define  PWR_CTL_SET_D3		0x3
#define  PWR_CTL_ENABLE_PME	(1 << 8)
#define  PWR_CTL_STATUS_PME	(1 << 15)

/* EHCI Memory Registers */
#define EHCI_USB_CMD		0x20
#define  EHCI_USB_CMD_RUN	(1 << 0)
#define  EHCI_USB_CMD_PSE	(1 << 4)
#define  EHCI_USB_CMD_ASE	(1 << 5)
#define EHCI_PORTSC(port)	(0x64 + (port * 4))
#define  EHCI_PORTSC_ENABLED	(1 << 2)
#define  EHCI_PORTSC_SUSPEND	(1 << 7)

/* XHCI PCI Registers */
#define XHCI_PWR_CTL_STS	0x74
#define XHCI_USB2PR		0xd0
#define XHCI_USB2PRM		0xd4
#define  XHCI_USB2PR_HCSEL	0x7fff
#define XHCI_USB3PR		0xd8
#define  XHCI_USB3PR_SSEN	0x3f
#define XHCI_USB3PRM		0xdc
#define XHCI_USB3FUS		0xe0
#define  XHCI_USB3FUS_SS_MASK	3
#define  XHCI_USB3FUS_SS_SHIFT	3
#define XHCI_USB3PDO		0xe8

/* XHCI Memory Registers */
#define XHCI_USB3_PORTSC(port)	((pch_is_lp() ? 0x510 : 0x570) + (port * 0x10))
#define  XHCI_USB3_PORTSC_CHST	(0x7f << 17)
#define  XHCI_USB3_PORTSC_WCE	(1 << 25)	/* Wake on Connect */
#define  XHCI_USB3_PORTSC_WDE	(1 << 26)	/* Wake on Disconnect */
#define  XHCI_USB3_PORTSC_WOE	(1 << 27)	/* Wake on Overcurrent */
#define  XHCI_USB3_PORTSC_WRC	(1 << 19)	/* Warm Reset Complete */
#define  XHCI_USB3_PORTSC_LWS  	(1 << 16)	/* Link Write Strobe */
#define  XHCI_USB3_PORTSC_PED 	(1 << 1)	/* Port Enabled/Disabled */
#define  XHCI_USB3_PORTSC_WPR	(1 << 31)	/* Warm Port Reset */
#define  XHCI_USB3_PORTSC_PLS	(0xf << 5)	/* Port Link State */
#define   XHCI_PLSR_DISABLED	(4 << 5)	/* Port is disabled */
#define   XHCI_PLSR_RXDETECT	(5 << 5)	/* Port is disconnected */
#define   XHCI_PLSR_POLLING	(7 << 5)	/* Port is polling */
#define   XHCI_PLSW_ENABLE	(5 << 5)	/* Transition from disabled */

/* Serial IO IOBP Registers */
#define SIO_IOBP_PORTCTRL0	0xcb000000	/* SDIO D23:F0 */
#define  SIO_IOBP_PORTCTRL0_ACPI_IRQ_EN		(1 << 5)
#define  SIO_IOBP_PORTCTRL0_PCI_CONF_DIS	(1 << 4)
#define SIO_IOBP_PORTCTRL1	0xcb000014	/* SDIO D23:F0 */
#define  SIO_IOBP_PORTCTRL1_SNOOP_SELECT(x)	(((x) & 3) << 13)
#define SIO_IOBP_GPIODF		0xcb000154
#define  SIO_IOBP_GPIODF_SDIO_IDLE_DET_EN	(1 << 4)
#define  SIO_IOBP_GPIODF_DMA_IDLE_DET_EN	(1 << 3)
#define  SIO_IOBP_GPIODF_UART_IDLE_DET_EN	(1 << 2)
#define  SIO_IOBP_GPIODF_I2C_IDLE_DET_EN	(1 << 1)
#define  SIO_IOBP_GPIODF_SPI_IDLE_DET_EN	(1 << 0)
#define SIO_IOBP_PORTCTRL2	0xcb000240	/* DMA D21:F0 */
#define SIO_IOBP_PORTCTRL3	0xcb000248	/* I2C0 D21:F1 */
#define SIO_IOBP_PORTCTRL4	0xcb000250	/* I2C1 D21:F2 */
#define SIO_IOBP_PORTCTRL5	0xcb000258	/* SPI0 D21:F3 */
#define SIO_IOBP_PORTCTRL6	0xcb000260	/* SPI1 D21:F4 */
#define SIO_IOBP_PORTCTRL7	0xcb000268	/* UART0 D21:F5 */
#define SIO_IOBP_PORTCTRL8	0xcb000270	/* UART1 D21:F6 */
#define SIO_IOBP_PORTCTRLX(x)	(0xcb000240 + ((x) * 8))
/* PORTCTRL 2-8 have the same layout */
#define  SIO_IOBP_PORTCTRL_ACPI_IRQ_EN		(1 << 21)
#define  SIO_IOBP_PORTCTRL_PCI_CONF_DIS		(1 << 20)
#define  SIO_IOBP_PORTCTRL_SNOOP_SELECT(x)	(((x) & 3) << 18)
#define  SIO_IOBP_PORTCTRL_INT_PIN(x)		(((x) & 0xf) << 2)
#define  SIO_IOBP_PORTCTRL_PM_CAP_PRSNT		(1 << 1)
#define SIO_IOBP_FUNCDIS0	0xce00aa07	/* DMA D21:F0 */
#define SIO_IOBP_FUNCDIS1	0xce00aa47	/* I2C0 D21:F1 */
#define SIO_IOBP_FUNCDIS2	0xce00aa87	/* I2C1 D21:F2 */
#define SIO_IOBP_FUNCDIS3	0xce00aac7	/* SPI0 D21:F3 */
#define SIO_IOBP_FUNCDIS4	0xce00ab07	/* SPI1 D21:F4 */
#define SIO_IOBP_FUNCDIS5	0xce00ab47	/* UART0 D21:F5 */
#define SIO_IOBP_FUNCDIS6	0xce00ab87	/* UART1 D21:F6 */
#define SIO_IOBP_FUNCDIS7	0xce00ae07	/* SDIO D23:F0 */
#define  SIO_IOBP_FUNCDIS_DIS			(1 << 8)

/* Serial IO Devices */
#define SIO_ID_SDMA		0 /* D21:F0 */
#define SIO_ID_I2C0		1 /* D21:F1 */
#define SIO_ID_I2C1		2 /* D21:F2 */
#define SIO_ID_SPI0		3 /* D21:F3 */
#define SIO_ID_SPI1		4 /* D21:F4 */
#define SIO_ID_UART0		5 /* D21:F5 */
#define SIO_ID_UART1		6 /* D21:F6 */
#define SIO_ID_SDIO		7 /* D23:F0 */

#define SIO_REG_PPR_CLOCK		0x800
#define  SIO_REG_PPR_CLOCK_EN		 (1 << 0)
#define SIO_REG_PPR_RST			0x804
#define  SIO_REG_PPR_RST_ASSERT		 0x3
#define SIO_REG_PPR_GEN			0x808
#define  SIO_REG_PPR_GEN_LTR_MODE_MASK	 (1 << 2)
#define  SIO_REG_PPR_GEN_VOLTAGE_MASK	 (1 << 3)
#define  SIO_REG_PPR_GEN_VOLTAGE(x)	 ((x & 1) << 3)
#define SIO_REG_AUTO_LTR		0x814

#define SIO_REG_SDIO_PPR_GEN		0x1008
#define SIO_REG_SDIO_PPR_SW_LTR		0x1010
#define SIO_REG_SDIO_PPR_CMD12		0x3c
#define  SIO_REG_SDIO_PPR_CMD12_B30	 (1 << 30)

#define SIO_PIN_INTA 1 /* IRQ5 in ACPI mode */
#define SIO_PIN_INTB 2 /* IRQ6 in ACPI mode */
#define SIO_PIN_INTC 3 /* IRQ7 in ACPI mode */
#define SIO_PIN_INTD 4 /* IRQ13 in ACPI mode */

/* PCI Configuration Space (D31:F3): SMBus */
#define PCH_SMBUS_DEV		PCI_DEV(0, 0x1f, 3)
#define SMB_BASE		0x20
#define HOSTC			0x40
#define SMB_RCV_SLVA		0x09

/* HOSTC bits */
#define I2C_EN			(1 << 2)
#define SMB_SMI_EN		(1 << 1)
#define HST_EN			(1 << 0)

/* SMBus I/O bits. */
#define SMBHSTSTAT		0x0
#define SMBHSTCTL		0x2
#define SMBHSTCMD		0x3
#define SMBXMITADD		0x4
#define SMBHSTDAT0		0x5
#define SMBHSTDAT1		0x6
#define SMBBLKDAT		0x7
#define SMBTRNSADD		0x9
#define SMBSLVDATA		0xa
#define SMLINK_PIN_CTL		0xe
#define SMBUS_PIN_CTL		0xf

#define SMBUS_TIMEOUT		(10 * 1000 * 100)


/* Southbridge IO BARs */

#define GPIOBASE		0x48

#define PMBASE		0x40

/*
 * SPI Opcode Menu setup for SPIBAR lockdown
 * should support most common flash chips.
 */

#define SPIBAR_OFFSET 0x3800
#define SPIBAR8(x) RCBA8(x + SPIBAR_OFFSET)
#define SPIBAR16(x) RCBA16(x + SPIBAR_OFFSET)
#define SPIBAR32(x) RCBA32(x + SPIBAR_OFFSET)

/* Reigsters within the SPIBAR */
#define SSFC 0x91
#define FDOC 0xb0
#define FDOD 0xb4

#define SPI_OPMENU_0 0x01 /* WRSR: Write Status Register */
#define SPI_OPTYPE_0 0x01 /* Write, no address */

#define SPI_OPMENU_1 0x02 /* BYPR: Byte Program */
#define SPI_OPTYPE_1 0x03 /* Write, address required */

#define SPI_OPMENU_2 0x03 /* READ: Read Data */
#define SPI_OPTYPE_2 0x02 /* Read, address required */

#define SPI_OPMENU_3 0x05 /* RDSR: Read Status Register */
#define SPI_OPTYPE_3 0x00 /* Read, no address */

#define SPI_OPMENU_4 0x20 /* SE20: Sector Erase 0x20 */
#define SPI_OPTYPE_4 0x03 /* Write, address required */

#define SPI_OPMENU_5 0x9f /* RDID: Read ID */
#define SPI_OPTYPE_5 0x00 /* Read, no address */

#define SPI_OPMENU_6 0xd8 /* BED8: Block Erase 0xd8 */
#define SPI_OPTYPE_6 0x03 /* Write, address required */

#define SPI_OPMENU_7 0x0b /* FAST: Fast Read */
#define SPI_OPTYPE_7 0x02 /* Read, address required */

#define SPI_OPMENU_UPPER ((SPI_OPMENU_7 << 24) | (SPI_OPMENU_6 << 16) | \
			  (SPI_OPMENU_5 << 8) | SPI_OPMENU_4)
#define SPI_OPMENU_LOWER ((SPI_OPMENU_3 << 24) | (SPI_OPMENU_2 << 16) | \
			  (SPI_OPMENU_1 << 8) | SPI_OPMENU_0)

#define SPI_OPTYPE ((SPI_OPTYPE_7 << 14) | (SPI_OPTYPE_6 << 12) | \
		    (SPI_OPTYPE_5 << 10) | (SPI_OPTYPE_4 << 8) |  \
		    (SPI_OPTYPE_3 << 6) | (SPI_OPTYPE_2 << 4) |	  \
		    (SPI_OPTYPE_1 << 2) | (SPI_OPTYPE_0))

#define SPI_OPPREFIX ((0x50 << 8) | 0x06) /* EWSR and WREN */

#define SPIBAR_HSFS                 0x3804   /* SPI hardware sequence status */
#define  SPIBAR_HSFS_SCIP           (1 << 5) /* SPI Cycle In Progress */
#define  SPIBAR_HSFS_AEL            (1 << 2) /* SPI Access Error Log */
#define  SPIBAR_HSFS_FCERR          (1 << 1) /* SPI Flash Cycle Error */
#define  SPIBAR_HSFS_FDONE          (1 << 0) /* SPI Flash Cycle Done */
#define SPIBAR_HSFC                 0x3806   /* SPI hardware sequence control */
#define  SPIBAR_HSFC_BYTE_COUNT(c)  (((c - 1) & 0x3f) << 8)
#define  SPIBAR_HSFC_CYCLE_READ     (0 << 1) /* Read cycle */
#define  SPIBAR_HSFC_CYCLE_WRITE    (2 << 1) /* Write cycle */
#define  SPIBAR_HSFC_CYCLE_ERASE    (3 << 1) /* Erase cycle */
#define  SPIBAR_HSFC_GO             (1 << 0) /* GO: start SPI transaction */
#define SPIBAR_FADDR                0x3808   /* SPI flash address */
#define SPIBAR_FDATA(n)             (0x3810 + (4 * n)) /* SPI flash data */

#endif /* __ACPI__ */
#endif /* SOUTHBRIDGE_INTEL_LYNXPOINT_PCH_H */
