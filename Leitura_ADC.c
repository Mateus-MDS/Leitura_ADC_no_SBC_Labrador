#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

typedef unsigned char u8;

// ===================================================
// VARIÁVEIS GLOBAIS
// ===================================================
int i2c_fd = -1;
const char *i2c_fname = "/dev/i2c-2";
const char *adc_path = "/sys/kernel/auxadc/adc0";

// Coeficientes de calibração do BMP280
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
} bmp280_calib_data;

bmp280_calib_data calib;
int32_t t_fine;

// ===================================================
// FUNÇÕES I2C
// ===================================================
int i2c_init(void) {
    if ((i2c_fd = open(i2c_fname, O_RDWR)) < 0) {
        perror("Erro ao abrir I2C");
        return -1;
    }
    return i2c_fd;
}

int i2c_write(u8 slave_addr, u8 reg, u8 data) {
    u8 outbuf[2] = {reg, data};
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data msgset;
    
    msg.addr = slave_addr;
    msg.flags = 0;
    msg.len = 2;
    msg.buf = outbuf;
    
    msgset.msgs = &msg;
    msgset.nmsgs = 1;
    
    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("Erro no write I2C");
        return -1;
    }
    return 0;
}

int i2c_write_cmd(u8 slave_addr, u8 cmd) {
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data msgset;
    
    msg.addr = slave_addr;
    msg.flags = 0;
    msg.len = 1;
    msg.buf = &cmd;
    
    msgset.msgs = &msg;
    msgset.nmsgs = 1;
    
    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("Erro no write I2C");
        return -1;
    }
    return 0;
}

int i2c_read_bytes(u8 slave_addr, u8 reg, u8 *buffer, u8 length) {
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    
    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &reg;
    
    msgs[1].addr = slave_addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = length;
    msgs[1].buf = buffer;
    
    msgset.msgs = msgs;
    msgset.nmsgs = 2;
    
    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("Erro na leitura I2C");
        return -1;
    }
    return 0;
}

int i2c_read_raw(u8 slave_addr, u8 *buffer, u8 length) {
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data msgset;
    
    msg.addr = slave_addr;
    msg.flags = I2C_M_RD;
    msg.len = length;
    msg.buf = buffer;
    
    msgset.msgs = &msg;
    msgset.nmsgs = 1;
    
    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        perror("Erro na leitura I2C");
        return -1;
    }
    return 0;
}

// ===================================================
// BH1750 - SENSOR DE LUMINOSIDADE
// ===================================================
float lerBH1750() {
    u8 slave_addr = 0x23;
    u8 buf[2];
    
    i2c_write_cmd(slave_addr, 0x10);
    usleep(180000);
    
    if (i2c_read_raw(slave_addr, buf, 2) == 0) {
        uint16_t raw = (buf[0] << 8) | buf[1];
        float lux = raw / 1.2;
        return lux;
    }
    return -1;
}

// ===================================================
// BMP280 - SENSOR DE TEMPERATURA E PRESSÃO
// ===================================================
void lerCalibracaoBMP280() {
    u8 slave_addr = 0x76;
    u8 buf[24];
    
    if (i2c_read_bytes(slave_addr, 0x88, buf, 24) == 0) {
        calib.dig_T1 = (buf[1] << 8) | buf[0];
        calib.dig_T2 = (buf[3] << 8) | buf[2];
        calib.dig_T3 = (buf[5] << 8) | buf[4];
        calib.dig_P1 = (buf[7] << 8) | buf[6];
        calib.dig_P2 = (buf[9] << 8) | buf[8];
        calib.dig_P3 = (buf[11] << 8) | buf[10];
        calib.dig_P4 = (buf[13] << 8) | buf[12];
        calib.dig_P5 = (buf[15] << 8) | buf[14];
        calib.dig_P6 = (buf[17] << 8) | buf[16];
        calib.dig_P7 = (buf[19] << 8) | buf[18];
        calib.dig_P8 = (buf[21] << 8) | buf[20];
        calib.dig_P9 = (buf[23] << 8) | buf[22];
    }
}

float compensarTemperatura(int32_t adc_T) {
    int32_t var1, var2;
    
    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) * ((int32_t)calib.dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    float T = (t_fine * 5 + 128) >> 8;
    return T / 100.0;
}

float compensarPressao(int32_t adc_P) {
    int64_t var1, var2, p;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    
    return (float)p / 256.0 / 100.0;
}

void lerBMP280(float *tempC, float *pressao) {
    u8 slave_addr = 0x76;
    u8 buf[6];
    
    if (i2c_read_bytes(slave_addr, 0xF7, buf, 6) == 0) {
        int32_t adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)buf[2] >> 4);
        int32_t adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)buf[5] >> 4);
        
        *tempC = compensarTemperatura(adc_T);
        *pressao = compensarPressao(adc_P);
    } else {
        *tempC = -1;
        *pressao = -1;
    }
}

// ===================================================
// LM35 - SENSOR DE TEMPERATURA ANALÓGICO (ADC)
// ===================================================
float lerLM35() {
    FILE *fp;
    unsigned long timestamp = 0;
    int value = 0, scale = 0;
    float temperatura = -1;
    
    fp = fopen(adc_path, "r");
    if (fp == NULL) {
        perror("Erro ao abrir ADC");
        return -1;
    }
    
    if (fscanf(fp, "%lu %d / %d", &timestamp, &value, &scale) == 3) {
        float voltage = (value * 3.3f) / scale;
        temperatura = voltage / 0.01;
    }
    
    fclose(fp);
    return temperatura;
}

// ===================================================
// PROGRAMA PRINCIPAL
// ===================================================
int main() {
    if (i2c_init() < 0) {
        return 1;
    }
    
    u8 bmp_addr = 0x76;
    i2c_write(bmp_addr, 0xF4, 0x27);
    i2c_write(bmp_addr, 0xF5, 0xA0);
    usleep(100000);
    
    lerCalibracaoBMP280();
    
    float temp_bmp280, pressao, lux, temp_lm35;
    
    while (1) {
        lerBMP280(&temp_bmp280, &pressao);
        lux = lerBH1750();
        temp_lm35 = lerLM35();
        
        printf("Temperatura BMP280: %.2f C\n", temp_bmp280);
        printf("Pressao BMP280: %.2f hPa\n", pressao);
        printf("Temperatura LM35: %.2f C\n", temp_lm35);
        printf("Luminosidade: %.2f lux\n", lux);
        printf("\n");
        
        sleep(2);
    }
    
    close(i2c_fd);
    return 0;
}