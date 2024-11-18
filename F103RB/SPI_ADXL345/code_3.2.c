#include "stm32f10x.h"

// Fonctions de configuration et envoi SPI
void init_gpioA(unsigned char num_bit, unsigned int quartet_config);
void init_SPI1(void);
void SPI1_Send(char data);
void DELAY(volatile unsigned int delay);

// Temporisation simple
void DELAY(volatile unsigned int delay)
{
    volatile unsigned int i;
    for(i = 0; i < (delay * 5400); i++) ;
}

// Initialisation des GPIO pour les broches SPI
void init_gpioA(unsigned char num_bit, unsigned int quartet_config) {
    unsigned char bit_ref = (num_bit * 4) & 31;

    // Activer l'horloge pour GPIOA : RCC->APB2ENR bit 2
    RCC->APB2ENR |= (1 << 2);

    // Limiter quartet_config a 4 bits
    quartet_config &= 0xF;

    if (num_bit < 8) {
        GPIOA->CRL &= ~(0xF << bit_ref);  // Effacer les anciens bits dans CRL
        GPIOA->CRL |= (quartet_config << bit_ref);  // Configurer les nouveaux bits
    } else {
        GPIOA->CRH &= ~(0xF << bit_ref);  // Effacer les anciens bits dans CRH
        GPIOA->CRH |= (quartet_config << bit_ref);  // Configurer les nouveaux bits
    }
}

// Initialisation du SPI1 (maitre)
void init_SPI1(void) {
    // Activer les horloges AFIO et SPI1
    RCC->APB2ENR |= (1 << 0);  // Horloge AFIO
    RCC->APB2ENR |= (1 << 12);  // Horloge SPI1

    // Configuration des broches SPI (PA4=SS, PA5=SCLK, PA6=MISO, PA7=MOSI)
    init_gpioA(4, 0b0011);  // PA4 (SS) : Output push-pull, max speed 50 MHz
    init_gpioA(5, 0b1011);  // PA5 (SCLK) : Alternate function output push-pull
    init_gpioA(6, 0b1000);  // PA6 (MISO) : Input avec pull-up
    GPIOA->ODR |= (1 << 6);  // Activer la pull-up sur PA6 (MISO)
    init_gpioA(7, 0b1011);  // PA7 (MOSI) : Alternate function output push-pull

    // Configurer SPI1 en mode maitre
    SPI1->CR1 = 0;  // Reinitialisation
    SPI1->CR1 |= (1 << 2);  // Selection du maitre (MSTR = 1)
    
    // Configurer la frequence d'horloge SPI : fpclk / 256 (BR = "111")
    SPI1->CR1 |= (7 << 3);  // BR[2:0] = 111, diviseur d'horloge par 256
    
    // CPOL = 0, CPHA = 0 : Front montant pour l'echantillonnage des donnees
    SPI1->CR1 &= ~((1 << 1) | (1 << 0));  // CPOL=0, CPHA=0
    
    // DFF = 0 : Communication en 8 bits
    SPI1->CR1 &= ~(1 << 11);  // DFF=0 pour 8 bits

    // Activer SS output enable (SSOE) dans CR2 pour contreler SS automatiquement
    SPI1->CR2 |= (1 << 2);

    // Activer SPI (SPE)
    SPI1->CR1 |= (1 << 6);  // SPE = 1
}

// Fonction pour envoyer une donnee via SPI1
void SPI1_Send(char data) {
    // Abaisser SS pour selectionner l'esclave
    GPIOA->ODR &= ~(1 << 4);

    // Attendre que le tampon soit vide (TXE)
    while (!(SPI1->SR & (1 << 1)));

    // Envoyer la donnee
    SPI1->DR = data;

    // Attendre que la transmission soit terminee (BSY)
    while (SPI1->SR & (1 << 7));

    // Remonter SS pour deselectionner l'esclave
    GPIOA->ODR |= (1 << 4);
}

int main(void) {
    // Initialiser SPI1
    init_SPI1();
    
    while (1) {
        DELAY(50);       // Attendre 50 ms
        SPI1_Send('D');  // Envoi 'D'
		DELAY(50);       // Attendre 50 ms
        SPI1_Send('F');  // Envoi 'F'
		DELAY(50);       // Attendre 50 ms
        SPI1_Send('L');  // Envoi 'L'
  
    }
}
