#include <stdio.h>
#include "PWMServoDriver.h"

#define PULSE_WIDTH_CENTER 1500

int main() 
{
    // I2Cアドレスはデフォルトで0x40とする
    uint8_t i2c_addr = PCA9685_I2C_ADDRESS;
    
    // PWMServoDriverを作成
    PWMServoDriver* driver = PWMServoDriver_create(i2c_addr);
    
    // 初期化
    if (!PWMServoDriver_begin(driver, 0)) 
    {
        printf("Failed to initialize PWM servo driver\n");
        return 1;
    }

    // 周波数を50Hzに設定 (サーボ用)
    PWMServoDriver_setPWMFreq(driver, 50.0);

    // 軸4,7,8,11を90度回転させる
    for(int i = 0; i < 300; i++)
    {
        PWMServoDriver_writeMicroseconds(driver, 4, PULSE_WIDTH_CENTER - i);
        PWMServoDriver_writeMicroseconds(driver, 7, PULSE_WIDTH_CENTER - i);
        PWMServoDriver_writeMicroseconds(driver, 8, PULSE_WIDTH_CENTER + i);
        PWMServoDriver_writeMicroseconds(driver, 11, PULSE_WIDTH_CENTER + i);
        usleep(10000); 
    }

    // 軸2,5,10,13を90度回転させる
    for(int i = 0; i < 300; i++)
    {
        PWMServoDriver_writeMicroseconds(driver, 2, PULSE_WIDTH_CENTER - i);
        PWMServoDriver_writeMicroseconds(driver, 5, PULSE_WIDTH_CENTER - i);
        PWMServoDriver_writeMicroseconds(driver, 10, PULSE_WIDTH_CENTER + i);
        PWMServoDriver_writeMicroseconds(driver, 13, PULSE_WIDTH_CENTER + i);
        usleep(10000); 
    }

    // 軸3,6,9,12を60度回転させる
    for(int i = 0; i < 200; i++)
    {
        PWMServoDriver_writeMicroseconds(driver, 3, PULSE_WIDTH_CENTER - i);
        PWMServoDriver_writeMicroseconds(driver, 6, PULSE_WIDTH_CENTER - i);
        PWMServoDriver_writeMicroseconds(driver, 9, PULSE_WIDTH_CENTER + i);
        PWMServoDriver_writeMicroseconds(driver, 12, PULSE_WIDTH_CENTER + i);
        usleep(10000); 
    }
    
    // 使用後は解放
    PWMServoDriver_destroy(driver);

    return 0;
}
