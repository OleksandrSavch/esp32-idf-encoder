#pragma once

#include "esp_err.h"

/**
 * @brief Initialize the button on the specified GPIO pin.
 *
 * Configures the given GPIO pin as input with interrupt to detect button presses.
 *
 * @param pin GPIO number to which the button is connected.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t button_init(gpio_num_t pin);

/**
 * @brief Check if the button was pressed since the last call.
 *
 * This function returns true only once per press. It resets the internal flag after being called.
 *
 * @return true if the button was pressed, false otherwise.
 */
bool button_pressed_flag(void);
