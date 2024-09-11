#include "PWMServoDriver.h"
#include <stdlib.h>
#include <unistd.h> // for sleep

// ダミー関数
void* I2CDevice_create(uint8_t addr, void* i2c);
void I2CDevice_destroy(void* device);
bool I2CDevice_begin(void* device);
bool I2CDevice_write_then_read(void* device, uint8_t* write_buf, size_t write_len, uint8_t* read_buf, size_t read_len);
bool I2CDevice_write(void* device, uint8_t* buffer, size_t len);

PWMServoDriver* PWMServoDriver_create(uint8_t addr) {
    PWMServoDriver* driver = (PWMServoDriver*)malloc(sizeof(PWMServoDriver));
    driver->_i2caddr = addr;
    driver->_oscillator_freq = FREQUENCY_OSCILLATOR;
    driver->i2c_dev = I2CDevice_create(addr, NULL); // NULLは実際のI2Cオブジェクトに置き換える
    return driver;
}

void PWMServoDriver_destroy(PWMServoDriver* driver) {
    if (driver->i2c_dev) {
        I2CDevice_destroy(driver->i2c_dev);
    }
    free(driver);
}

bool PWMServoDriver_begin(PWMServoDriver* driver, uint8_t prescale) {
    if (driver->i2c_dev == NULL) {
        driver->i2c_dev = I2CDevice_create(driver->_i2caddr, NULL); // NULLは実際のI2Cオブジェクトに置き換える
    }
    if (!I2CDevice_begin(driver->i2c_dev)) {
        return false;
    }
    PWMServoDriver_reset(driver);

    // デフォルトの内部周波数を設定
    PWMServoDriver_setOscillatorFrequency(driver, FREQUENCY_OSCILLATOR);

    if (prescale) {
        PWMServoDriver_setExtClk(driver, prescale);
    } else {
        // デフォルト周波数を設定
        PWMServoDriver_setPWMFreq(driver, 1000);
    }

    return true;
}

void PWMServoDriver_reset(PWMServoDriver* driver) {
    PWMServoDriver_write8(driver, PCA9685_MODE1, MODE1_RESTART);
    sleep(1);
}

void PWMServoDriver_sleep(PWMServoDriver* driver) {
    uint8_t awake = PWMServoDriver_read8(driver, PCA9685_MODE1);
    uint8_t sleep = awake | MODE1_SLEEP; // スリープビットをセット
    PWMServoDriver_write8(driver, PCA9685_MODE1, sleep);
    usleep(5000); // スリープが有効になるまで待つ
}

void PWMServoDriver_wakeup(PWMServoDriver* driver) {
    uint8_t sleep = PWMServoDriver_read8(driver, PCA9685_MODE1);
    uint8_t wakeup = sleep & ~MODE1_SLEEP; // スリープビットをクリア
    PWMServoDriver_write8(driver, PCA9685_MODE1, wakeup);
}

void PWMServoDriver_setExtClk(PWMServoDriver* driver, uint8_t prescale) {
    uint8_t oldmode = PWMServoDriver_read8(driver, PCA9685_MODE1);
    uint8_t newmode = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // スリープ
    PWMServoDriver_write8(driver, PCA9685_MODE1, newmode); // スリープ状態にし、内部オシレータをオフにする

    // スリープとEXTCLKビットをセットして、外部クロックを使用
    PWMServoDriver_write8(driver, PCA9685_MODE1, (newmode |= MODE1_EXTCLK));

    PWMServoDriver_write8(driver, PCA9685_PRESCALE, prescale); // プリスケーラを設定

    usleep(5000);
    // スリープビットをクリアして開始
    PWMServoDriver_write8(driver, PCA9685_MODE1, (newmode & ~MODE1_SLEEP) | MODE1_RESTART | MODE1_AI);
}

void PWMServoDriver_setPWMFreq(PWMServoDriver* driver, float freq) {
    // オシレータ依存の周波数出力範囲
    if (freq < 1) freq = 1;
    if (freq > 3500) freq = 3500; // データシートの制限は3052=50MHz/(4*4096)

    float prescaleval = ((driver->_oscillator_freq / (freq * 4096.0)) + 0.5) - 1;
    if (prescaleval < PCA9685_PRESCALE_MIN) prescaleval = PCA9685_PRESCALE_MIN;
    if (prescaleval > PCA9685_PRESCALE_MAX) prescaleval = PCA9685_PRESCALE_MAX;
    uint8_t prescale = (uint8_t)prescaleval;

    uint8_t oldmode = PWMServoDriver_read8(driver, PCA9685_MODE1);
    uint8_t newmode = (oldmode & ~MODE1_RESTART) | MODE1_SLEEP; // スリープ
    PWMServoDriver_write8(driver, PCA9685_MODE1, newmode); // スリープ状態にする
    PWMServoDriver_write8(driver, PCA9685_PRESCALE, prescale); // プリスケーラを設定
    PWMServoDriver_write8(driver, PCA9685_MODE1, oldmode);
    usleep(5000);
    // MODE1レジスタを設定して自動インクリメントをオンにする
    PWMServoDriver_write8(driver, PCA9685_MODE1, oldmode | MODE1_RESTART | MODE1_AI);
}

