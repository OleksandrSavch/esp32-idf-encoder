#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the LCD display.
 *
 * @return ESP_OK on success, or error code on failure.
 */
esp_err_t display_init(void);

/**
 * @brief Update the displayed distance value.
 *
 * @param meters Distance in meters to display.
 */
void display_update_distance(float meters);

/**
 * @brief Update the displayed speed value.
 *
 * @param mps Speed in meters per second to display.
 */
void display_update_speed(float mps);

/**
 * @brief Show a static message on the display (overrides speed/distance).
 *
 * @param msg Null-terminated string message.
 */
void display_show_message(const char* msg);     

/**
 * @brief Show both speed and distance together.
 *
 * @param speed Speed in m/s.
 * @param distance Distance in meters.
 */
void display_show_status(float speed, float distance);

/**
 * @brief Display the device's IP address on the screen.
 */
void display_show_ip(void);

#ifdef __cplusplus
}
#endif
