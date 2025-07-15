/*
 * a simple example to drive the PCA9685, i2c 16-channel PWM chip.
 *
 * this was based on an example from the internet.  Don't know whether
 * it was AI generated, but it didn't build and had some mistakes
 * and missing characters, which were corrected using the datasheet.
 * 
 * NOTE: Ported to new i2c driver ... see breadcrumbs throughout
 * 
 */

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_types.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "pca9685.h"

static const char *TAG = "PCA9685_SERVO_MAIN";

#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_SEL CHANNEL_1

void app_main() {
    
    ESP_LOGI(TAG, "Initializing PCA9685...");
    pca9685_init();

    while (1) {
        int pulse = 0;
#ifdef SWEEP
        // Sweep the servo from minimum to maximum
        for (pulse = SERVO_MIN; pulse <= SERVO_MAX; pulse++) {
            pca9685_set_pwm(CHANNEL_SEL, 0, pulse);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "pulse at %d", pulse);
        // Sweep the servo back from maximum to minimum
        for (pulse = SERVO_MAX; pulse >= SERVO_MIN; pulse--) {
            pca9685_set_pwm(CHANNEL_SEL, 0, pulse);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "pulse at %d", pulse);
#endif

#ifdef MAX_POS
        pulse = SERVO_MAX;
#endif
#ifdef MID_POS
//        pulse = (SERVO_MAX-SERVO_MIN)/2 + SERVO_MIN;
        pulse = SERVO_MID;
#endif
#ifdef MIN_POS
        pulse = SERVO_MIN;
#endif
#ifndef SWEEP
        pca9685_set_pwm(CHANNEL_SEL, 0, pulse);
        ESP_LOGI(TAG, "pulse at %d", pulse);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif
    }
}
