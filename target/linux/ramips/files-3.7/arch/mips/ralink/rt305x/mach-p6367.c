/*
 *  Pentagram P6367 board support
 *
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/ethtool.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <asm/mach-ralink/machine.h>
#include <asm/mach-ralink/dev-gpio-buttons.h>
#include <asm/mach-ralink/dev-gpio-leds.h>
#include <asm/mach-ralink/rt305x.h>
#include <asm/mach-ralink/rt305x_regs.h>
#include <asm/mach-ralink/ramips_eth_platform.h>

#include "devices.h"

#define P6367_GPIO_LED_SYS      11
#define P6367_GPIO_LED_AP       12
#define P6367_GPIO_LED_WISP     9
#define P6367_GPIO_LED_WLAN     13
#define P6367_GPIO_LED_3G       7
#define P6367_GPIO_LED_WPS      14



#define P6367_GPIO_BUTTON_RESET	10

#define P6367_KEYS_POLL_INTERVAL	20
#define P6367_KEYS_DEBOUNCE_INTERVAL (3 * P6367_KEYS_POLL_INTERVAL)

const struct flash_platform_data p6367_flash = {
	.type		= "mx25l3205d",
};

struct spi_board_info p6367_spi_slave_info[] __initdata = {
	{
		.modalias	= "m25p80",
		.platform_data	= &p6367_flash,
		.irq		= -1,
		.max_speed_hz	= 10000000,
		.bus_num	= 0,
		.chip_select	= 0,
	},
};

static struct gpio_led p6367_leds_gpio[] __initdata = {
	{
		.name		= "p6367:green:sys",
		.gpio		= P6367_GPIO_LED_SYS,
		.active_low	= 1,
	}, {
		.name		= "p6367:green:ap",
		.gpio		= P6367_GPIO_LED_AP,
		.active_low	= 1,
	}, {
		.name		= "p6367:green:wisp",
		.gpio		= P6367_GPIO_LED_WISP,
		.active_low	= 1,
	}, {
		.name		= "p6367:green:wlan",
		.gpio		= P6367_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "p6367:green:3g",
		.gpio		= P6367_GPIO_LED_3G,
		.active_low	= 1,
	}, {
		.name		= "p6367:green:wps",
		.gpio		= P6367_GPIO_LED_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button p6367_gpio_buttons[] __initdata = {
	{
		.desc		= "RESET/WPS",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = P6367_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= P6367_GPIO_BUTTON_RESET,
		.active_low	= 1,
	}
};

static void __init p6367_init(void)
{
	rt305x_register_flash(0);

	rt305x_gpio_init(RT305X_GPIO_MODE_GPIO << RT305X_GPIO_MODE_UART0_SHIFT);

    rt305x_register_spi(p6367_spi_slave_info,
			    ARRAY_SIZE(p6367_spi_slave_info));
	rt305x_esw_data.vlan_config = RT305X_ESW_VLAN_CONFIG_WLLLL;
	rt305x_register_ethernet();
	ramips_register_gpio_leds(-1, ARRAY_SIZE(p6367_leds_gpio),
				  p6367_leds_gpio);
	ramips_register_gpio_buttons(-1, P6367_KEYS_POLL_INTERVAL,
				     ARRAY_SIZE(p6367_gpio_buttons),
				     p6367_gpio_buttons);
	rt305x_register_wifi();
	rt305x_register_wdt();
	rt305x_register_usb();
}

MIPS_MACHINE(RAMIPS_MACH_P6367, "P6367", "Pentagram P6367",
	     p6367_init);
