#include "usart_fifo.h"

struct usart_fifo
{
	volatile uint8_t rx_last_error;
	volatile uint8_t rx_buffer[USART_RX_BUFFER_SIZE];
	volatile uint8_t rx_buffer_head;
	volatile uint8_t rx_buffer_tail;
	//volatile uint8_t tx_buffer[USART_TX_BUFFER_SIZE];
	//volatile uint8_t tx_buffer_head;
	//volatile uint8_t tx_buffer_tail;
} usart1_fifo;


//------------------------------------------------------------------------------
uint8_t mcu_usart1_fifo_receive(uint8_t * data)
{
	uint8_t rx_head;
	uint8_t rx_tail;

	rx_head = usart1_fifo.rx_buffer_head;

	if (rx_head == usart1_fifo.rx_buffer_tail)
		return USART_FIFO_NO_DATA;

	rx_tail = (uint8_t)((usart1_fifo.rx_buffer_tail + 1) & (uint8_t)(
			USART_RX_BUFFER_SIZE - 1));

	usart1_fifo.rx_buffer_tail = rx_tail;

	*data = usart1_fifo.rx_buffer[rx_tail];

	return usart1_fifo.rx_last_error;
}

void handler_usart_rx()
{
	uint16_t status;
	uint8_t rx_data;
	uint8_t rx_last_error;
	uint8_t rx_head;

	status = USART1->SR;
	rx_data = USART1->DR;
	rx_last_error = 0;

	if (status & USART_SR_FE)
		rx_last_error |= USART_FIFO_ERROR_FRAME;

	if (status & USART_SR_ORE)
		rx_last_error |= USART_FIFO_ERROR_OVERRUN;

	if (status & USART_SR_NE)
		rx_last_error |= USART_FIFO_ERROR_NOISE;

	rx_head = (uint8_t)((usart1_fifo.rx_buffer_head + 1) & (uint8_t)(
			USART_RX_BUFFER_SIZE - 1));

	if (rx_head == usart1_fifo.rx_buffer_tail)
	{
		rx_last_error |= USART_FIFO_ERROR_BUFFER_OVERFLOW;
	}
	else
	{
		usart1_fifo.rx_buffer[rx_head] = rx_data;
		usart1_fifo.rx_buffer_head = rx_head;
	}

	usart1_fifo.rx_last_error = rx_last_error;
}

void mcu_usart1_init()
{
	usart1_fifo.rx_buffer_head = 0;
	usart1_fifo.rx_buffer_tail = 0;
	usart1_fifo.rx_last_error = 0;
	//usart1_fifo.tx_buffer_head = 0;
	//usart1_fifo.tx_buffer_tail = 0;
}
