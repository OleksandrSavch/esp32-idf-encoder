#include "settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char* TAG = "SETTINGS";
static const char* NVS_NAMESPACE = "storage";
static const char* KEY_DIAMETER = "diameter";
static const char* KEY_FACTOR   = "factor";

static const float DEFAULT_DIAMETER = 100.0f;
static const float DEFAULT_FACTOR = 1.0f;

static bool nvs_initialized = false;
// Cache for loaded settings to avoid unnecessary writes
static float cached_diameter = -1.0f;
static float cached_factor = -1.0f;
static bool cache_loaded = false;

// Ensures NVS is initialized before reading or writing
static esp_err_t ensure_nvs_ready(void) {
    if (!nvs_initialized) {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_LOGW(TAG, "NVS init requires erase");
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        if (err == ESP_OK) {
            nvs_initialized = true;
        }
        return err;
    }
    return ESP_OK;
}

// Loads settings from NVS or applies defaults if not found
esp_err_t settings_load(float* out_diameter, float* out_factor) {
    if (!out_diameter || !out_factor) return ESP_ERR_INVALID_ARG;

    esp_err_t err = ensure_nvs_ready();
    if (err != ESP_OK) return err;

    nvs_handle_t handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS open failed, using defaults");
        *out_diameter = DEFAULT_DIAMETER;
        *out_factor = DEFAULT_FACTOR;
    } else {
        size_t size = sizeof(float);

        err = nvs_get_blob(handle, KEY_DIAMETER, out_diameter, &size);
        if (err != ESP_OK) {
            *out_diameter = DEFAULT_DIAMETER;
            ESP_LOGW(TAG, "Diameter not found, default %.2f", *out_diameter);
        }

        size = sizeof(float);
        err = nvs_get_blob(handle, KEY_FACTOR, out_factor, &size);
        if (err != ESP_OK) {
            *out_factor = DEFAULT_FACTOR;
            ESP_LOGW(TAG, "Factor not found, default %.3f", *out_factor);
        }

        nvs_close(handle);
    }

    cached_diameter = *out_diameter;
    cached_factor = *out_factor;
    cache_loaded = true;

    ESP_LOGI(TAG, "Loaded settings: diameter=%.2f, factor=%.3f", *out_diameter, *out_factor);

    return ESP_OK;
}

// Saves settings to NVS only if values changed (to reduce flash wear)
esp_err_t settings_save(float diameter, float factor) {
    esp_err_t err = ensure_nvs_ready();
    if (err != ESP_OK) return err;

    if (cache_loaded && diameter == cached_diameter && factor == cached_factor) {
        ESP_LOGI(TAG, "No change in settings, skip save");
        return ESP_OK;
    }

    nvs_handle_t handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed");
        return err;
    }

    err = nvs_set_blob(handle, KEY_DIAMETER, &diameter, sizeof(float));
    if (err != ESP_OK) {
        nvs_close(handle);
        ESP_LOGE(TAG, "Save diameter failed");
        return err;
    }

    err = nvs_set_blob(handle, KEY_FACTOR, &factor, sizeof(float));
    if (err != ESP_OK) {
        nvs_close(handle);
        ESP_LOGE(TAG, "Save factor failed");
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);

    if (err == ESP_OK) {
        cached_diameter = diameter;
        cached_factor = factor;
        cache_loaded = true;

        ESP_LOGI(TAG, "Settings saved: diameter=%.2f mm, factor=%.3f", diameter, factor);
    } else {
        ESP_LOGE(TAG, "Commit failed");
    }

    return err;
}
