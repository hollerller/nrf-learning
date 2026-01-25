/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
/* STEP 3 - Include the header file of the I2C API */
#include <zephyr/drivers/i2c.h>

#include <zephyr/drivers/gpio.h>

/* STEP 4.1 - Include the header file of printk() */
#include <zephyr/sys/printk.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* STEP 8 - Define the the addresses of relevant sensor registers and settings */

#define LUX_THRESHOLD 50.0f

/* STEP 6 - Get the node identifier of the sensor */
#define I2C_NODE DT_NODELABEL(bh1750)
#define LED_NODE DT_ALIAS(my_alert)
#define POWER_ON 0x01
#define MEASURE_MODE 0x10

int main(void)
{

	int ret;

	/* STEP 7 - Retrieve the API-specific device structure and make sure that the device is
	 * ready to use  */
	static const struct i2c_dt_spec bh1750 = I2C_DT_SPEC_GET(I2C_NODE);

	static const struct gpio_dt_spec greenLed = GPIO_DT_SPEC_GET(LED_NODE, gpios);

	if (!device_is_ready(bh1750.bus))
	{
		printk("I2C bus %s is not ready!\n\r", bh1750.bus->name);
		return -1;
	}
	else
	{
		printk("Sensor connected!");
	}
	/* STEP 9 - Setup the sensor by writing the value 0x2A to the MODE_CONTROL1 register
	0x2A Means : IR Gain: 1x, RGB Gain: 1x, Measurement mode: 120ms mode
	*/

	uint8_t cmd;

	cmd = POWER_ON; // 0x01
	// Write commands to turn on the device

	ret = i2c_write_dt(&bh1750, &cmd, 1);
	k_msleep(10);

	if (ret != 0)
	{
		printk("Failed to turn on device");
		return -1;
	}

	// Configure device on continuous H-resolution mode
	cmd = MEASURE_MODE;

	ret = i2c_write_dt(&bh1750, &cmd, 1);
	k_msleep(180);

	if (ret != 0)
	{
		printk("Failed to configure measure mode");
		return -1;
	}

	// Configure pin as output for LED

	if (!gpio_is_ready_dt(&greenLed))
	{
		return 0;
	}

	ret = gpio_pin_configure_dt(&greenLed, GPIO_OUTPUT_LOW);
	if (ret < 0)
	{
		return 0;
	}

	/* STEP 10 - Enable measurement by writing 1 to bit 4 of the MODE_CONTROL2 register */

	while (1)
	{
		/* STEP 11 - Read the RGB values from the sensor */

		uint8_t data[2];

		if (i2c_read_dt(&bh1750, data, 2) == 0)
		{
			uint16_t raw = (data[0] << 8) | data[1];
			float lux = raw / 1.2f;
			printk("Light: %.2f lux\n", lux);

			if (lux < 10)
			{
				printk("Se fue la luz: %.2f Compren velas\n", lux);
				gpio_pin_set_dt(&greenLed, 0);
			}
			else
			{
				gpio_pin_set_dt(&greenLed, 1);
			}
		}

		k_msleep(SLEEP_TIME_MS);
	}
}
