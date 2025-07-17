/*
 * servo_defs.c
 *
 * abstract the math interface to the physical servos
 */

#include <stdint.h>
#include <math.h>
#include "esp_log.h"
#include "pca9685.h"
#include "servo_defs.h"

#define TAG "servo"

/*
 * define the servos
 */
servo_def_t servo_defs[PCA9685_MAX_CHANNELS] = {
   {0, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {1, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {2, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {3, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {4, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {5, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {6, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {7, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {8, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {9, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {10, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {11, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {12, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {13, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {14, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0},  // Futaba 3107
   {15, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, FUTABA_3107_MINA, FUTABA_3107_MAXA, 0}  // Futaba 3107
};

/*
 * map the real world coordinates to the pulse width calibration values
 * and move the servo to the requested real world angle
 * 
 * - pulse width numbers are always positive
 * - real world coordinates can be negative or positive
 * - all math is integer
 * 
 */
float calc_map_span(uint16_t imin, uint16_t imax, int32_t rmin, int32_t rmax)  {
    uint16_t ispan = imax - imin;  // the span of the "to" range
    int32_t  rspan = abs(rmax - rmin);  // the span of the "from" range
    return(ispan/(float)rspan);
}

uint16_t int_map(int32_t in, uint16_t imin, uint16_t imax, int32_t rmin, int32_t rmax)  {

    uint16_t out = imin + abs(in) * calc_map_span(imin, imax, rmin, rmax);

    return(out);
}

void servo_precalc(void)  {
    for(int i = 0; i < PCA9685_MAX_CHANNELS; i++)  {
        servo_defs[i].pre_incr = calc_map_span(
                                            servo_defs[i].servo_min, servo_defs[i].servo_max,
                                            servo_defs[i].mina, servo_defs[i].maxa
                                            );
        ESP_LOGI(TAG, "pre_incr = %f", servo_defs[i].pre_incr);
    }
}

void servo_init(void)  {
    pca9685_init();
    servo_precalc();
}

/*
 * make a move on the specified channel to the real world angle
 * servo_move_real_pre()  uses the precalculated value from servo_defs[].
 * truncate the value if exceeds limits.
 * return the actual value set.
 * 
 *   angle       abs(angle-servo_defs[channel].mina)                 pulse
 *   -----       -----------------------------------                 -----
 *   -45                abs(-45 -(-45)) = 0                      servo_min = 205
 *    0                 abs(0 -(-45))   = 45           servo_min + 45 * pre_incr(2.278) = 308  (aka servo_mid)
 *   +45                abs(45 -(-45))  = 90           servo_min + 90 * pre_incr(2.278) = 410
 */
uint16_t servo_move_real_pre(uint8_t channel, int32_t angle)  {

    uint16_t pulse = servo_defs[channel].servo_min + abs(angle-servo_defs[channel].mina) * servo_defs[channel].pre_incr;
    if(pulse > servo_defs[channel].servo_max) pulse = servo_defs[channel].servo_max;
    else if(pulse < servo_defs[channel].servo_min) pulse = servo_defs[channel].servo_min;
    pca9685_set_pwm(channel, 0, pulse);
    return(pulse);
}

void servo_rest(uint8_t channel)  {

}

