#ifndef GENERAL_H
#define GENERAL_H
#include <stm32f10x.h>
#include <stdio.h>

// Definitions
#define IOPAEN 2 
#define IOPBEN 3 
#define IOPCEN 4

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define GPIO_INPUT_ANALOG    0x0
#define GPIO_INPUT_FLOATING  0x4
#define GPIO_INPUT_PULL      0x8
#define GPIO_OUT_PP_10MHZ    0x1
#define GPIO_OUT_PP_2MHZ     0x2
#define GPIO_OUT_PP_50MHZ    0x3
#define GPIO_OUT_OD_10MHZ    0x5
#define GPIO_OUT_OD_2MHZ     0x6
#define GPIO_OUT_OD_50MHZ    0x7
#define GPIO_AF_PP_10MHZ     0x9
#define GPIO_AF_PP_2MHZ      0xA
#define GPIO_AF_PP_50MHZ     0xB
#define GPIO_AF_OD_10MHZ     0xD
#define GPIO_AF_OD_2MHZ      0xE
#define GPIO_AF_OD_50MHZ     0xF

//==================GPIO Fonctions==================
void init_gpio(unsigned char gpio_id, unsigned char num_bit, unsigned int quartet_config);
void init_gpioA(unsigned char num_bit, unsigned int quartet_config);
void init_gpioB(unsigned char num_bit, unsigned int quartet_config);
void init_gpioC(unsigned char num_bit, unsigned int quartet_config);

//==================Temporisation==================
static void delay_ms(int ms);
static void delay_us(int us);
static void delay_s(int s);

#endif // GENERAL_H