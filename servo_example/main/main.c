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
#include "servo_defs.h"

#define TAG "PCA9685_SERVO_MAIN"

#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_SEL CHANNEL_1



void app_main(void) {
   ESP_LOGI(TAG, "Initializing servo subsystem...");
   if(servo_init() != ESP_OK)
        ESP_LOGE(TAG, "Error initializing servos");

   int32_t angle = 0;
   uint8_t ch = 0;

   while(1)  {
      servo_move_real_pre(ch, servo_defs[ch].mina, false);  // absolute move to ccw
      servo_rest(ch);  // back to middle
      ESP_LOGI(TAG, "rest move resulted in %ld deg", servo_get_angle(ch));

      /*
       * make 45 1 deg relative moves
       */
      for(int i = 0; i < 45; i++)  {
        servo_move_real_pre(ch, 1, true);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      servo_move_real_pre(ch, servo_defs[ch].maxa, false);  // correct for stack up
      servo_rest(ch);  // back to middle
      ESP_LOGI(TAG, "rest move resulted in %ld deg", servo_get_angle(ch));

      for(int i = 0; i < 45; i++)  {
        servo_move_real_pre(ch, -1, true);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      servo_rest(ch);  // back to middle
      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }

#ifdef SWEEP_USING_ABS
   while(1)  {
      for(angle = servo_defs[ch].mina; angle <= servo_defs[ch].maxa; angle+=1)  {
        servo_move_real_pre(ch, angle, false);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      for(angle = servo_defs[ch].maxa; angle >= servo_defs[ch].mina; angle-=1)  {
        servo_move_real_pre(ch, angle, false);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
#endif

#ifdef SWEEP_USING_REL
   /*
    * relative move test
    */
   while(1)  {
      servo_move_real_pre(ch, servo_defs[ch].mina, false);  // absolute move to ccw
      /*
       * make 90 1 deg relative moves
       */
      for(int i = 0; i < 90; i++)  {
        servo_move_real_pre(ch, 1, true);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      servo_move_real_pre(ch, servo_defs[ch].maxa, false);  // correct for stack up
      for(int i = 0; i < 90; i++)  {
        servo_move_real_pre(ch, -1, true);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
#endif

#ifdef OBSCURE_OG
    ESP_LOGI(TAG, "Initializing PCA9685...");
    pca9685_init();

    while (1) {
        int pulse = 0;
#ifdef SWEEP
        // Sweep the servo from minimum to maximum
        for (pulse = PCA9685_SERVO_MIN; pulse <= PCA9685_SERVO_MAX; pulse++) {
            pca9685_set_pwm(CHANNEL_SEL, 0, pulse);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "pulse at %d", pulse);
        // Sweep the servo back from maximum to minimum
        for (pulse = PCA9685_SERVO_MAX; pulse >= PCA9685_SERVO_MIN; pulse--) {
            pca9685_set_pwm(CHANNEL_SEL, 0, pulse);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "pulse at %d", pulse);
#endif  //SWEEP

#ifdef MAX_POS
        pulse = PCA9685_SERVO_MAX;
#endif
#ifdef MID_POS
//        pulse = (SERVO_MAX-SERVO_MIN)/2 + SERVO_MIN;
        pulse = PCA9685_SERVO_MID;
#endif
#ifdef MIN_POS
        pulse = PCA9685_SERVO_MIN;
#endif
#ifndef SWEEP
        pca9685_set_pwm(CHANNEL_SEL, 0, pulse);
        ESP_LOGI(TAG, "pulse at %d", pulse);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif
    }

#endif  // OBSCURE_OG

}
