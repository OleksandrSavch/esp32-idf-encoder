#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "esp_http_server.h"

/**
 * @brief Starts the HTTP web server.
 *
 * Initializes and configures the web server, registers default handlers,
 * and starts listening for incoming HTTP requests.
 *
 * @return ESP_OK on success, or an appropriate error code.
 */
esp_err_t start_webserver(void);

/**
 * @brief Registers custom API handlers to the HTTP server.
 *
 * Use this function to attach additional URI handlers for REST or other endpoints.
 *
 * @param server The handle of the running HTTP server instance.
 */
void register_api_handlers(httpd_handle_t server);

#ifdef __cplusplus
}
#endif
