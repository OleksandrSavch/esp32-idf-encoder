#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_config_post_handler(httpd_req_t *req);
/**
 * @brief POST request handler to save Wi-Fi settings into NVS.
 */
extern const httpd_uri_t uri_wifi_post;

#ifdef __cplusplus
}
#endif
