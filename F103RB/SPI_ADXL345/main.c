#include "stm32f10x.h"
#include <stdio.h>
#include "general.h"
#include "uart.h"
#include "spi.h"
#include "adxl345.h"

#define ADXL345_POWER_CTL   0x2D  // Adresse du registre pour le contrôle de l'alimentation
#define ADXL345_DATA_FORMAT 0x31  // Adresse du registre pour le format des données
#define ADXL345_DEVID       0x00  // Adresse du registre pour l'ID de l'appareil
#define ADXL345_CS_PIN      4
#define ADXL345_DATAX0      0x32  // Adresse du registre pour les données de l'axe X

#define DEBUG 0
#define ID_UNIQUE 0x2A
#define CODE_SESSION 0x0F
#define CODE_CAPTEUR 0x01

// Préparer et envoyer le paquet de données
void send_data_packet(int16_t x, int16_t y, int16_t z) {
    if (DEBUG){
        x = -1000; y = 482; z = 1000;
    }
    uint8_t packet[12] = {
        0x7E,
        ID_UNIQUE, CODE_SESSION, CODE_CAPTEUR,
        (uint8_t)x, (uint8_t)(x >> 8), 
        (uint8_t)y, (uint8_t)(y >> 8),
        (uint8_t)z, (uint8_t)(z >> 8),
        0x00, // Checksum
        0x7F
    };
    uint16_t checksum = 0;
    for (int i = 0; i < 10; i++)
        checksum += packet[i];
    packet[10] = checksum % 256;

    for (int i = 0; i < 12; i++) 
            USART2_WriteFIFO(packet[i]);
        
        if (DEBUG){
            USART2_SendString("\nData: ");
            while (USART2_ProcessHexFIFO() == UART_OK)
                    USART2_SendChar(' ');
            
            for (int i = 0; i < 12; i++)
                    USART2_WriteFIFO(packet[i]);

            USART2_SendString("\nData: ");		
        }
        while(USART2_ProcessCharFIFO() == UART_OK);
}

/**
 * @brief Fonction principale qui initialise les périphériques et lit les données de l'accéléromètre ADXL345.
 *
 * Cette fonction exécute les étapes suivantes :
 * 1. initialisation de USART2 avec une vitesse de transmission de 9600 * 2. initialisation de SPI1 avec un diviseur d'horloge de 16.
 * Initialise SPI1 avec un diviseur d'horloge de 16, mode 1, et un format de données de 8 bits.
 * Configure la broche de sélection de puce (CS) pour la communication SPI.
 * Initialise l'accéléromètre ADXL345 avec la broche SPI et CS spécifiée.
 * Si le débogage est activé, il teste la communication avec l'accéléromètre ADXL345 et vérifie l'identification de l'appareil.
 * Si l'ID de l'appareil correspond, envoie l'ID de l'appareil sur USART2.
 * 7. Lit tous les registres de l'ADXL345 et envoie les données sur USART2.
 * 8. Entre dans une boucle infinie où il lit les données d'accélération de l'ADXL345 et les envoie comme un paquet de données.
 * 9. Si le débogage est activé, entre dans une boucle infinie après avoir lu les données d'accélération.
 * 10. Délai de 1000 millisecondes avant la prochaine lecture.
 */
// Fonction principale
int main(void) {
    USART2_init(9600);
    int16_t x, y, z;
    SPI1_Init(SPI_CLK_DIV_16,1,1,8);
    SPI_ConfigCS(GPIOA, 4);
    
    ADXL345_Init(SPI1, GPIOA, 4);

    if (DEBUG){
        ADXL345_TestCommunication(SPI1,GPIOA, 4);
        
        uint8_t device_id = ADXL345_GetID(SPI1, GPIOA, 4);
        
        if (device_id == 0xE5) {
            USART2_SendString("Device ID: 0xE5\n");            
        } else {
            USART2_SendString("Error: ADXL345 device ID mismatch. ID: ");
            USART2_SendHexString(device_id);
            USART2_SendString("\n");
        }
        ADXL345_Data data = ADXL345_ReadAllRegisters(SPI1, GPIOA, 4);
        ADXL345_SendDataUSART(data);
    }
    
    while (1) {
        ADXL345_ReadAcceleration(SPI1, GPIOA, 4, &x, &y, &z);
        send_data_packet(x, y, z);
        if (DEBUG)
            while(1);
        delay_ms(1000);
    }
}


