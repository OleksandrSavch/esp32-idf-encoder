#include "encoder.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "ENCODER"
#define PCNT_HIGH_LIMIT  32767
#define PCNT_LOW_LIMIT  -32768
#define SPEED_TASK_PERIOD_MS 1000   // Speed update period in milliseconds

// Internal pulse counter and parameters
static int total_pulse_count = 0;
static pcnt_unit_handle_t pcnt_unit = NULL;
static int pulses_per_rev = 600;
static float wheel_diameter_m = 0.1f;         
static float calibration_factor = 1.0f;       
static float distance_per_pulse = 0.0f;       

/// Variables used for speed calculation

static int last_pulse_count = 0;
static float last_speed = 0.0;
static int64_t last_time_us = 0;

/**
 * @brief Pulse counter event callback for high/low limit overflow handling.
 */
static bool IRAM_ATTR pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx) {
    if (edata->watch_point_value == PCNT_HIGH_LIMIT) {
        total_pulse_count += PCNT_HIGH_LIMIT;
    } else if (edata->watch_point_value == PCNT_LOW_LIMIT) {
        total_pulse_count += PCNT_LOW_LIMIT;
    }
    return true;
}

/**
 * @brief Updates the distance per pulse value based on wheel diameter and pulses per revolution.
 */
static void update_distance_per_pulse(void) {
    distance_per_pulse = (float)M_PI * wheel_diameter_m / (float)pulses_per_rev;
}

/**
 * @brief Initializes the encoder using pulse counter with specified GPIOs.
 */
void encoder_init(gpio_num_t pin_a, gpio_num_t pin_b, int ppr, float wheel_diameter_mm) {
    pulses_per_rev = ppr;
    wheel_diameter_m = wheel_diameter_mm / 1000.0f;
    update_distance_per_pulse();  // <- important to call this after setting wheel diameter

    pcnt_unit_config_t unit_config = {
        .high_limit = PCNT_HIGH_LIMIT,
        .low_limit = PCNT_LOW_LIMIT,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    // Channel A configuration
    pcnt_chan_config_t chan_a = {
        .edge_gpio_num = pin_a,
        .level_gpio_num = pin_b,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a, &pcnt_chan_a));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    // Channel B configuration
    pcnt_chan_config_t chan_b = {
        .edge_gpio_num = pin_b,
        .level_gpio_num = pin_a,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b, &pcnt_chan_b));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_INVERSE, PCNT_CHANNEL_LEVEL_ACTION_KEEP));

    // Add overflow watchpoints
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, PCNT_HIGH_LIMIT));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, PCNT_LOW_LIMIT));

    // Register callback
    pcnt_event_callbacks_t cbs = {
        .on_reach = pcnt_on_reach,
    };
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, NULL));

    // Enable and start the unit
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    ESP_LOGI(TAG, "Encoder initialized");
}

/**
 * @brief Returns the current total pulse count including overflow.
 */
int encoder_get_pulses(void) {
    int count = 0;
    if (pcnt_unit) {
        pcnt_unit_get_count(pcnt_unit, &count);
        return total_pulse_count + count;
    }
    return 0;
}

/**
 * @brief Resets pulse count and speed state.
 */
void encoder_reset(void) {
    if (pcnt_unit) {
        pcnt_unit_clear_count(pcnt_unit);
        total_pulse_count = 0;
        last_pulse_count = 0;
        last_speed = 0.0f;
    }
}

/**
 * @brief Returns the calculated distance in meters.
 */
float encoder_get_distance_m(void) {
    int pulses = encoder_get_pulses();
    return pulses * distance_per_pulse * calibration_factor;
}

/**
 * @brief Sets new wheel diameter in millimeters.
 */
void encoder_set_wheel_diameter_mm(float diameter_mm) {
    if (diameter_mm > 0.0f) {
        wheel_diameter_m = diameter_mm / 1000.0f;
        update_distance_per_pulse();
    }
}

/**
 * @brief Sets calibration factor for distance correction.
 */
void encoder_set_calibration_factor(float factor) {
    if (factor > 0.0f) {
        calibration_factor = factor;
    }
}

/**
 * @brief Returns current wheel diameter in millimeters.
 */
float encoder_get_wheel_diameter_mm(void) {
    return wheel_diameter_m * 1000.0f;
}

/**
 * @brief Returns current calibration factor.
 */
float encoder_get_calibration_factor(void) {
    return calibration_factor;
}

/**
 * @brief Updates speed calculation based on encoder pulses over time.
 */
void encoder_update_speed(void) {
    int current_pulses = encoder_get_pulses();
    int delta_pulses = current_pulses - last_pulse_count;

    int64_t now_us = esp_timer_get_time();
    float interval_s = (last_time_us == 0)
        ? 1.0f
        : (now_us - last_time_us) / 1000000.0f;

    float distance = delta_pulses * distance_per_pulse * calibration_factor;
    last_speed = distance / interval_s;

    last_pulse_count = current_pulses;
    last_time_us = now_us;
}

/**
 * @brief Returns last calculated speed in meters per second.
 */
float encoder_get_speed_mps(void) {
    return last_speed;
}

/**
 * @brief Background FreeRTOS task to periodically update speed.
 */
static void encoder_speed_task(void* arg) {
    while (1) {
        encoder_update_speed();
        vTaskDelay(pdMS_TO_TICKS(SPEED_TASK_PERIOD_MS));
    }
}

/**
 * @brief Starts background task for speed measurement.
 */
void encoder_start_speed_task(void) {
    xTaskCreatePinnedToCore(encoder_speed_task, "encoder_speed_task", 2048, NULL, 5, NULL, 0);
}
