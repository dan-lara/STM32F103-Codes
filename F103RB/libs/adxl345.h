#ifndef ADXL345_H
#define ADXL345_H

#include "general.h"
#include "spi.h"
#include "uart.h"

#define ADXL345_POWER_CTL   0x2D  // Adresse du registre pour le controle de l'alimentation
#define ADXL345_DATA_FORMAT 0x31  // Adresse du registre pour le format des donnees
#define ADXL345_DEVID       0x00  // Adresse du registre pour l'ID de l'appareil
#define ADXL345_BW_RATE     0x2C  // Adresse du registre pour le taux de bande passante
#define ADXL345_FIFO_CTL    0x38  // Adresse du registre pour le controle FIFO
#define ADXL345_INT_SOURCE  0x30  // Adresse du registre pour la source d'interruption
#define ADXL345_DATAX0      0x32  // Adresse du registre pour les donnees initiales de l'axe X

// Structure pour stocker les donnees de l'ADXL345
typedef struct {
    uint8_t devid;
    uint8_t power_ctl;
    uint8_t data_format;
    uint8_t bw_rate;
    uint8_t fifo_ctl;
    uint8_t int_source;
    int16_t x_data;
    int16_t y_data;
    int16_t z_data;
} ADXL345_Data;

// Prototypage des fonctions
uint8_t ADXL345_Init(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin);

void ADXL345_TestCommunication(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin);
uint8_t ADXL345_GetID(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin);

void ADXL345_ReadAcceleration(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, int16_t *x, int16_t *y, int16_t *z);

ADXL345_Data ADXL345_ReadAllRegisters(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin);
uint8_t ADXL345_ReadRegister(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg);

void ADXL345_SendDataUSART(ADXL345_Data data);

#endif // ADXL345_H