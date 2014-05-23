#ifndef __MAINBOARD_ONBOARD_H
#define __MAINBOARD_ONBOARD_H

#ifndef __ACPI__
void lan_init(void);
#endif

/* defines for programming the MAC address */
#define MCCLOUD_NIC_VENDOR_ID		0x10EC
#define MCCLOUD_NIC_DEVICE_ID		0x8168

/* 0x00: White LINK LED and Amber ACTIVE LED */
#define MCCLOUD_NIC_LED_MODE		0x00

/* NIC wake is GPIO 8 */
#define MCCLOUD_NIC_WAKE_GPIO		8

/* WLAN wake is GPIO 10 */
#define MCCLOUD_WLAN_WAKE_GPIO		10

#endif
