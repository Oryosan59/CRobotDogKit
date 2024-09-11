#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>

#define ACCEL_X_OUT 0x3b
#define ACCEL_Y_OUT 0x3d
#define ACCEL_Z_OUT 0x3f
#define TEMP_OUT    0x41
#define GYRO_X_OUT  0x43
#define GYRO_Y_OUT  0x45
#define GYRO_Z_OUT  0x47

#define PWR_MGMT_1  0x6B
#define PWR_MGMT_2  0x6C
#define DEV_ADDR    0x68

#define RAD_TO_DEG  57.324
#define GYRO_SENSITIVITY 131.0
#define ACCEL_SENSITIVITY 16384.0

// 16bitデータを2バイトから取得する
int16_t read_word_2c(int fd, uint8_t reg){
    uint8_t buf[2];
    if (read(fd, buf, 2) != 2) {
        perror("I2C read failed");
        return 0;
    }
    int16_t value = (buf[0] << 8) | buf[1];
    if (value >= 32768) value -= 65536;
    return value;
}

// 加速度データ取得 (X, Y, Z)
void get_accel_data(int fd, float *ax, float *ay, float *az) {
    *ax = read_word_2c(fd, ACCEL_X_OUT) / ACCEL_SENSITIVITY;
    *ay = read_word_2c(fd, ACCEL_Y_OUT) / ACCEL_SENSITIVITY;
    *az = read_word_2c(fd, ACCEL_Z_OUT) / ACCEL_SENSITIVITY;
}

// ジャイロデータ取得 (X, Y, Z)
void get_gyro_data(int fd, float *gx, float *gy, float *gz) {
    *gx = read_word_2c(fd, GYRO_X_OUT) / GYRO_SENSITIVITY;
    *gy = read_word_2c(fd, GYRO_Y_OUT) / GYRO_SENSITIVITY;
    *gz = read_word_2c(fd, GYRO_Z_OUT) / GYRO_SENSITIVITY;
}

// 温度データ取得
float get_temp_data(int fd) {
    int16_t temp_raw = read_word_2c(fd, TEMP_OUT);
    return temp_raw / 340.0 + 36.53; // データシートの公式より
}

// ピッチとロールの計算
void calculate_pitch_roll(float ax, float ay, float az, float *pitch, float *roll) {
    *pitch = atan(ay / az) * RAD_TO_DEG;
    *roll = atan(-ax / sqrtf(ay * ay + az * az)) * RAD_TO_DEG;
}

void mainloop(int fd) {
    float ax, ay, az;
    float gx, gy, gz;
    float pitch, roll;
    float temperature;

    while (1) {
        // 加速度データの取得
        get_accel_data(fd, &ax, &ay, &az);
        // ジャイロデータの取得
        get_gyro_data(fd, &gx, &gy, &gz);
        // 温度データの取得
        temperature = get_temp_data(fd);
        // ピッチとロールの計算
        calculate_pitch_roll(ax, ay, az, &pitch, &roll);

        // データの表示
        printf("Accel: ax=%.2f, ay=%.2f, az=%.2f\n", ax, ay, az);
        printf("Gyro: gx=%.2f, gy=%.2f, gz=%.2f\n", gx, gy, gz);
        printf("Temp: %.2f C\n", temperature);
        printf("Pitch: %.2f, Roll: %.2f\n", pitch, roll);
        printf("------------------------\n");

        usleep(500000);  // 500ms待機
    }
}

int main() {
    int fd;
    char *filename = (char*)"/dev/i2c-1";

    // I2Cデバイスのオープン
    if ((fd = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return 1;
    }

    // MPU6050のアドレス指定
    if (ioctl(fd, I2C_SLAVE, DEV_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        return 1;
    }

    // MPU6050の起動
    uint8_t buf[2] = {PWR_MGMT_1, 0};
    if (write(fd, buf, 2) != 2) {
        perror("Failed to reset MPU6050");
        return 1;
    }

    // メインループ
    mainloop(fd);

    close(fd);
    return 0;
}
