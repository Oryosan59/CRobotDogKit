#ifndef PWMSERVODRIVER_H
#define PWMSERVODRIVER_H

#include <stdint.h>
#include <stdbool.h>

// レジスタアドレス
#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_SUBADR1 0x02
#define PCA9685_SUBADR2 0x03
#define PCA9685_SUBADR3 0x04
#define PCA9685_ALLCALLADR 0x05
#define PCA9685_LED0_ON_L 0x06
#define PCA9685_LED0_ON_H 0x07
#define PCA9685_LED0_OFF_L 0x08
#define PCA9685_LED0_OFF_H 0x09
#define PCA9685_ALLLED_ON_L 0xFA
#define PCA9685_ALLLED_ON_H 0xFB
#define PCA9685_ALLLED_OFF_L 0xFC
#define PCA9685_ALLLED_OFF_H 0xFD
#define PCA9685_PRESCALE 0xFE
#define PCA9685_TESTMODE 0xFF

// MODE1ビット
#define MODE1_ALLCAL 0x01
#define MODE1_SUB3 0x02
#define MODE1_SUB2 0x04
#define MODE1_SUB1 0x08
#define MODE1_SLEEP 0x10
#define MODE1_AI 0x20
#define MODE1_EXTCLK 0x40
#define MODE1_RESTART 0x80

// MODE2ビット
#define MODE2_OUTNE_0 0x01
#define MODE2_OUTNE_1 0x02
#define MODE2_OUTDRV 0x04
#define MODE2_OCH 0x08
#define MODE2_INVRT 0x10

#define PCA9685_I2C_ADDRESS 0x40
#define FREQUENCY_OSCILLATOR 25000000

#define PCA9685_PRESCALE_MIN 3
#define PCA9685_PRESCALE_MAX 255

// 構造体定義
typedef struct {
    uint8_t _i2caddr;
    uint32_t _oscillator_freq;
    void* i2c_dev; // I2Cデバイスへのポインタ
} PWMServoDriver;

// 関数プロトタイプ
PWMServoDriver* PWMServoDriver_create(uint8_t addr);
void PWMServoDriver_destroy(PWMServoDriver* driver);
bool PWMServoDriver_begin(PWMServoDriver* driver, uint8_t prescale);
void PWMServoDriver_reset(PWMServoDriver* driver);
void PWMServoDriver_sleep(PWMServoDriver* driver);
void PWMServoDriver_wakeup(PWMServoDriver* driver);
void PWMServoDriver_setExtClk(PWMServoDriver* driver, uint8_t prescale);
void PWMServoDriver_setPWMFreq(PWMServoDriver* driver, float freq);
void PWMServoDriver_setOutputMode(PWMServoDriver* driver, bool totempole);
uint16_t PWMServoDriver_getPWM(PWMServoDriver* driver, uint8_t num, bool off);
uint8_t PWMServoDriver_setPWM(PWMServoDriver* driver, uint8_t num, uint16_t on, uint16_t off);
void PWMServoDriver_setPin(PWMServoDriver* driver, uint8_t num, uint16_t val, bool invert);
uint8_t PWMServoDriver_readPrescale(PWMServoDriver* driver);
void PWMServoDriver_writeMicroseconds(PWMServoDriver* driver, uint8_t num, uint16_t Microseconds);
void PWMServoDriver_setOscillatorFrequency(PWMServoDriver* driver, uint32_t freq);
uint32_t PWMServoDriver_getOscillatorFrequency(PWMServoDriver* driver);
uint8_t PWMServoDriver_read8(PWMServoDriver* driver, uint8_t addr);
void PWMServoDriver_write8(PWMServoDriver* driver, uint8_t addr, uint8_t d);

#endif

