#include "general.h"

void init_gpio(unsigned char gpio_id, unsigned char num_bit, unsigned int quartet_config)
{
    // Calculez la position du quartet de bits à configurer dans CRL ou CRH
    unsigned char bit_ref = (num_bit * 4) & 31;

    // Activer l'horloge pour GPIOX:
    switch (gpio_id)
    {
    case 'a':
        RCC->APB2ENR |= (1<<IOPAEN);// Bit  dans RCC->APB2ENR doit être mis à 1
        break;
    case 'b':
        RCC->APB2ENR |= (1<<IOPBEN);// Bit  dans RCC->APB2ENR doit être mis à 1
        break;
    case 'c':
         RCC->APB2ENR |= (1<<IOPCEN);// Bit  dans RCC->APB2ENR doit être mis à 1
        break;
    }

    // Limiter quartet_config à 4 bits
    quartet_config &= 0xF;

    switch (gpio_id)
    {
    case 'a':
        init_gpioA(num_bit, quartet_config);
        break;
    case 'b':
        init_gpioB(num_bit, quartet_config);
        break;
    case 'c':
        init_gpioC(num_bit, quartet_config);
        break;
    default:
        break;
    }
}
void init_gpioA(unsigned char num_bit, unsigned int quartet_config) {
    // Limiter quartet_config à 4 bits
    quartet_config &= 0xF;
    // Calculez la position du quartet de bits à configurer dans CRL ou CRH
    unsigned char bit_ref = (num_bit * 4) & 31;
    // Configurer le registre CRL si le numéro de bit est inférieur à 8
    if (num_bit < 8) {
        // Effacer les anciens bits du quartet correspondant dans CRL
        GPIOA->CRL &= ~(0xF << bit_ref);
        // Configurer les nouveaux bits du quartet dans CRL
        GPIOA->CRL |= (quartet_config << bit_ref);
    } else {
        // Effacer les anciens bits du quartet correspondant dans CRH
        GPIOA->CRH &= ~(0xF << bit_ref);
        // Configurer les nouveaux bits du quartet dans CRH
        GPIOA->CRH |= (quartet_config << bit_ref);
    }
}
void init_gpioB(unsigned char num_bit, unsigned int quartet_config) {
    // Calculez la position du quartet de bits à configurer dans CRL ou CRH
    unsigned char bit_ref = (num_bit * 4) & 31;
    // Configurer le registre CRL si le numéro de bit est inférieur à 8
    if (num_bit < 8) {
        // Effacer les anciens bits du quartet correspondant dans CRL
        GPIOB->CRL &= ~(0xF << bit_ref);
        // Configurer les nouveaux bits du quartet dans CRL
        GPIOB->CRL |= (quartet_config << bit_ref);
    } else {
        // Effacer les anciens bits du quartet correspondant dans CRH
        GPIOB->CRH &= ~(0xF << bit_ref);
        // Configurer les nouveaux bits du quartet dans CRH
        GPIOB->CRH |= (quartet_config << bit_ref);
    }
}
void init_gpioC(unsigned char num_bit, unsigned int quartet_config) {
    // Calculez la position du quartet de bits à configurer dans CRL ou CRH
    unsigned char bit_ref = (num_bit * 4) & 31;
    // Configurer le registre CRL si le numéro de bit est inférieur à 8
    if (num_bit < 8) {
        // Effacer les anciens bits du quartet correspondant dans CRL
        GPIOC->CRL &= ~(0xF << bit_ref);
        // Configurer les nouveaux bits du quartet dans CRL
        GPIOC->CRL |= (quartet_config << bit_ref);
    } else {
        // Effacer les anciens bits du quartet correspondant dans CRH
        GPIOC->CRH &= ~(0xF << bit_ref);
        // Configurer les nouveaux bits du quartet dans CRH
        GPIOC->CRH |= (quartet_config << bit_ref);
    }
}


//=================================================================================================
static void delay_ms(int ms) {
    for(int i = 0; i < ms * 1000; i++)
        __NOP();
}
static void delay_us(int us) {
    for(int i = 0; i < us; i++)
        __NOP();
}
static void delay_s(int s) {
    for(int i = 0; i < s * 1000000; i++)
        __NOP();
}