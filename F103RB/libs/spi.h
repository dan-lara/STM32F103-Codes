#ifndef SPI_H
#define SPI_H

#include "general.h"

// Définitions pour la configuration
#define SPI_ENABLE          ((uint16_t)0x0040)
#define SPI_MASTER_MODE     ((uint16_t)0x0004)
#define SPI_8BIT_MODE      ((uint16_t)0x0000)
#define SPI_16BIT_MODE     ((uint16_t)0x0800)
#define SPI_SSOE           ((uint16_t)0x0004)
#define SPI_SSM            ((uint16_t)0x0001)
// Enums pour la sélection des configurations
typedef enum {
    SPI_CLK_DIV_2   = 0,
    SPI_CLK_DIV_4   = 1,
    SPI_CLK_DIV_8   = 2,
    SPI_CLK_DIV_16  = 3,
    SPI_CLK_DIV_32  = 4,
    SPI_CLK_DIV_64  = 5,
    SPI_CLK_DIV_128 = 6,
    SPI_CLK_DIV_256 = 7
} SPI_Time_Div;

// Fonctions d'initialisation
void SPI1_Init(SPI_Time_Div time_div, uint8_t CPOL, uint8_t CPHA, uint8_t data_size);
void SPI2_Init(SPI_Time_Div time_div, uint8_t CPOL, uint8_t CPHA, uint8_t data_size);

// Configuration du CS
void SPI_ConfigCS(GPIO_TypeDef* port, uint16_t pin);

// Fonctions de lecture et d'écriture
uint8_t SPI_Read(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg);
void SPI_ReadN(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg, uint8_t* buffer, uint16_t length);
void SPI_Write(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg, uint8_t data);


// Implémentation des fonctions
void SPI1_Init(SPI_Time_Div time_div, uint8_t CPOL, uint8_t CPHA, uint8_t data_size) {
    // Activer l'horloge pour SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    // Configurer les GPIOs : SCK (PA5), MISO (PA6), MOSI (PA7)
    init_gpio('a', 5, GPIO_AF_PP_50MHZ);
    init_gpio('a', 6, GPIO_INPUT_FLOATING);
    init_gpio('a', 7, GPIO_AF_PP_50MHZ);

    // Configurer SPI1
    SPI1->CR1 = 0;
    SPI1->CR1 = SPI_MASTER_MODE | (time_div << 3) | (CPOL << 1) | (CPHA << 0);
    SPI1->CR1 |= (data_size == 16) ? SPI_16BIT_MODE : SPI_8BIT_MODE;
    SPI1->CR1 |= (1 << 9); //SSM
    SPI1->CR1 |= (1 << 8); //SSI
    
    SPI1->CR2 = 0;
    SPI1->CR2 |= (1 << 2);//SSOE

    SPI1->CR1 |= (1 << 6);//SPI enable
}

void SPI2_Init(SPI_Time_Div time_div, uint8_t CPOL, uint8_t CPHA, uint8_t data_size) {
    // Activer l'horloge pour SPI2
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    // Configurer les GPIOs : SCK (PB13), MISO (PB14), MOSI (PB15)
    init_gpio('b', 13, GPIO_AF_PP_50MHZ);
    init_gpio('b', 14, GPIO_INPUT_FLOATING);
    init_gpio('b', 15, GPIO_AF_PP_50MHZ);

    // Configurer SPI2
    SPI2->CR1 = SPI_MASTER_MODE | (time_div << 3) | (CPOL << 1) | (CPHA << 0);
    SPI2->CR1 |= (data_size == 16) ? SPI_16BIT_MODE : SPI_8BIT_MODE;
    SPI2->CR1 |= SPI_ENABLE;
}

void SPI_ConfigCS(GPIO_TypeDef* port, uint16_t pin) {
    uint8_t pin_position = (pin < 8) ? (pin * 4) : ((pin - 8) * 4);
    volatile uint32_t* cr = (pin < 8) ? &(port->CRL) : &(port->CRH);

    *cr &= ~(0xF << pin_position);
    *cr |= (0x3 << pin_position);  // Sortie push-pull, max 50MHz
    port->BSRR = (1 << pin);       // CS haut (inactif)
}

uint8_t SPI_Read(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg) {
    uint8_t data;

    // Sélectionner CS (actif bas)
    cs_port->BSRR = (1 << (cs_pin + 16));

    // Envoyer le registre pour lecture
    while (!(SPIx->SR & SPI_SR_TXE));
    SPIx->DR = reg | 0x80;
    while (!(SPIx->SR & SPI_SR_RXNE));
    (void)SPIx->DR;  // Lecture fictive

    // Recevoir les données
    while (!(SPIx->SR & SPI_SR_TXE));
    SPIx->DR = 0xFF;  // Écriture fictive
    while (!(SPIx->SR & SPI_SR_RXNE));
    data = SPIx->DR;

    // Désélectionner CS
    cs_port->BSRR = (1 << cs_pin);

    return data;
}

void SPI_ReadN(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg, uint8_t* buffer, uint16_t length) {
    // Ativar CS (CS low)
    cs_port->BSRR = (1 << (cs_pin + 16));

    // Enviar endereço com bit de leitura
    while (!(SPIx->SR & SPI_SR_TXE)); // Esperar que o buffer de transmissão esteja vazio
    SPIx->DR = reg | 0x80;            // Bit 7 alto para leitura
    while (!(SPIx->SR & SPI_SR_RXNE)); // Aguardar recepção
    (void)SPIx->DR;                   // Leitura descartada (dummy read)

    // Ler N bytes
    for (uint16_t i = 0; i < length; i++) {
        while (!(SPIx->SR & SPI_SR_TXE)); // Buffer de transmissão vazio
        SPIx->DR = 0xFF;                  // Dummy write para gerar clock
        while (!(SPIx->SR & SPI_SR_RXNE)); // Esperar dado recebido
        buffer[i] = SPIx->DR;             // Ler dado do buffer
    }

    while (SPIx->SR & SPI_SR_BSY); // Esperar fim da transmissão
    // Desativar CS (CS high)
    cs_port->BSRR = (1 << cs_pin);
}

void SPI_Write(SPI_TypeDef* SPIx, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint8_t reg, uint8_t data) {
    // Sélectionner CS (actif bas)
    cs_port->BSRR = (1 << (cs_pin + 16));

    // Envoyer le registre pour écriture
    while (!(SPIx->SR & SPI_SR_TXE));
    SPIx->DR = reg & 0x7F;
    while (!(SPIx->SR & SPI_SR_RXNE));
    (void)SPIx->DR;  // Lecture fictive

    // Envoyer les données
    while (!(SPIx->SR & SPI_SR_TXE));
    SPIx->DR = data;
    while (!(SPIx->SR & SPI_SR_RXNE));
    (void)SPIx->DR;  // Lecture fictive

    // Désélectionner CS
    cs_port->BSRR = (1 << cs_pin);
}

#endif // SPI_H