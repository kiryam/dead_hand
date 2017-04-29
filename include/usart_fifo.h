#ifndef USART_FIFO_H
#define USART_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f2xx_conf.h"


#define USART_TX_BUFFER_SIZE    8
#define USART_RX_BUFFER_SIZE    8

enum usart_fifo_error
{
    USART_FIFO_ERROR_FRAME = 0x1,
    USART_FIFO_ERROR_OVERRUN = 0x2,
    USART_FIFO_ERROR_NOISE = 0x4,
    USART_FIFO_ERROR_BUFFER_OVERFLOW = 0x8,
    USART_FIFO_NO_DATA = 0x16
};

void mcu_usart_init();
uint8_t mcu_usart1_fifo_receive(uint8_t * data);

#ifdef __cplusplus
}
#endif

#endif
