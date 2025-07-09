#include "calibration.h"
#include "nvs_flash.h"
#include "nvs.h"

static float Calibration_factor = 1.0f;  // Default calibration factor

/**
 * @brief Load calibration factor from NVS.
 *
 * Reads the saved calibration factor from NVS storage.
 * If no value is stored, the default factor (1.0) remains.
 */
void calibration_load() {
    nvs_handle_t nvs;
    if (nvs_open("storage", NVS_READONLY, &nvs) == ESP_OK) {
        size_t size = sizeof(Calibration_factor);
        nvs_get_blob(nvs, "calib_factor", &Calibration_factor, &size);
        nvs_close(nvs);
    }
}

/**
 * @brief Save calibration factor to NVS.
 *
 * Saves the provided calibration factor to non-volatile storage.
 * Returns an error if the value is invalid (<= 0).
 *
 * @param value The calibration factor to save.
 * @return esp_err_t ESP_OK on success, otherwise error code.
 */
esp_err_t calibration_save(float value) {
    if (value <= 0.0f) return ESP_ERR_INVALID_ARG;

    Calibration_factor = value;

    nvs_handle_t nvs;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(nvs, "calib_factor", &Calibration_factor, sizeof(Calibration_factor));
    if (err == ESP_OK) nvs_commit(nvs);
    nvs_close(nvs);
    return err;
}

/**
 * @brief Get current calibration factor.
 *
 * @return float Current calibration factor.
 */
float calibration_get() {
    return Calibration_factor;
}
