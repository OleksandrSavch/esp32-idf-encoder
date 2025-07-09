#include "display.h"
#include "i2c-lcd1602.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "smbus.h"
#include "esp_netif.h"

#define TAG "DISPLAY"

#define I2C_NUM     I2C_NUM_0
#define SDA_GPIO    GPIO_NUM_21
#define SCL_GPIO    GPIO_NUM_22
#define LCD_ADDR    0x27
#define LCD_COLS    16
#define LCD_ROWS    2

static i2c_lcd1602_info_t *lcd;
static smbus_info_t *smbus;

// Initialize the LCD display via I2C and set up the required SMBus interface.
esp_err_t display_init(void) {
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_GPIO,
        .scl_io_num = SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM, &cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM, cfg.mode, 0, 0, 0));

    smbus = smbus_malloc();
    ESP_ERROR_CHECK(smbus_init(smbus, I2C_NUM, LCD_ADDR));

    lcd = i2c_lcd1602_malloc();
    ESP_ERROR_CHECK(i2c_lcd1602_init(lcd, smbus, true, LCD_ROWS, LCD_COLS, LCD_COLS));
    i2c_lcd1602_clear(lcd);
    i2c_lcd1602_set_cursor(lcd, false); // Hide blinking cursor

    return ESP_OK;
}

// Update the first line of the display with the current distance in meters.
void display_update_distance(float meters) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Dist: %.2f m", meters);
    i2c_lcd1602_move_cursor(lcd, 0, 0);
    i2c_lcd1602_write_string(lcd, buf);
}

// Update the second line of the display with the current speed in m/s.
void display_update_speed(float mps) {
    char buf[17];
    snprintf(buf, sizeof(buf), "Speed: %.2f", mps);
    i2c_lcd1602_move_cursor(lcd, 0, 1);
    i2c_lcd1602_write_string(lcd, buf);
}
static float prev_speed = -1.0f;
static float prev_dist = -1.0f;

// Show a custom message, clearing both lines of the LCD.
void display_show_message(const char* msg) {
    i2c_lcd1602_clear(lcd);
    i2c_lcd1602_move_cursor(lcd, 0, 0);
    i2c_lcd1602_write_string(lcd, msg);
}

// Update both lines with speed and distance only if values have changed.
void display_show_status(float speed, float distance) {
    if (speed == prev_speed && distance == prev_dist)
        return;     // Skip redundant updates

    prev_speed = speed;
    prev_dist = distance;

    char line1[17], line2[17];
    snprintf(line1, sizeof(line1), "Dist:   %7.2fm", distance);
    snprintf(line2, sizeof(line2), "Speed:  %5.2fm/s", speed);

    i2c_lcd1602_move_cursor(lcd, 0, 0);
    i2c_lcd1602_write_string(lcd, line1);

    i2c_lcd1602_move_cursor(lcd, 0, 1);
    i2c_lcd1602_write_string(lcd, line2);
}

// Display the assigned IP address after successful Wi-Fi connection.
void display_show_ip(void) {
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char line1[17];
        char line2[17];
        snprintf(line1, sizeof(line1), "Wi-Fi connected");
        snprintf(line2, sizeof(line2), IPSTR, IP2STR(&ip_info.ip));

        i2c_lcd1602_clear(lcd);
        i2c_lcd1602_move_cursor(lcd, 0, 0);
        i2c_lcd1602_write_string(lcd, line1);
        i2c_lcd1602_move_cursor(lcd, 0, 1);
        i2c_lcd1602_write_string(lcd, line2);
    } else {
        display_show_message("No IP address");
    }
}
