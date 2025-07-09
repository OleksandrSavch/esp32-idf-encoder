#pragma once

#include "driver/i2c.h"

/**
 * @brief Scans the specified I2C bus for connected devices.
 *
 * Initializes the I2C master interface on the given port and GPIO pins,
 * then probes all possible addresses to detect devices.
 *
 * @param port I2C port number (e.g., I2C_NUM_0)
 * @param sda GPIO number for SDA line
 * @param scl GPIO number for SCL line
 */

void i2c_scan_bus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

