#include "adxl345.h"

uint8_t ADXL345_Init(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin) {
    // Configurer le CS comme haut initialement
    cs_port->BSRR = (1 << cs_pin);
    delay_ms(10);

    // Lire et verifier l'ID de l'appareil
    uint8_t deviceId = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DEVID);
    if (deviceId != 0xE5) {
       return 0;  // echec de l'initialisation
    }

    // Configurer le capteur
    SPI_Write(SPIx, cs_port, cs_pin, ADXL345_POWER_CTL, 0x08); // Reinitialiser POWER_CTL
    delay_ms(1);
    SPI_Write(SPIx, cs_port, cs_pin, ADXL345_DATA_FORMAT, 0x00); // ±2g et SPI à 4 fils
    delay_ms(1);
    SPI_Write(SPIx, cs_port, cs_pin, ADXL345_POWER_CTL, 0x08); // Activer le mode de mesure
    delay_ms(1);
    SPI_Write(SPIx, cs_port, cs_pin, ADXL345_BW_RATE, 0x0A); // Taux de donnees : 100Hz
    delay_ms(1);

    // Verifier POWER_CTL
    uint8_t powerCtl = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_POWER_CTL);
    return (powerCtl & 0x08) ? 1 : 0; // Succes si le bit de mesure est actif
}

void ADXL345_TestCommunication(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin) {
    uint8_t deviceId = ADXL345_GetID(SPIx, cs_port, cs_pin);
    char buffer[50];
    
    sprintf(buffer, "Device ID: 0x%02X\r\n", deviceId);
    USART2_SendString(buffer);

    if (deviceId == 0xE5) {
        USART2_SendString("ADXL345 trouve avec succes!\r\n");
    } else {
        USART2_SendString("Erreur : ADXL345 non trouve.\r\n");
    }
}

uint8_t ADXL345_GetID(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin) {
    return SPI_Read(SPIx, GPIOA, cs_pin, ADXL345_DEVID);
}

uint8_t ADXL345_ReadRegister(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg) {
    return SPI_Read(SPIx, cs_port, cs_pin, reg);
}

ADXL345_Data ADXL345_ReadAllRegisters(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin) {
    ADXL345_Data data;

    // Lire les registres
    data.devid = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DEVID);
    data.power_ctl = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_POWER_CTL);
    data.data_format = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DATA_FORMAT);
    data.bw_rate = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_BW_RATE);
    data.fifo_ctl =SPI_Read(SPIx, cs_port, cs_pin, ADXL345_FIFO_CTL);
    data.int_source = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_INT_SOURCE);

    // Lire les donnees d'acceleration
    uint8_t accel_data[6];
    SPI_ReadN(SPIx, cs_port, cs_pin, 0x32, accel_data, 6);
    // uint8_t x_l = SPI_Read(SPIx, cs_port, cs_pin, 0x32);
    // uint8_t x_h = SPI_Read(SPIx, cs_port, cs_pin, 0x33);
    // uint8_t y_l = SPI_Read(SPIx, cs_port, cs_pin, 0x34);
    // uint8_t y_h = SPI_Read(SPIx, cs_port, cs_pin, 0x35);
    // uint8_t z_l = SPI_Read(SPIx, cs_port, cs_pin, 0x36);
    // uint8_t z_h = SPI_Read(SPIx, cs_port, cs_pin, 0x37);
    

    // data.x_data = (int16_t)((x_h << 8) | x_l);
    // data.y_data = (int16_t)((y_h << 8) | y_l);
    // data.z_data = (int16_t)((z_h << 8) | z_l);
    data.x_data = (int16_t)((accel_data[1] << 8) | accel_data[0]);
    data.y_data = (int16_t)((accel_data[3] << 8) | accel_data[2]);
    data.z_data = (int16_t)((accel_data[5] << 8) | accel_data[4]);
    return data;
}

void ADXL345_SendDataUSART(ADXL345_Data data) {
    char buffer[100];
    
    // Envoyer l'en-tete
    USART2_SendString("\r\n=== Donnees des registres ADXL345 ===\r\n");
    
    // Envoyer les donnees des registres de configuration
    sprintf(buffer, "Device ID: 0x%02X\r\n", data.devid);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Controle de l'alimentation: 0x%02X\r\n", data.power_ctl);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Format des donnees: 0x%02X\r\n", data.data_format);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Taux de bande passante: 0x%02X\r\n", data.bw_rate);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Controle FIFO: 0x%02X\r\n", data.fifo_ctl);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Source d'interruption: 0x%02X\r\n", data.int_source);
    USART2_SendString(buffer);
    
    // Envoyer les donnees d'acceleration
    sprintf(buffer, "Donnees d'acceleration:\r\n");
    USART2_SendString(buffer);
    sprintf(buffer, "X: %d\r\n", data.x_data);
    USART2_SendString(buffer);
    sprintf(buffer, "Y: %d\r\n", data.y_data);
    USART2_SendString(buffer);
    sprintf(buffer, "Z: %d\r\n", data.z_data);
    USART2_SendString(buffer);
    
    USART2_SendString("===========================\r\n");
}

void ADXL345_ReadAcceleration(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, int16_t *x, int16_t *y, int16_t *z) {
    uint8_t x0 = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DATAX0);
    uint8_t x1 = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DATAX0 + 1);
    uint8_t y0 = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DATAX0 + 2);
    uint8_t y1 = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DATAX0 + 3);
    uint8_t z0 = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DATAX0 + 4);
    uint8_t z1 = SPI_Read(SPIx, cs_port, cs_pin, ADXL345_DATAX0 + 5);

    *x = ((int16_t)x1 << 8) | x0;
    *y = ((int16_t)y1 << 8) | y0;
    *z = ((int16_t)z1 << 8) | z0;
}