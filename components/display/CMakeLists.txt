idf_component_register(SRCS "display.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp32-smbus esp32-i2c-lcd1602 driver esp_netif 
                       )
