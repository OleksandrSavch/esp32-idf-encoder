#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_err.h"

/**
 * @brief Load calibration factor from NVS and apply it to the encoder.
 *
 * This function reads the calibration factor from non-volatile storage (NVS)
 * and updates the encoder module accordingly.
 */
void calibration_load(void);

/**
 * @brief Save a new calibration factor to NVS.
 *
 * This function stores the provided calibration factor in non-volatile storage
 * and applies it immediately to the encoder.
 *
 * @param factor The new calibration factor to save.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t calibration_save(float factor);

/**
 * @brief Get the current calibration factor.
 *
 * This function retrieves the current calibration factor used by the encoder.
 *
 * @return float The current calibration factor.
 */
float calibration_get(void);

#ifdef __cplusplus
}
#endif
