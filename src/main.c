/**
 * @brief Main application entry point.
 *
 * Initializes NVS flash, SPIFFS filesystem,
 * display, encoder settings (wheel diameter, calibration factor),
 * button on GPIO12,
 * starts Wi-Fi connection task,
 * and runs the main loop to update the display and handle button presses.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "button.h"

#include "myfs.h"
#include "wifi_connect.h"
#include "webserver.h"
#include "encoder.h"
#include "display.h"
#include "settings.h"

#define TAG_MAIN "MAIN"
#define BUTTON_GPIO 12

void app_main(void) {
    // Get and log the reason for the last reset
    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGI(TAG_MAIN, "Reset reason: %d", reason);
    ESP_LOGI(TAG_MAIN, "===== app_main started =====");

    // Initialize NVS (non-volatile storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize SPIFFS filesystem and list files for diagnostics
    myfs_init();
    list_spiffs_files();

    // Initialize display (I2C 16x2 LCD)
    if (display_init() != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Display init failed");
    }

    // Load encoder settings: wheel diameter and calibration factor
    float diameter = 100.0f;    // in millimeters
    float factor = 1.0f;
    if (settings_load(&diameter, &factor)) {
        ESP_LOGI(TAG_MAIN, "Loaded settings: diameter=%.2f, factor=%.3f", diameter, factor);
    } else {
        ESP_LOGW(TAG_MAIN, "Using default settings");
    }

    // Initialize encoder with given parameters
    encoder_init(GPIO_NUM_13, GPIO_NUM_14, 600, diameter);
    encoder_set_calibration_factor(factor);
    encoder_start_speed_task();

    // Initialize hardware button
    button_init(BUTTON_GPIO);

    // Start Wi-Fi connection task pinned to core 1
    xTaskCreatePinnedToCore(wifi_connect_task, "wifi_connect_task", 4096, NULL, 5, NULL, 1);

    // Main loop to display status and handle button presses
    while (1) {
        display_show_status(encoder_get_speed_mps(), encoder_get_distance_m());

        if (button_pressed_flag()) {
            encoder_reset();
            ESP_LOGI(TAG_MAIN, "Button pressed — encoder reset");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));    // Delay 1 second
    }
}