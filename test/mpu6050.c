#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>

#define MPU6050_ADDR 0x68

// レジスタアドレス
#define WHO_AM_I    0x75
#define SELF_TEST_X 0x0D
#define SELF_TEST_Y 0x0E
#define SELF_TEST_Z 0x0F
#define SELF_TEST_A 0x10
#define ACCEL_XOUT_H 0x3B
#define ACCEL_YOUT_H 0x3D
#define ACCEL_ZOUT_H 0x3F
#define TEMP_OUT_H   0x41
#define GYRO_XOUT_H  0x43
#define GYRO_YOUT_H  0x45
#define GYRO_ZOUT_H  0x47
#define PWR_MGMT_1   0x6B

#define RAD_TO_DEG  57.295779513082320876798154814105
#define GYRO_SENSITIVITY 131.0
#define ACCEL_SENSITIVITY 16384.0

// 16ビットのレジスタからデータを取得
int16_t read_word_2c(int fd, uint8_t reg) 
{
    uint8_t buf[2];
    if (write(fd, &reg, 1) != 1) {
        perror("Failed to write register address");
        return 0;
    }
    if (read(fd, buf, 2) != 2) {
        perror("Failed to read register value");
        return 0;
    }
    return (buf[0] << 8) | buf[1];
}

// 1バイトを読み込む
uint8_t read_byte(int fd, uint8_t reg) 
{
    uint8_t value;
    if (write(fd, &reg, 1) != 1) {
        perror("Failed to write register address");
        return 0;
    }
    if (read(fd, &value, 1) != 1) {
        perror("Failed to read register value");
        return 0;
    }
    return value;
}

// レジスタに1バイトを書き込む
void write_byte(int fd, uint8_t reg, uint8_t value) 
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    if (write(fd, buf, 2) != 2) {
        perror("I2C write failed");
    }
}

// WHO_AM_Iの値を確認
void check_device_id(int fd) 
{
    uint8_t who_am_i = read_byte(fd, WHO_AM_I);
    printf("WHO_AM_I register: 0x%X\n", who_am_i);
}

// セルフテストの実行
void run_self_test(int fd) 
{
    uint8_t self_test_x = read_byte(fd, SELF_TEST_X);
    uint8_t self_test_y = read_byte(fd, SELF_TEST_Y);
    uint8_t self_test_z = read_byte(fd, SELF_TEST_Z);
    uint8_t self_test_a = read_byte(fd, SELF_TEST_A);

    printf("Self Test X: 0x%X\n", self_test_x);
    printf("Self Test Y: 0x%X\n", self_test_y);
    printf("Self Test Z: 0x%X\n", self_test_z);
    printf("Self Test A: 0x%X\n", self_test_a);
}

// 加速度とジャイロデータの取得
void get_sensor_data(int fd, float *ax, float *ay, float *az, float *gx, float *gy, float *gz, float *temp) 
{
    *ax = read_word_2c(fd, ACCEL_XOUT_H) / ACCEL_SENSITIVITY;
    *ay = read_word_2c(fd, ACCEL_YOUT_H) / ACCEL_SENSITIVITY;
    *az = read_word_2c(fd, ACCEL_ZOUT_H) / ACCEL_SENSITIVITY;

    *gx = read_word_2c(fd, GYRO_XOUT_H) / GYRO_SENSITIVITY;
    *gy = read_word_2c(fd, GYRO_YOUT_H) / GYRO_SENSITIVITY;
    *gz = read_word_2c(fd, GYRO_ZOUT_H) / GYRO_SENSITIVITY;

    int16_t temp_raw = read_word_2c(fd, TEMP_OUT_H);
    *temp = temp_raw / 340.0 + 36.53;  // 温度計算公式
}

// ピッチとロールの計算
void calculate_pitch_roll(float ax, float ay, float az, float *pitch, float *roll) 
{
    *pitch = atan(ay / sqrt(ax * ax + az * az)) * RAD_TO_DEG;
    *roll = atan(-ax / az) * RAD_TO_DEG;
}

int main() 
{
    int fd;
    char *filename = (char*)"/dev/i2c-1";

    // I2Cデバイスのオープン
    if ((fd = open(filename, O_RDWR)) < 0) 
    {
        perror("Failed to open the i2c bus");
        return 1;
    }

    // MPU6050をスタートさせるためにPWR_MGMT_1をクリア
    write_byte(fd, PWR_MGMT_1, 0x00);

    // WHO_AM_Iレジスタを確認
    check_device_id(fd);

    // セルフテストの実行
    run_self_test(fd);

    // センサーデータの取得
    float ax, ay, az, gx, gy, gz, temp, pitch, roll;

    while (1)
    {
        // 加速度,ジャイロ,温度データの取得
        get_sensor_data(fd, &ax, &ay, &az, &gx, &gy, &gz &temp);
        
        // ピッチとロールの計算
        calculate_pitch_roll(ax, ay, az, &pitch, &roll)

        // センサーデータの表示
        printf("Accel: ax=%.2f, ay=%.2f, az=%.2f\n", ax, ay, az);
        printf("Gyro: gx=%.2f, gy=%.2f, gz=%.2f\n", gx, gy, gz);
        printf("Temp: %.2f°C\n", temp);
        printf("Pitch: %.2f°, Roll: %.2f°\n", pitch, roll);
        printf("---------------------------------\n");

        usleep(500000);  
    }

    close(fd);
    return 0;
}
