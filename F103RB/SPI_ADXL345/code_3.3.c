#include "stm32f10x.h"
#include <stdio.h>

#define ADXL345_POWER_CTL   0x2D  // Adresse du registre pour le contrôle de l'alimentation
#define ADXL345_DATA_FORMAT 0x31  // Adresse du registre pour le format des données
#define ADXL345_DEVID       0x00  // Adresse du registre pour l'ID de l'appareil
#define ADXL345_CS_PIN      4

typedef struct {
    uint8_t devid;          // 0x00 - ID de l'appareil
    uint8_t power_ctl;      // 0x2D - Contrôle de l'alimentation
    uint8_t data_format;    // 0x31 - Format des données
    uint8_t bw_rate;        // 0x2C - Taux de bande passante
    uint8_t fifo_ctl;       // 0x38 - Contrôle FIFO
    uint8_t int_source;     // 0x30 - Source d'interruption
    int16_t x_data;         // 0x32 (LSB) & 0x33 (MSB) - Données de l'axe X
    int16_t y_data;         // 0x34 (LSB) & 0x35 (MSB) - Données de l'axe Y
    int16_t z_data;         // 0x36 (LSB) & 0x37 (MSB) - Données de l'axe Z
} ADXL345_Data;

void init_USART2(void) {
    RCC->APB2ENR |= (1 << APB2ENR_IOPAEN);  // Activer l'horloge GPIOA
    RCC->APB1ENR |= (1 << 17);

    // Configurer PA2 comme sortie (TX)
    GPIOA->CRL &= ~(0xF << 8);
    GPIOA->CRL |= (0xB << 8);

    // Configurer PA3 comme entrée (RX)
    GPIOA->CRL &= ~(0xF << 12);
    GPIOA->CRL |= (0x4 << 12);

    // Configurer USART2
    USART2->BRR = 3750; // Baudrate 9600, APB1 à 36 MHz
    USART2->CR1 |= (1 << 2) | (1 << 3); // Activer RX et TX
    USART2->CR1 |= (1 << 13); // Activer USART2
}

void USART2_SendChar(char c) {
    while (!(USART2->SR & (1 << 7))); // Attendre que le registre de données soit vide
    USART2->DR = c;
}

void USART2_SendString(char* str) {
    while (*str) {
        USART2_SendChar(*str++);
    }
}

void init_gpioA(void) {
    // Activer l'horloge GPIOA
    RCC->APB2ENR |= (1 << 2);  // Activer l'horloge GPIOA
    
    // Configurer chaque broche :
    // PA4 (CS) - Sortie push-pull
    GPIOA->CRL &= ~(0xF << 16);
    GPIOA->CRL |= (0x3 << 16);  // Mode sortie 50MHz
    
    // PA5 (SCK) - Fonction alternative push-pull
    GPIOA->CRL &= ~(0xF << 20);
    GPIOA->CRL |= (0xB << 20);  // Fonction alternative sortie 50MHz
    
    // PA6 (MISO) - Entrée flottante/Entrée pull-up
    GPIOA->CRL &= ~(0xF << 24);
    GPIOA->CRL |= (0x4 << 24);  // Entrée flottante
    
    // PA7 (MOSI) - Fonction alternative push-pull
    GPIOA->CRL &= ~(0xF << 28);
    GPIOA->CRL |= (0xB << 28);  // Fonction alternative sortie 50MHz
    
    // Mettre la broche CS à l'état haut initialement (état de repos)
    GPIOA->BSRR = (1 << 4);
}

void init_SPI1(void) {
    // Activer l'horloge SPI1
    RCC->APB2ENR |= (1 << 12);

    // Configurer le registre de contrôle 1 de SPI1 (SPI_CR1)
    SPI1->CR1 = 0;                  // Effacer le registre SPI_CR1
    SPI1->CR1 |= (1 << 2);          // Régler le bit MSTR, sélectionner le mode maître
    SPI1->CR1 |= (3 << 3);          // Régler le taux de baud à fPCLK/16 (fréquence moyenne)
    SPI1->CR1 |= (1 << 1);          // Régler CPOL = 1, l'horloge est haute au repos
    SPI1->CR1 |= (1 << 0);          // Régler CPHA = 1, les données sont échantillonnées sur le front montant
    SPI1->CR1 |= (1 << 9);          // Régler SSM = 1, gérer le signal CS par logiciel
    SPI1->CR1 |= (1 << 8);          // Régler SSI = 1, signal CS haut

    // Configurer le registre de contrôle 2 de SPI1 (SPI_CR2)
    SPI1->CR2 = 0;
    SPI1->CR2 |= (1 << 2);          // Régler SSOE = 1, gérer automatiquement le signal CS

    // Activer le périphérique SPI1
    SPI1->CR1 |= (1 << 6);
}

