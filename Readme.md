Sistema de Leitura de Sensores I2C e ADC no SBC Labrador

Projeto desenvolvido para realizar a leitura de temperatura, pressão e luminosidade utilizando sensores conectados ao SBC Labrador. O sistema usa a interface I2C para comunicação com os sensores digitais BMP280 e BH1750, e leitura analógica via ADC para os valores de resistência do potênciometro.

Objetivo:
Ler e exibir em tempo real os valores de temperatura (BMP280), pressão atmosférica (BMP280) e luminosidade (BH1750), demonstrando o uso de comunicação I2C e leitura analógica em sistemas embarcados Linux.

Funcionalidades:

Leitura de temperatura e pressão do sensor BMP280 com compensação de calibração.

Leitura de luminosidade em lux do sensor BH1750.

Leitura de temperatura analógica, com base nos valores de de resistência do potênciometro, através do ADC do sistema.

Exibição dos valores no terminal a cada 2 segundos.

Componentes utilizados:

BMP280 ? Sensor de temperatura e pressão (I2C ? endereço 0x76)

BH1750 ? Sensor de luminosidade (I2C ? endereço 0x23)

Potênciometro ? Representans os valores de um Sensor analógico de temperatura (entrada ADC0)

SBC Labrador ? Interface I2C-2 (/dev/i2c-2) e ADC (/sys/kernel/auxadc/adc0)

Diagrama de conexão:
SBC Labrador
??? I2C SDA/SCL ? BMP280 e BH1750
??? ADC0 ? Saída do potênciometro

??? 3V3/GND ? Alimentação dos sensores
