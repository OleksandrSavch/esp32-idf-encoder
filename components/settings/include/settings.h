#pragma once

#include "esp_err.h"

/**
 * @brief Load settings from NVS or use defaults.
 * 
 * @param out_diameter Pointer to store diameter value (mm)
 * @param out_factor Pointer to store calibration factor
 * @return esp_err_t 
 */
esp_err_t settings_load(float* out_diameter, float* out_factor);

/**
 * @brief Save settings to NVS.
 * 
 * @param diameter Diameter in millimeters
 * @param factor Calibration factor
 * @return esp_err_t 
 */
esp_err_t settings_save(float diameter, float factor);
