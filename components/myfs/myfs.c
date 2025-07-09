#include <stdio.h>
#include <string.h>
#include "esp_spiffs.h"
#include "esp_log.h"
#include "myfs.h"
#include <dirent.h>  // for DIR, opendir, readdir, closedir

#define TAG "MYFS"
#define LOG_FILE "/spiffs/encoder_log.txt"

/**
 * @brief Lists all files stored in the SPIFFS partition.
 */
void list_spiffs_files(void) {
    DIR *dir = opendir("/spiffs");
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open /spiffs");
        return;
    }

    struct dirent *entry;
    ESP_LOGI(TAG, "Files in /spiffs:");
    while ((entry = readdir(dir)) != NULL) {
        ESP_LOGI(TAG, "  %s", entry->d_name);
    }
    closedir(dir);
}

/**
 * @brief Initializes the SPIFFS filesystem.
 * 
 * Mounts the filesystem or formats it if the mount fails.
 * Logs total and used space.
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t myfs_init(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format filesystem (%s)", esp_err_to_name(ret));
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS mounted. Total: %d, Used: %d", total, used);
    }

    return ESP_OK;
}

/**
 * @brief Logs encoder distance and speed to a file (placeholder).
 * 
 * Currently returns ESP_OK without writing. Future implementation may write to LOG_FILE.
 * 
 * @param distance Distance in meters
 * @param speed Speed in m/s
 * @return esp_err_t ESP_OK
 */
esp_err_t myfs_log_data(float distance, float speed) {
    return ESP_OK;
}
