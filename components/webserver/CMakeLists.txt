idf_component_register(
    SRCS "webserver.c" "wifi_handler.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_server spiffs myfs encoder nvs_flash wifi calibration settings
)
