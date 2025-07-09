#pragma once

#include "esp_err.h"
#include <stdbool.h> 

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connects to the configured Wi-Fi network.
 *
 * @return ESP_OK if connection is successful, error code otherwise.
 */
esp_err_t wifi_connect(void);

/**
 * @brief Checks whether Wi-Fi is currently connected.
 *
 * @return true if connected, false otherwise.
 */
bool wifi_is_connected(void);

/**
 * @brief FreeRTOS task function that manages Wi-Fi connection.
 *
 * This task handles Wi-Fi connection lifecycle, including retries.
 *
 * @param pvParameters Task parameters (unused).
 */
void wifi_connect_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
