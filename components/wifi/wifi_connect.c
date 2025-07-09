#include <string.h>
#include "wifi_connect.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"
#include "display.h"
#include "webserver.h"


#define TAG "WIFI"
#define WIFI_CONNECT_TIMEOUT_MS 15000

#define WIFI_SSID      "SSID"           // <-- check your Wi-Fi SSID
#define WIFI_PASSWORD  "password"       // <-- check your Wi-Fi password
#define WIFI_CONNECTED_BIT BIT0

/**
 * @brief FreeRTOS task to connect to Wi-Fi and manage connection status.
 * 
 * Attempts to connect and waits for connection or timeout.
 * Displays status on the screen and starts webserver when connected.
 * Deletes itself once done.
 * 
 * @param pvParameters Task parameters (unused).
 */
void wifi_connect_task(void *pvParameters) {
    esp_err_t err = wifi_connect();  // your function to connect to Wi-Fi
    ESP_LOGI(TAG, "wifi_connect() returned %d", err);

    uint32_t start_tick = xTaskGetTickCount();
    while (!wifi_is_connected() && (xTaskGetTickCount() - start_tick) < pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS)) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (!wifi_is_connected()) {
        ESP_LOGW(TAG, "Wi-Fi timeout — offline");
        display_show_message("Wi-Fi offline");
    } else {
        ESP_LOGI(TAG, "Wi-Fi connected");
        display_show_ip();  
        start_webserver();
    }

    vTaskDelete(NULL);
}
static EventGroupHandle_t wifi_event_group; ///< Event group to signal Wi-Fi connection status
static volatile bool s_wifi_connected = false; ///< Flag to indicate Wi-Fi connection status

/**
 * @brief Event handler for Wi-Fi and IP events.
 * 
 * Handles events for starting connection, disconnection, and IP acquisition.
 * Retries connection on disconnect and sets flag and event on IP acquisition.
 * 
 * @param arg User context (unused)
 * @param event_base Event base identifier
 * @param event_id Event identifier
 * @param event_data Pointer to event data
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Disconnected, retrying...");
        s_wifi_connected = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Got IP!");
        s_wifi_connected = true;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief Initializes and starts Wi-Fi connection.
 * 
 * Sets up Wi-Fi driver, configures STA mode with SSID and password,
 * and waits for connection event or timeout.
 * 
 * @return ESP_OK on success, ESP_FAIL on failure or timeout.
 */
esp_err_t wifi_connect(void) {
    ESP_LOGI(TAG, "Connecting to SSID: %s", WIFI_SSID);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = { 0 };
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Wi-Fi connected successfully");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "Wi-Fi connection timeout or failed");
        esp_wifi_stop();
        return ESP_FAIL;
    }
}

/**
 * @brief Returns current Wi-Fi connection status.
 * 
 * @return true if connected, false otherwise.
 */
bool wifi_is_connected(void) {
    return s_wifi_connected;
}
