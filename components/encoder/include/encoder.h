#pragma once

#include "driver/gpio.h"
#include <stdint.h>

/**
 * @brief Initializes the encoder with specified GPIO pins for phases A and B.
 * 
 * @param pin_a GPIO pin for signal A
 * @param pin_b GPIO pin for signal B
 * @param pulses_per_revolution Number of pulses per one revolution
 * @param wheel_diameter_mm Diameter of the shaft or wheel in millimeters
 */
void encoder_init(gpio_num_t pin_a, gpio_num_t pin_b, int pulses_per_revolution, float wheel_diameter_mm);

/**
 * @brief Returns the total number of pulses (with direction).
 * 
 * @return int Current pulse count
 */
int encoder_get_pulses(void);

/**
 * @brief Resets the pulse counter to zero.
 */
void encoder_reset(void);

/**
 * @brief Calculates the distance traveled in meters.
 * 
 * @return float Distance traveled in meters
 */
float encoder_get_distance_m(void);

/**
 * @brief Calculates speed in meters per second.
 * 
 * @return float Speed in meters per second
 */
float encoder_get_speed_mps(void);

/**
 * @brief Should be called periodically to update the speed.
 */
void encoder_update_speed(void);

/**
 * @brief Starts a separate FreeRTOS task to calculate speed.
 */
void encoder_start_speed_task(void);

/**
 * @brief Sets a calibration factor to correct distance calculation.
 * 
 * @param factor Calibration factor (multiplied by the calculated distance)
 */
void encoder_set_calibration_factor(float factor);

/**
 * @brief Returns the current calibration factor.
 */
float encoder_get_calibration_factor(void);

/**
 * @brief Sets a new shaft or wheel diameter in millimeters.
 * 
 * @param diameter_mm New diameter value
 */
void encoder_set_wheel_diameter_mm(float diameter_mm);

/**
 * @brief Returns the current diameter value in millimeters.
 */
float encoder_get_wheel_diameter_mm(void);
