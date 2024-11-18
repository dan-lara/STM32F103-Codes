#ifndef UART_H
#define UART_H

#include "general.h"

// Structure pour la gestion du buffer circulaire
typedef struct {
    uint8_t* buffer;           // Buffer de données
    uint16_t size;             // Taille totale du buffer
    uint16_t writePtr;         // Pointeur d'écriture
    uint16_t readPtr;          // Pointeur de lecture
    uint16_t available;        // Espace disponible
} UART_FIFO_t;

// Codes d'état pour les opérations UART
typedef enum {
    UART_OK = 0,
    UART_ERROR,
    UART_BUSY,
    UART_BUFFER_FULL,
    UART_BUFFER_EMPTY
} UART_Status_t;

// Prototypes des fonctions
UART_Status_t USART2_init(uint32_t baudRate);
UART_Status_t USART2_SendChar(char c);
UART_Status_t USART2_SendString(const char* str);
UART_Status_t USART2_SendHexString(uint8_t value);
UART_Status_t USART2_ProcessCharFIFO();
UART_Status_t USART2_ProcessHexFIFO();
UART_Status_t USART2_WriteFIFO(uint8_t data);

// Définitions privées
#define UART_TXE_FLAG     (1<<7)  // Bit 7 de SR (TXE)
#define DEFAULT_FIFO_SIZE 64      // Taille par défaut du buffer

// Variables statiques pour la FIFO
static UART_FIFO_t uartFifo;
static uint8_t fifoBuffer[DEFAULT_FIFO_SIZE];

UART_Status_t USART2_init(uint32_t baudRate) {
    RCC->APB2ENR |= (1 << 2);  // GPIOA clock enable
    RCC->APB1ENR |= (1 << 17); // USART2 clock enable
		
    init_gpio('a', 2, GPIO_AF_PP_50MHZ);//0xB
	
    init_gpio('a', 3, GPIO_INPUT_FLOATING);//0x4
    uint32_t brrValue = 36000000 / baudRate;
    USART2->BRR = brrValue; // Baudrate 9600, APB1 clock 36 MHz
    USART2->CR1 |= (1 << 2) | (1 << 3); // Enable receiver and transmitter
    USART2->CR1 |= (1 << 13); // Enable USART2

    // Initialisation de la FIFO
    uartFifo.buffer = fifoBuffer;
    uartFifo.size = DEFAULT_FIFO_SIZE;
    uartFifo.writePtr = 0;
    uartFifo.readPtr = 0;
    uartFifo.available = DEFAULT_FIFO_SIZE;

    return UART_OK;
}

UART_Status_t USART2_SendChar(char c) {
    // Attendre que le registre d'émission soit vide
    while (!(USART2->SR & UART_TXE_FLAG));
    USART2->DR = c;
    return UART_OK;
}

UART_Status_t USART2_SendString(const char* str) {
    while (*str)
        if (USART2_SendChar(*str++) != UART_OK) 
            return UART_ERROR;
    return UART_OK;
}

UART_Status_t USART2_SendHexString(uint8_t data) {
    char hexStr[3];
    sprintf(hexStr, "%02X", data);
    return USART2_SendString(hexStr);
}

UART_Status_t USART2_ProcessCharFIFO() {
    // Vérifier s'il y a des données à envoyer
    if (uartFifo.readPtr != uartFifo.writePtr) {
        USART2_SendChar(uartFifo.buffer[uartFifo.readPtr]);
        uartFifo.readPtr = (uartFifo.readPtr + 1) % uartFifo.size;
        uartFifo.available++;
        return UART_OK;
    }
    return UART_BUFFER_EMPTY;
}

UART_Status_t USART2_ProcessHexFIFO() {
    // Vérifier s'il y a des données à envoyer
    if (uartFifo.readPtr != uartFifo.writePtr) {
        USART2_SendHexString(uartFifo.buffer[uartFifo.readPtr]);
        uartFifo.readPtr = (uartFifo.readPtr + 1) % uartFifo.size;
        uartFifo.available++;
        return UART_OK;
    }
    return UART_BUFFER_EMPTY;
}

UART_Status_t USART2_WriteFIFO(uint8_t data) {
    if (uartFifo.available) {
        uartFifo.buffer[uartFifo.writePtr] = data;
        uartFifo.writePtr = (uartFifo.writePtr + 1) % DEFAULT_FIFO_SIZE;  // Rebouclage du pointeur d'écriture
        uartFifo.available--;
        return UART_OK;
    }
    return UART_BUFFER_FULL;
}


#endif // UART_H