void SPI_Write(uint8_t reg, uint8_t data) {
    // Tirer CS bas pour initier la communication
    GPIOA->BSRR = (1 << (ADXL345_CS_PIN + 16));

    // Envoyer l'adresse du registre
    while (!(SPI1->SR & (1 << 1))); // Attendre que TXE soit réglé
    SPI1->DR = reg;

    // Attendre la transmission des données
    while (!(SPI1->SR & (1 << 0))); // Attendre que RXNE soit réglé
    (void)SPI1->DR; // Lire pour effacer RXNE

    // Envoyer les données
    while (!(SPI1->SR & (1 << 1))); // Attendre que TXE soit réglé
    SPI1->DR = data;

    // Attendre la transmission des données
    while (!(SPI1->SR & (1 << 0))); // Attendre que RXNE soit réglé
    (void)SPI1->DR; // Lire pour effacer RXNE

    // Tirer CS haut pour terminer la communication
    GPIOA->BSRR = (1 << ADXL345_CS_PIN);
}

uint8_t SPI_Read(uint8_t reg) {
    uint8_t receivedData;

    // Tirer CS bas pour initier la communication
    GPIOA->BSRR = (1 << (ADXL345_CS_PIN + 16));

    // Envoyer l'adresse du registre avec le bit de lecture (bit 7 réglé à 1)
    while (!(SPI1->SR & (1 << 1))); // Attendre que TXE soit réglé
    SPI1->DR = reg | 0x80;

    // Attendre la transmission des données
    while (!(SPI1->SR & (1 << 0))); // Attendre que RXNE soit réglé
    (void)SPI1->DR; // Lire pour effacer RXNE

    // Envoyer un octet factice pour recevoir des données
    while (!(SPI1->SR & (1 << 1))); // Attendre que TXE soit réglé
    SPI1->DR = 0x00;

    // Attendre les données reçues
    while (!(SPI1->SR & (1 << 0))); // Attendre que RXNE soit réglé
    receivedData = SPI1->DR;

    // Tirer CS haut pour terminer la communication
    GPIOA->BSRR = (1 << ADXL345_CS_PIN);

    return receivedData;
}

static void delay_ms(int ms) {
    for(int i = 0; i < ms * 1000; i++) {
        __NOP();
    }
}

uint8_t init_ADXL345(void) {
    // Attendre la mise sous tension
    delay_ms(10);
    
    // Mettre CS haut initialement
    GPIOA->BSRR = (1 << ADXL345_CS_PIN);
    delay_ms(1);
    
    // Lire et vérifier l'ID de l'appareil
    uint8_t deviceId = SPI_Read(ADXL345_DEVID);
    if (deviceId != 0xE5) {  // ADXL345 devrait retourner 0xE5 comme ID de l'appareil
        return 0;  // Échec de l'initialisation
    }
    
    // Réinitialiser tous les registres
    SPI_Write(0x2D, 0x00);  // Effacer POWER_CTL
    delay_ms(1);
    
    // Configurer le format des données (avant d'activer la mesure)
    // Régler la plage à ±2g (0x00) ou ±16g (0x0B) et le mode SPI 4 fils
    SPI_Write(ADXL345_DATA_FORMAT, 0x00);
    delay_ms(1);
    
    // Configurer le registre POWER_CTL
    // Régler le bit 3 (bit de mesure) à 1 pour le mode de mesure
    // Les autres bits aux valeurs par défaut
    SPI_Write(ADXL345_POWER_CTL, 0x08);
    delay_ms(1);
    
    // Régler le taux de données à 100Hz (par défaut)
    SPI_Write(0x2C, 0x0A);
    delay_ms(1);
    
    // Vérifier si le mode de mesure a été activé
    uint8_t powerCtl = SPI_Read(ADXL345_POWER_CTL);
    if ((powerCtl & 0x08) != 0x08) {
        return 0;  // Échec de l'initialisation
    }
    
    return 1;  // Initialisation réussie
}