void PWMServoDriver_setOutputMode(PWMServoDriver* driver, bool totempole) {
    uint8_t oldmode = PWMServoDriver_read8(driver, PCA9685_MODE2);
    uint8_t newmode;
    if (totempole) {
        newmode = oldmode | MODE2_OUTDRV;
    } else {
        newmode = oldmode & ~MODE2_OUTDRV;
    }
    PWMServoDriver_write8(driver, PCA9685_MODE2, newmode);
}

uint8_t PWMServoDriver_readPrescale(PWMServoDriver* driver) {
    return PWMServoDriver_read8(driver, PCA9685_PRESCALE);
}

uint16_t PWMServoDriver_getPWM(PWMServoDriver* driver, uint8_t num, bool off) {
    uint8_t buffer[2] = {PCA9685_LED0_ON_L + 4 * num, 0};
    if (off) buffer[0] += 2;
    I2CDevice_write_then_read(driver->i2c_dev, buffer, 1, buffer, 2);
    return (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
}

uint8_t PWMServoDriver_setPWM(PWMServoDriver* driver, uint8_t num, uint16_t on, uint16_t off) {
    uint8_t buffer[5];
    buffer[0] = PCA9685_LED0_ON_L + 4 * num;
    buffer[1] = on;
    buffer[2] = on >> 8;
    buffer[3] = off;
    buffer[4] = off >> 8;

    if (I2CDevice_write(driver->i2c_dev, buffer, 5)) {
        return 0;
    } else {
        return 1;
    }
}

void PWMServoDriver_setPin(PWMServoDriver* driver, uint8_t num, uint16_t val, bool invert) {
    // 0から4095までの値にクランプ
    val = val > 4095 ? 4095 : val;
    if (invert) {
        if (val == 0) {
            // 信号が完全にオンの特別な値
            PWMServoDriver_setPWM(driver, num, 4096, 0);
        } else if (val == 4095) {
            // 信号が完全にオフの特別な値
            PWMServoDriver_setPWM(driver, num, 0, 4096);
        } else {
            PWMServoDriver_setPWM(driver, num, 0, 4095 - val);
        }
    } else {
        if (val == 4095) {
            // 信号が完全にオンの特別な値
            PWMServoDriver_setPWM(driver, num, 4096, 0);
        } else if (val == 0) {
            // 信号が完全にオフの特別な値
            PWMServoDriver_setPWM(driver, num, 0, 4096);
        } else {
            PWMServoDriver_setPWM(driver, num, 0, val);
        }
    }
}

void PWMServoDriver_writeMicroseconds(PWMServoDriver* driver, uint8_t num, uint16_t Microseconds) {
    double pulse = Microseconds;
    double pulselength;

    // 1秒あたりのマイクロ秒数
    pulselength = 1000000.0;

    // プリスケールを読み取る
    uint16_t prescale = PWMServoDriver_readPrescale(driver);

    // データシートのセクション7.3.5の方程式1に基づいてPWMのパルスを計算
    prescale += 1;
    pulselength *= prescale;
    pulselength /= driver->_oscillator_freq;

    // パルス幅を調整
    pulse /= pulselength;

    PWMServoDriver_setPWM(driver, num, 0, (uint16_t)pulse);
}


uint32_t PWMServoDriver_getOscillatorFrequency(PWMServoDriver* driver) {
    return driver->_oscillator_freq;
}

void PWMServoDriver_setOscillatorFrequency(PWMServoDriver* driver, uint32_t freq) {
    driver->_oscillator_freq = freq;
}

uint8_t PWMServoDriver_read8(PWMServoDriver* driver, uint8_t addr) {
    uint8_t buffer[1] = {addr};
    I2CDevice_write_then_read(driver->i2c_dev, buffer, 1, buffer, 1);
    return buffer[0];
}

void PWMServoDriver_write8(PWMServoDriver* driver, uint8_t addr, uint8_t d) {
    uint8_t buffer[2] = {addr, d};
    I2CDevice_write(driver->i2c_dev, buffer, 2);
}
