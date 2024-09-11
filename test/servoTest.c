#include <stdio.h>
#include "PWMServoDriver.h"

#define NUM_CHANNELS 14 //(2~14,16)

int main() {
    // I2Cアドレスはデフォルトで0x40とする
    uint8_t i2c_addr = PCA9685_I2C_ADDRESS;
    
    // PWMServoDriverを作成
    PWMServoDriver* driver = PWMServoDriver_create(i2c_addr);
    
    // 初期化
    if (!PWMServoDriver_begin(driver, 0)) {
        printf("Failed to initialize PWM servo driver\n");
        return 1;
    }

    // 周波数を50Hzに設定 (サーボ用)
    PWMServoDriver_setPWMFreq(driver, 50.0);

    uint8_t channels[NUM_CHANNELS] = {2,3,4,5,6,7,8,9,10,11,12,13,14,16};

    // 各チャンネルにPWM出力を1500マイクロ秒（サーボの中立位置に相当）に設定
    for(int i = 0; i < NUM_CHANNELS; i++){
        PWMServoDriver_writeMicroseconds(driver, channels[i], 1500);
        printf("PWM set to 1500 microseconds on channel %d/n", channels[i]);
    }
    
    // 使用後は解放
    PWMServoDriver_destroy(driver);

    return 0;
}