void test_ADXL345_communication(void) {
    uint8_t deviceId = SPI_Read(ADXL345_DEVID);
    char buffer[50];
    
    sprintf(buffer, "Device ID lu: 0x%02X\r\n", deviceId);
    USART2_SendString(buffer);
    
    if (deviceId == 0xE5) {
        USART2_SendString("ADXL345 trouvé et réagit correctement!\r\n");
    } else {
        USART2_SendString("ERREUR: ADXL345 ne répond pas correctement!\r\n");
    }
}

uint8_t get_ADXL345_ID(void) {
    return SPI_Read(ADXL345_DEVID);
}

ADXL345_Data ADXL345_ReadAllRegisters(void) {
    ADXL345_Data data;
    
    // Lire les registres de configuration
    data.devid = SPI_Read(0x00);
    data.power_ctl = SPI_Read(0x2D);
    data.data_format = SPI_Read(0x31);
    data.bw_rate = SPI_Read(0x2C);
    data.fifo_ctl = SPI_Read(0x38);
    data.int_source = SPI_Read(0x30);
    
    // Lire les données d'accélération
    uint8_t x_l = SPI_Read(0x32);
    uint8_t x_h = SPI_Read(0x33);
    uint8_t y_l = SPI_Read(0x34);
    uint8_t y_h = SPI_Read(0x35);
    uint8_t z_l = SPI_Read(0x36);
    uint8_t z_h = SPI_Read(0x37);
    
    // Combiner les octets pour former des valeurs de 16 bits
    data.x_data = (int16_t)((x_h << 8) | x_l);
    data.y_data = (int16_t)((y_h << 8) | y_l);
    data.z_data = (int16_t)((z_h << 8) | z_l);
    
    return data;
}

void ADXL345_SendDataUSART(ADXL345_Data data) {
    char buffer[100];
    
    // Envoyer l'en-tête
    USART2_SendString("\r\n=== Données des registres ADXL345 ===\r\n");
    
    // Envoyer les données des registres de configuration
    sprintf(buffer, "Device ID: 0x%02X\r\n", data.devid);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Contrôle de l'alimentation: 0x%02X\r\n", data.power_ctl);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Format des données: 0x%02X\r\n", data.data_format);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Taux de bande passante: 0x%02X\r\n", data.bw_rate);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Contrôle FIFO: 0x%02X\r\n", data.fifo_ctl);
    USART2_SendString(buffer);
    
    sprintf(buffer, "Source d'interruption: 0x%02X\r\n", data.int_source);
    USART2_SendString(buffer);
    
    // Envoyer les données d'accélération
    sprintf(buffer, "Données d'accélération:\r\n");
    USART2_SendString(buffer);
    sprintf(buffer, "X: %d\r\n", data.x_data);
    USART2_SendString(buffer);
    sprintf(buffer, "Y: %d\r\n", data.y_data);
    USART2_SendString(buffer);
    sprintf(buffer, "Z: %d\r\n", data.z_data);
    USART2_SendString(buffer);
    
    USART2_SendString("===========================\r\n");
}

int main(void) {
    init_USART2();

    // Initialiser SPI et GPIO pour la communication
    init_SPI1();
    init_gpioA();
    // Initialiser ADXL345
    char resp = init_ADXL345();
    if (resp == 1)
        USART2_SendString("Initialisation réussie \n");
    else {
        USART2_SendString("Capteur non initialisé \n");
        //while(1);
    }
    test_ADXL345_communication();
    
    // Récupérer et vérifier l'ID de l'appareil ADXL345
    uint8_t device_id = get_ADXL345_ID();
    char buffer[20];
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = "0123456789ABCDEF"[(device_id >> 4) & 0x0F]; // High nibble
    buffer[3] = "0123456789ABCDEF"[device_id & 0x0F];      // Low nibble
    buffer[4] = '\n'; // Newline
    buffer[5] = '\0';
    
    if (device_id == 0xE5) {
        USART2_SendString("Device ID: ");
        USART2_SendString(buffer);
    } else {
        USART2_SendString("Erreur: ID de l'appareil ADXL345 incompatible.\n ID trouvé: ");
        USART2_SendString(buffer);
    }
    
    ADXL345_Data sensor_data = ADXL345_ReadAllRegisters();
    ADXL345_SendDataUSART(sensor_data);
    while (1) {
        // Boucle principale
    }
}
