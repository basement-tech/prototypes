/*
 * servo_defs.c
 *
 * abstract the math interface to the physical servos
 */

#include <stdint.h>
#include "pca9685.h"
#include "servo_defs.h"

/*
 * define the servos
 */
servo_def_t servo_defs[PCA9685_MAX_CHANNELS] = {
   {0, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {1, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {2, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {3, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {4, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {5, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {6, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {7, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {8, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {9, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {10, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {11, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {12, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {13, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {14, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45},  // Futaba 3107
   {15, PCA9685_SERVO_MIN, PCA9685_SERVO_MID, PCA9685_SERVO_MAX, -45, 45}  // Futaba 3107
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
void servo_move_real(uint8_t channel, int32_t angle)  {
    map()
    uint16_t pulsew = ((servo_defs[channel].servo_max - servo_defs[channel].servo_min))
}

void servo_rest(uint8_t channel)  {

}

