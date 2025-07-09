#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_FREQ_HZ 100000
#define TAG "I2C_SCAN"

/**
 * @brief Scans the I2C bus for devices.
 * 
 * Initializes the I2C master driver on the specified port and pins,
 * then attempts to write to every possible 7-bit address (1-126).
 * Logs any detected devices.
 * 
 * @param port I2C port number (e.g., I2C_NUM_0)
 * @param sda GPIO number used for SDA line
 * @param scl GPIO number used for SCL line
 */

void i2c_scan_bus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
{
    ESP_LOGI(TAG, "Scanning I2C bus...");

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
        .clk_flags = 0
    };

    // Configure the I2C driver
    esp_err_t err = i2c_param_config(port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return;
    }

    // Install the I2C driver
    err = i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return;
    }

    // Scan through all possible I2C addresses (1 to 126)
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Found device at 0x%02X", addr);
        }
    }

    ESP_LOGI(TAG, "I2C scan complete.");

    err = i2c_driver_delete(port);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "i2c_driver_delete failed: %s", esp_err_to_name(err));
    }
}
