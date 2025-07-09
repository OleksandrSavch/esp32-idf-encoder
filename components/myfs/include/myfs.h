#pragma once

#include "esp_err.h"  // Required for esp_err_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the SPIFFS file system
 */
esp_err_t myfs_init(void);

/**
 * @brief Lists files in the SPIFFS file system
 */
void list_spiffs_files(void);

#ifdef __cplusplus
}
#endif
