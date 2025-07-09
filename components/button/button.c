#include "driver/gpio.h"
#include <stdbool.h>       // ← for bool
#include "esp_err.h"
#include "button.h"
#include "esp_attr.h"     // or "esp_intr_alloc.h" — IRAM_ATTR

// button.c - realization of button handling
// This file implements the button handling functionality for the ESP32.

static gpio_num_t button_pin;
static volatile bool pressed = false;

/**
 * @brief GPIO interrupt handler for the button.
 *
 * This ISR sets a flag indicating that the button was pressed.
 *
 * @param arg Not used.
 */

static void IRAM_ATTR button_isr_handler(void* arg) {
    pressed = true;
}

/**
 * @brief Initialize the button GPIO and configure interrupt.
 *
 * Configures the specified GPIO pin as an input with internal pull-up resistor
 * and sets up an interrupt on falling edge to detect button press.
 *
 * @param pin GPIO number where the button is connected.
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t button_init(gpio_num_t pin) {
    button_pin = pin;

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, button_isr_handler, NULL);

    return ESP_OK;
}

/**
 * @brief Check whether the button was pressed.
 *
 * Returns true if the button was pressed since the last check.
 * This resets the internal press flag after returning true.
 *
 * @return true if button was pressed, false otherwise.
 */

bool button_pressed_flag(void) {
    if (pressed) {
        pressed = false;
        return true;
    }
    return false;
}
