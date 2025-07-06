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

// PCA9685 I2C address and parameters
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 23
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_FREQ_HZ 100000

/*
 * futaba S3107 Servo
 */
#define SERVO_FREQ 46.5   // operating frequency of the servos yields exactly 20 mS period
#define SERVO_MIN 205     // yields 1.0 mS 
#define SERVO_MID 307     // yields 1.5 mS
#define SERVO_MAX 410     // yields 2.0 mS
#define CHANNEL_0 0

static const char *TAG = "PCA9685_SERVO";

/*
i2c_master_bus_config_t i2c_mst_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_MASTER_NUM,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .glitch_ignore_cnt = 7,
};
*/
i2c_master_bus_handle_t bus_handle;

i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = PCA9685_I2C_ADDRESS,
    .scl_speed_hz = I2C_MASTER_FREQ_HZ,
};
i2c_master_dev_handle_t dev_handle;

i2c_master_bus_config_t i2c_mst_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_MASTER_NUM,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .flags.enable_internal_pullup = true,
    .glitch_ignore_cnt = 7,
};

void i2c_master_init(void)  {
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));  // set the config in bus_handle structure
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));  // configure the device characteristics
}

#ifdef OG
void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}
#endif

void pca9685_write_byte(uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS));
    //i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_I2C_ADDRESS, write_buf, 2, 1000 / portTICK_PERIOD_MS);
}

/*
 * each pwm channel has four 8-bit registers in address order.
 * channel 0 starts at the address 0x06, hence the math below
 * for register addresses
 * 
 * From the datasheet:
 * The turn-on time of each LED driver output and the duty cycle of PWM can be controlled
 * independently using the LEDn_ON and LEDn_OFF registers.
 * There will be two 12-bit registers per LED output. These registers will be programmed by
 * the user. Both registers will hold a value from 0 to 4095. One 12-bit register will hold a
 * value for the ON time and the other 12-bit register will hold the value for the OFF time. The
 * ON and OFF times are compared with the value of a 12-bit counter that will be running
 * continuously from 0000h to 0FFFh (0 to 4095 decimal).
 * Update on ACK requires all 4 PWM channel registers to be loaded before outputs will
 * change on the last ACK.
 * The ON time, which is programmable, will be the time the LED output will be asserted and
 * the OFF time, which is also programmable, will be the time when the LED output will be
 * negated. In this way, the phase shift becomes completely programmable. The resolution
 * for the phase shift is 1⁄4096 of the target frequency. Table 7 (datasheet) lists these registers.
 */
void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off) {
    uint8_t write_buf[5] = {
        0x06 + 4 * channel,  // LEDn_ON_L register address
        on & 0xFF,           // Low byte of ON value
        on >> 8,             // High byte of ON value
        off & 0xFF,          // Low byte of OFF value
        off >> 8             // High byte of OFF value
    };
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS));
    //i2c_master_write_to_device(I2C_MASTER_NUM, PCA9685_I2C_ADDRESS, write_buf, 5, 1000 / portTICK_PERIOD_MS);
}

void pca9685_init() {
    pca9685_write_byte(PCA9685_MODE1, MODE1_SLEEP);  // Put PCA9685 into sleep mode
    uint8_t prescale = (uint8_t)(round((float)FREQUENCY_OSCILLATOR / (PCA9685_BIT_RESOLUTION * SERVO_FREQ))) - 1;  // Set PWM frequency to 50 Hz
    ESP_LOGI(TAG, "prescale = %u", prescale);
    pca9685_write_byte(PCA9685_PRESCALE, prescale);
    pca9685_write_byte(PCA9685_MODE1, MODE1_RSTN);  // Restart and set to normal mode
}

//#define MIN_POS
//#define MID_POS
//#define MAX_POS
#define SWEEP

void app_main() {
    ESP_LOGI(TAG, "Initializing I2C...");
    i2c_master_init();
    
    ESP_LOGI(TAG, "Initializing PCA9685...");
    pca9685_init();

    while (1) {
        int pulse = 0;
#ifdef SWEEP
        // Sweep the servo from minimum to maximum
        for (pulse = SERVO_MIN; pulse <= SERVO_MAX; pulse++) {
            pca9685_set_pwm(CHANNEL_0, 0, pulse);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "pulse at %d", pulse);
        // Sweep the servo back from maximum to minimum
        for (pulse = SERVO_MAX; pulse >= SERVO_MIN; pulse--) {
            pca9685_set_pwm(CHANNEL_0, 0, pulse);
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
        pca9685_set_pwm(CHANNEL_0, 0, pulse);
        ESP_LOGI(TAG, "pulse at %d", pulse);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif
    }
}
