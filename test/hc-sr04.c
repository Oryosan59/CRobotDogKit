#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

#define GPIO_BASE 0xFE200000  // GPIOベースアドレス (バスアドレス)
#define BLOCK_SIZE (4*1024)   // メモリマッピングのサイズ

// GPIOレジスタオフセット
#define GPFSEL1 0x04          // GPIO Function Select 1 (GPIO10-19の設定)
#define GPSET0 0x1C           // GPIO Pin Output Set 0
#define GPCLR0 0x28           // GPIO Pin Output Clear 0
#define GPLEV0 0x34           // GPIO Pin Level 0

// GPIOピン番号
#define TRIG_PIN 23  // GPIO23
#define ECHO_PIN 24  // GPIO24

// 音速
#define SOUND_SPEED 34300  // cm/s

volatile unsigned int *gpio_addr;

// GPIOをメモリマップする
int setup_io()
{
    int mem_fd = open("/dev/mem" , O_RDWR | O_SYNC);
    if (men_fd < 0)
    {
        perror("Failed to open /dev/mem");
        return -1;
    }

    gpio_addr = (unsigned int*)mmap
    (
        NULL;
        BLOCK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        mem_fd
        GPIO_BASE
    );

    close(mem_fd);

    if (gpio == MAP_FAILED)
    {
        perror("Failed to mmap GPIO");
        return -1;
    }
    return 0;
    
}

// GPIOの出力設定
void set_gpio_ouotput(int pin)
{
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    gpio_addr[GPFSEL1 / 4 + reg] &= ~(7 << shift);
    gpio_addr[GPFSEL1 / 4 + reg] |= (1 << shift);
    __sync_synchronize();
}

// GPIOの入力設定
void set_gpio_input(int pin)
{
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    gpio_addr[GPFSEL1 / 4 + reg] &= ~(7 << shift);
    __sync_synchronize();
}

// GPIOピンをHighにする
void gpio_set(int pin)
{
    gpio_addr[GPSET0 / 4] = (1 << pin);
}

// GPIOピンをLowにする
void gpio_clear(int pin)
{
    gpio_addr[GPCLR0 / 4] = (1 << pin);
}

// GPIOピンの状態確認
int gpio_read(int pin)
{
    return (gpio_addr[GPLEV0 / 4] & (1 << pin)) != 0;
}

// マイクロ秒単位で、現在時刻を取得
long get_microseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

// 距離計測
float measure_distance() 
{
    // Triggerに10µsパルスを送信
    gpio_clear(TRIG_PIN);
    usleep(2);  // 2µs待機
    gpio_set(TRIG_PIN);
    nanosleep((const struct timespec[]){{0, 10000L}}, NULL);  // 10µs待機
    gpio_clear(TRIG_PIN);

    // EchoがHighになるまで待機
    while (gpio_read(ECHO_PIN) == 0);

    // EchoがHighの間の時間を計測
    long start_time = get_microseconds();
    while (gpio_read(ECHO_PIN) == 1);
    long end_time = get_microseconds();

    // パルス持続時間の計算
    long travel_time = end_time - start_time;

    // 距離を計算（cm単位）
    float distance = (travel_time * SOUND_SPEED) / 2.0 / 1000000.0;
    return distance;
}

int main()
{
    if (setup_io() == -1) 
    {
        return -1;  // GPIO初期化失敗時に終了
    }

    set_gpio_output(TRIG_PIN);  // Triggerピンを出力に設定
    set_gpio_input(ECHO_PIN);   // Echoピンを入力に設定

    while (1) 
    {
        float distance = measure_distance();
        printf("Measured Distance: %.2f cm\n", distance);
        sleep(1);  // 1秒ごとに計測
    }

    return 0;
}