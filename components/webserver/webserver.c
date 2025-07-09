#include "webserver.h"
#include "wifi_handler.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "settings.h"
#include "encoder.h"
#include "calibration.h"

#define TAG "WEBSERVER"
#define FILE_PATH_MAX 520

static esp_err_t serve_file_handler(httpd_req_t *req) {
    // Serves static files from SPIFFS (HTML, CSS, JS, etc.)
    char filepath[FILE_PATH_MAX];

    if (strcmp(req->uri, "/") == 0) {
        // Redirect root to /index.html
        httpd_resp_set_status(req, "302 Found");
        httpd_resp_set_hdr(req, "Location", "/index.html");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }

    snprintf(filepath, sizeof(filepath), "/spiffs%s", req->uri);
    FILE *file = fopen(filepath, "r");
    if (!file) {
        // File not found in SPIFFS
        ESP_LOGW(TAG, "File not found: %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    if (strstr(req->uri, ".html"))       httpd_resp_set_type(req, "text/html");
    else if (strstr(req->uri, ".css"))   httpd_resp_set_type(req, "text/css");
    else if (strstr(req->uri, ".js"))    httpd_resp_set_type(req, "application/javascript");
    else if (strstr(req->uri, ".json"))  httpd_resp_set_type(req, "application/json");
    else                                 httpd_resp_set_type(req, "text/plain");

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        httpd_resp_sendstr_chunk(req, line);
    }
    httpd_resp_sendstr_chunk(req, NULL);
    fclose(file);
    return ESP_OK;
}


static esp_err_t data_get_handler(httpd_req_t *req) {
    // Sends current speed and distance as a JSON response
    float distance = encoder_get_distance_m();
    float speed = encoder_get_speed_mps();

    char json_response[128];
    snprintf(json_response, sizeof(json_response),
             "{\"distance\": %.2f, \"speed\": %.2f}", distance, speed);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t reset_post_handler(httpd_req_t *req) {
    // Resets encoder counter on POST request
    encoder_reset();
    httpd_resp_sendstr(req, "Reset done");
    return ESP_OK;
}

static esp_err_t set_calib_handler(httpd_req_t *req) {
    // Handles calibration value submission (form-urlencoded)
    char buf[32] = {0};
    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (len <= 0) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No data");
    }
    buf[len] = 0;

    char *val_ptr = strstr(buf, "value=");
    if (!val_ptr) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Value not found");
    }
    float val = atof(val_ptr + 6);
    if (val <= 0.0f) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Value must be > 0");
    }

    esp_err_t err = calibration_save(val);
    if (err != ESP_OK) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Save failed");
    }

    httpd_resp_sendstr(req, "Calibration updated");
    return ESP_OK;
}

static esp_err_t api_get_settings_handler(httpd_req_t *req) {
    // Responds with stored settings (diameter, factor) in JSON format
    float diameter = 0, factor = 0;
    esp_err_t err = settings_load(&diameter, &factor);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load settings");
        return ESP_FAIL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "diameter", diameter);
    cJSON_AddNumberToObject(root, "factor", factor);

    const char *resp_str = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, resp_str);

    cJSON_Delete(root);
    free((void *)resp_str);

    return ESP_OK;
}

static esp_err_t api_post_settings_handler(httpd_req_t *req) {
    // Accepts and saves settings sent as JSON, applies them to encoder
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request");
        return ESP_FAIL;
    }
    buf[ret] = 0;

    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    float diameter, factor;
    esp_err_t err = settings_load(&diameter, &factor); // load current settings
    if (err != ESP_OK) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Load failed");
        return ESP_FAIL;
    }

    cJSON *diameter_json = cJSON_GetObjectItem(json, "diameter");
    if (cJSON_IsNumber(diameter_json)) {
        diameter = diameter_json->valuedouble;
    }

    cJSON *factor_json = cJSON_GetObjectItem(json, "factor");
    if (cJSON_IsNumber(factor_json)) {
        factor = factor_json->valuedouble;
    }

    ESP_LOGI(TAG, "Received updated settings: diameter=%.2f, factor=%.3f", diameter, factor);

    err = settings_save(diameter, factor);
    if (err != ESP_OK) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Save failed");
        return ESP_FAIL;
    }

    encoder_set_wheel_diameter_mm(diameter);
    encoder_set_calibration_factor(factor);

    cJSON_Delete(json);

    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t uri_root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = serve_file_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_index = {
    .uri       = "/index.html",
    .method    = HTTP_GET,
    .handler   = serve_file_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_config = {
    .uri       = "/config.html",
    .method    = HTTP_GET,
    .handler   = serve_file_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_settings = {
    .uri       = "/settings.html",
    .method    = HTTP_GET,
    .handler   = serve_file_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_data = {
    .uri       = "/data",
    .method    = HTTP_GET,
    .handler   = data_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_reset = {
    .uri       = "/reset",
    .method    = HTTP_POST,
    .handler   = reset_post_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_set_calib = {
    .uri       = "/set_calib",
    .method    = HTTP_POST,
    .handler   = set_calib_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_api_get_settings = {
    .uri       = "/api/settings",
    .method    = HTTP_GET,
    .handler   = api_get_settings_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t uri_api_post_settings = {
    .uri       = "/api/settings",
    .method    = HTTP_POST,
    .handler   = api_post_settings_handler,
    .user_ctx  = NULL
};

esp_err_t handle_favicon(httpd_req_t *req) {
    // Responds with 404 for /favicon.ico requests to reduce noise
    httpd_resp_send_404(req);  
    return ESP_OK;
}

static const httpd_uri_t favicon = {
    .uri      = "/favicon.ico",
    .method   = HTTP_GET,
    .handler  = handle_favicon,  
    .user_ctx = NULL
};

esp_err_t start_webserver(void) {
    // Starts HTTP server and registers URI handlers
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;  // or any other suitable number

    esp_err_t err = httpd_start(&server, &config);
    if (err == ESP_OK) {
        httpd_register_uri_handler(server, &uri_root);
        httpd_register_uri_handler(server, &uri_index);
        httpd_register_uri_handler(server, &uri_config);
        httpd_register_uri_handler(server, &uri_settings);
        httpd_register_uri_handler(server, &uri_data);
        httpd_register_uri_handler(server, &uri_reset);
        httpd_register_uri_handler(server, &uri_set_calib);
        httpd_register_uri_handler(server, &uri_api_get_settings);
        httpd_register_uri_handler(server, &uri_api_post_settings);
        httpd_register_uri_handler(server, &favicon);

        extern const httpd_uri_t uri_wifi_post;
        httpd_register_uri_handler(server, &uri_wifi_post);
        
        ESP_LOGI(TAG, "Webserver started");

    } else {
        ESP_LOGE(TAG, "Failed to start webserver");
    }

    return err;
}