//Code hérité -> Utilisé pour les tests et l'implémentation des bibliothèques
/*
#define BUFFER_SIZE 64  // Taille du tampon circulaire

volatile uint32_t head = 0;  // Index d'écriture pour le tampon circulaire
volatile uint32_t tail = 0;  // Index de lecture pour le tampon circulaire
volatile char buffer[BUFFER_SIZE];  // Tampon circulaire pour la transmission USART

void init_gpios(void) ;

// Initialiser GPIOA pour SPI1
void init_gpios(void) {
        init_gpio('a', 4, GPIO_OUT_PP_50MHZ);//0x3

        init_gpio('a', 5, GPIO_AF_PP_50MHZ);//0xB
    
        init_gpio('a', 6, GPIO_INPUT_FLOATING);//0x4
    
        init_gpio('a', 7, GPIO_AF_PP_50MHZ);//0xB
}

// Initialiser SPI1
void init_SPI1(void) {
    RCC->APB2ENR |= (1 << 12);

    SPI1->CR1 = 0;
    SPI1->CR1 |= (1 << 2);
    SPI1->CR1 |= (3 << 3);
    SPI1->CR1 |= (1 << 1);
    SPI1->CR1 |= (1 << 0);
    SPI1->CR1 |= (1 << 9);
    SPI1->CR1 |= (1 << 8);

    SPI1->CR2 = 0;
    SPI1->CR2 |= (1 << 2);

    SPI1->CR1 |= (1 << 6);
}

// Écriture SPI
void SPI_Write(uint8_t reg, uint8_t data) {
    GPIOA->BSRR = (1 << (ADXL345_CS_PIN + 16));
    while (!(SPI1->SR & (1 << 1)));
    SPI1->DR = reg;
    while (!(SPI1->SR & (1 << 0)));
    (void)SPI1->DR;

    while (!(SPI1->SR & (1 << 1)));
    SPI1->DR = data;
    while (!(SPI1->SR & (1 << 0)));
    (void)SPI1->DR;

    GPIOA->BSRR = (1 << ADXL345_CS_PIN);
}

// Lecture SPI
uint8_t SPI_Read(SPI1, GPIOA, 4,uint8_t reg) {
    uint8_t receivedData;

    GPIOA->BSRR = (1 << (ADXL345_CS_PIN + 16));
    while (!(SPI1->SR & (1 << 1)));
    SPI1->DR = reg | 0x80;
    while (!(SPI1->SR & (1 << 0)));
    (void)SPI1->DR;

    while (!(SPI1->SR & (1 << 1)));
    SPI1->DR = 0x00;
    while (!(SPI1->SR & (1 << 0)));
    receivedData = SPI1->DR;

    GPIOA->BSRR = (1 << ADXL345_CS_PIN);

    return receivedData;
}

// Initialiser ADXL345
void init_ADXL345(void) {
    SPI_Write(SPI1, GPIOA, 4,ADXL345_POWER_CTL, 0x08);
    SPI_Write(SPI1, GPIOA, 4,ADXL345_DATA_FORMAT, 0x00);
}

// Obtenir l'ID de l'appareil ADXL345
uint8_t get_ADXL345_ID(void) {
    return SPI_Read(SPI1, GPIOA, 4,ADXL345_DEVID);
}

// Lire les données d'accélération
void read_acceleration(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t x0 = SPI_Read(SPI1, GPIOA, 4,ADXL345_DATAX0);
    uint8_t x1 = SPI_Read(SPI1, GPIOA, 4,ADXL345_DATAX0 + 1);
    uint8_t y0 = SPI_Read(SPI1, GPIOA, 4,ADXL345_DATAX0 + 2);
    uint8_t y1 = SPI_Read(SPI1, GPIOA, 4,ADXL345_DATAX0 + 3);
    uint8_t z0 = SPI_Read(SPI1, GPIOA, 4,ADXL345_DATAX0 + 4);
    uint8_t z1 = SPI_Read(SPI1, GPIOA, 4,ADXL345_DATAX0 + 5);

    *x = ((int16_t)x1 << 8) | x0;
    *y = ((int16_t)y1 << 8) | y0;
    *z = ((int16_t)z1 << 8) | z0;
}

// Ajouter des données au tampon circulaire
void add_to_buffer(char c) {
    if ((head + 1) % BUFFER_SIZE != tail) {
        buffer[head] = c;
        head = (head + 1) % BUFFER_SIZE;
    }
}

// Transmettre les données du tampon circulaire via USART
void USART2_TransmitBuffer(void) {
        
    while (tail != head) {
        USART2_SendChar(buffer[tail]);
        tail = (tail + 1) % BUFFER_SIZE;
                
    }
        
}

// Transmettre les données du tampon circulaire via USART
void USART2_TransmitBuffer2(void) {
    while (tail != head) {
        USART2_SendChar(buffer[tail]);
                USART2_SendChar(' ');
        tail = (tail + 1) % BUFFER_SIZE;
    }

}

// Préparer et envoyer le paquet de données
void send_data_packet(int16_t x, int16_t y, int16_t z) {
        //x = -1000; y = 482; z = 1000;
    uint8_t packet[12] = {
        0x7E, 0x2A, 0x0F, 0x01,
        (uint8_t)x, (uint8_t)(x >> 8), 
        (uint8_t)y, (uint8_t)(y >> 8),
        (uint8_t)z, (uint8_t)(z >> 8),
        0x00, 0x7F
    };

    uint16_t checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += packet[i];
    }
    packet[10] = checksum % 256;


    for (int i = 0; i < 12; i++) {
        add_to_buffer(packet[i]);	
    }
        while (tail != head) {
            USART2_SendChar(buffer[tail]);
            tail = (tail + 1) % BUFFER_SIZE;
        }
}
// Fonction principale
int main(void) {
    init_USART2();
        init_gpios();
    init_SPI1();   
        init_ADXL345();
    uint8_t device_id = get_ADXL345_ID();
        
    if (device_id == 0xE5) {
        USART2_SendString("Device ID: 0xE5\n");
    } else {
        USART2_SendString("Error: ADXL345 device ID mismatch.\n");
    }
        USART2_SendString("\n");
    int16_t x, y, z;
    while (1) {
        read_acceleration(SPI1, GPIOA, 4, &x, &y, &z);
        send_data_packet(x, y, z);
                while(1);
        delay_ms(6000000);
    }
}
*/