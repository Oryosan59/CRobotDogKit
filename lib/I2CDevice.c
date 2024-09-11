#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// ダミーI2Cデバイス構造体
typedef struct {
    uint8_t addr;
    int file;
} I2CDevice;

// I2Cバスをオープンする関数
int i2c_open(const char *device) {
    int file;
    if ((file = open(device, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        exit(1);
    }
    return file;
}

// I2Cデバイスを選択する関数
void i2c_select_device(int file, int addr) {
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }
}

// I2Cデバイスの作成
void* I2CDevice_create(uint8_t addr, void* i2c) {
    I2CDevice* device = (I2CDevice*)malloc(sizeof(I2CDevice));
    if (device) {
        device->addr = addr;
        device->file = i2c_open("/dev/i2c-1"); // Jetson Nanoではi2c-1を使用
        i2c_select_device(device->file, addr);
    }
    return device;
}

// I2Cデバイスの破棄
void I2CDevice_destroy(void* device) {
    if (device) {
        close(((I2CDevice*)device)->file);
        free(device);
    }
}

// I2Cデバイスの初期化
bool I2CDevice_begin(void* device) {
    // 実際のデバイス初期化コードをここに実装
    return true;
}

// I2C書き込みと読み込み
bool I2CDevice_write_then_read(void* device, uint8_t* write_buf, size_t write_len, uint8_t* read_buf, size_t read_len) {
    I2CDevice* dev = (I2CDevice*)device;
    if (write(dev->file, write_buf, write_len) != write_len) {
        perror("Failed to write to the i2c bus");
        return false;
    }
    if (read(dev->file, read_buf, read_len) != read_len) {
        perror("Failed to read from the i2c bus");
        return false;
    }
    return true;
}

// I2C書き込み
bool I2CDevice_write(void* device, uint8_t* buffer, size_t len) {
    I2CDevice* dev = (I2CDevice*)device;
    if (write(dev->file, buffer, len) != len) {
        perror("Failed to write to the i2c bus");
        return false;
    }
    return true;
}

