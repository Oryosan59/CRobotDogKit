#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define GNSS_I2C_ADDRESS 0x20  // GNSSモジュールのI2Cアドレス
#define I2C_YEAR_H 0
#define I2C_LAT_1 7
#define I2C_LON_1 13
#define I2C_ALT_H 20
#define I2C_SOG_H 23
#define I2C_COG_H 26
#define I2C_USE_STAR 19

int i2c_fd;

// I2Cデバイスの初期化
int initI2C(const char *device, int address) {
    if ((i2c_fd = open(device, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        return -1;
    }
    return 0;
}

// I2Cレジスタの読み込み関数
int readReg(uint8_t reg, uint8_t *data, uint8_t len) {
    if (write(i2c_fd, &reg, 1) != 1) {
        perror("Failed to select register");
        return -1;
    }
    if (read(i2c_fd, data, len) != len) {
        perror("Failed to read register data");
        return -1;
    }
    return 0;
}

// 日付の取得
void getDate() {
    uint8_t dateData[4];
    if (readReg(I2C_YEAR_H, dateData, 4) == 0) {
        uint16_t year = (dateData[0] << 8) | dateData[1];
        uint8_t month = dateData[2];
        uint8_t date = dateData[3];
        printf("Date: %04d-%02d-%02d\n", year, month, date);
    }
}

// 緯度の取得
void getLat() {
    uint8_t latData[6];
    if (readReg(I2C_LAT_1, latData, 6) == 0) {
        uint8_t latDD = latData[0];
        uint8_t latMM = latData[1];
        uint32_t latMMMMM = (latData[2] << 16) | (latData[3] << 8) | latData[4];
        double latitude = latDD * 100.0 + latMM + latMMMMM / 100000.0;
        printf("Latitude: %02d°%02d'%05d\"\n", latDD, latMM, latMMMMM);
        printf("Latitude (Decimal): %.7f\n", latitude);
    }
}

// 経度の取得
void getLon() {
    uint8_t lonData[6];
    if (readReg(I2C_LON_1, lonData, 6) == 0) {
        uint8_t lonDDD = lonData[0];
        uint8_t lonMM = lonData[1];
        uint32_t lonMMMMM = (lonData[2] << 16) | (lonData[3] << 8) | lonData[4];
        double longitude = lonDDD * 100.0 + lonMM + lonMMMMM / 100000.0;
        printf("Longitude: %03d°%02d'%05d\"\n", lonDDD, lonMM, lonMMMMM);
        printf("Longitude (Decimal): %.7f\n", longitude);
    }
}

// 高度の取得
void getAlt() {
    uint8_t altData[3];
    if (readReg(I2C_ALT_H, altData, 3) == 0) {
        double altitude = (double)((altData[0] << 8) | altData[1]) + (double)altData[2] / 100.0;
        printf("Altitude: %.2f meters\n", altitude);
    }
}

// 使用中の衛星の数を取得
void getNumSatUsed() {
    uint8_t numSatData[1];
    if (readReg(I2C_USE_STAR, numSatData, 1) == 0) {
        printf("Number of satellites used: %d\n", numSatData[0]);
    }
}

int main() {
    // I2Cの初期化
    if (initI2C("/dev/i2c-1", GNSS_I2C_ADDRESS) < 0) {
        return 1;
    }

    // データの取得と表示
    getDate();
    getLat();
    getLon();
    getAlt();
    getNumSatUsed();

    // I2Cデバイスをクローズ
    close(i2c_fd);
    return 0;
}
