#include "wifi_handler.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"

#define TAG "WIFI_HANDLER"
#define MAX_POST_SIZE 512

static esp_err_t save_wifi_credentials(const char *ssid, const char *password) {
    // Stores SSID and password into NVS under "wifi_config" namespace
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("wifi_config", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_str(nvs, "ssid", ssid);
    if (err == ESP_OK) err = nvs_set_str(nvs, "password", password);
    if (err == ESP_OK) err = nvs_commit(nvs);

    nvs_close(nvs);
    return err;
}

// Handles incoming JSON POST requests with Wi-Fi credentials and stores them in NVS
esp_err_t wifi_config_post_handler(httpd_req_t *req) {
    char content[MAX_POST_SIZE] = {0};
    int total_len = req->content_len;

    if (total_len >= MAX_POST_SIZE) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Too much data");
        return ESP_FAIL;
    }

    int received = httpd_req_recv(req, content, total_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read POST data");
        return ESP_FAIL;
    }

    cJSON *json = cJSON_Parse(content);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    const cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
    const cJSON *password = cJSON_GetObjectItem(json, "password");

    if (!cJSON_IsString(ssid) || !cJSON_IsString(password) ||
        ssid->valuestring == NULL || password->valuestring == NULL) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing SSID or password");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Parsed SSID: %s", ssid->valuestring);
    ESP_LOGI(TAG, "Parsed PASS: %s", password->valuestring);

    esp_err_t err = save_wifi_credentials(ssid->valuestring, password->valuestring);
    cJSON_Delete(json);

    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save Wi-Fi settings");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, "{\"status\":\"ok\",\"message\":\"Saved. Rebooting...\"}");

    ESP_LOGI(TAG, "Wi-Fi credentials saved, restarting...");
    vTaskDelay(pdMS_TO_TICKS(1000));  // wait a bit before restart
    esp_restart();

    return ESP_OK;  // this line will not be reached, esp_restart() does not return
}

// URI handler structure for Wi-Fi POST request
const httpd_uri_t uri_wifi_post = {
    .uri       = "/config",
    .method    = HTTP_POST,
    .handler   = wifi_config_post_handler,
    .user_ctx  = NULL
};
