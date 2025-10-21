Sistema de Leitura de Sensores I2C e ADC ? SBC Labrador

Projeto desenvolvido para realizar a leitura de temperatura, press�o e luminosidade utilizando sensores conectados ao SBC Labrador. O sistema usa a interface I2C para comunica��o com os sensores digitais BMP280 e BH1750, e leitura anal�gica via ADC para os valores de resist�ncia do pot�nciometro.

Objetivo:
Ler e exibir em tempo real os valores de temperatura (BMP280), press�o atmosf�rica (BMP280) e luminosidade (BH1750), demonstrando o uso de comunica��o I2C e leitura anal�gica em sistemas embarcados Linux.

Funcionalidades:

Leitura de temperatura e press�o do sensor BMP280 com compensa��o de calibra��o.

Leitura de luminosidade em lux do sensor BH1750.

Leitura de temperatura anal�gica, com base nos valores de de resist�ncia do pot�nciometro, atrav�s do ADC do sistema.

Exibi��o dos valores no terminal a cada 2 segundos.

Componentes utilizados:

BMP280 ? Sensor de temperatura e press�o (I2C ? endere�o 0x76)

BH1750 ? Sensor de luminosidade (I2C ? endere�o 0x23)

Pot�nciometro ? Representans os valores de um Sensor anal�gico de temperatura (entrada ADC0)

SBC Labrador ? Interface I2C-2 (/dev/i2c-2) e ADC (/sys/kernel/auxadc/adc0)

Diagrama de conex�o:
SBC Labrador
??? I2C SDA/SCL ? BMP280 e BH1750
??? ADC0 ? Sa�da do pot�nciometro
??? 3V3/GND ? Alimenta��o dos sensores