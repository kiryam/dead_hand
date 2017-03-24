/**
 * original author:  Tilen Majerle<tilen@majerle.eu>
 * modification for STM32f10x: Alexander Lutsai<s.lyra@ya.ru>

   ----------------------------------------------------------------------
   	Copyright (C) Alexander Lutsai, 2016
    Copyright (C) Tilen Majerle, 2015

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
 */
#include "stm32f2xx.h"
#include "ssd1306_i2c.h"

/* Private defines */
#define I2C_TRANSMITTER_MODE   0
#define I2C_RECEIVER_MODE      1
#define I2C_ACK_ENABLE         1
#define I2C_ACK_DISABLE        0

void ssd1306_I2C_Init() {
	GPIO_InitTypeDef gpio;
	I2C_InitTypeDef i2c;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    i2c.I2C_ClockSpeed = 400000;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = 0x15;
    i2c.I2C_Ack = I2C_Ack_Disable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &i2c);

    GPIO_PinAFConfig(GPIOB, 8, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, 9, GPIO_AF_I2C1);

    gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpio);

    I2C_Cmd(I2C1, ENABLE);
}

void ssd1306_I2C_WriteMulti(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
	uint8_t i;
	ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE);
	ssd1306_I2C_WriteData(I2Cx, reg);
	for (i = 0; i < count; i++) {
		ssd1306_I2C_WriteData(I2Cx, data[i]);

	}
	ssd1306_I2C_Stop(I2Cx);
}

/*******************************************************************/
int16_t ssd1306_I2C_Start(I2C_TypeDef* I2Cx, uint8_t slaveAddress, uint8_t transmissionDirection )
{
    // На всякий слуыай ждем, пока шина осовободится
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    // Генерируем старт - тут все понятно )
    I2C_GenerateSTART(I2Cx, ENABLE);
    // Ждем пока взлетит нужный флаг
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
    // Посылаем адрес подчиненному  //возможно тут нужен сдвиг влево  //судя по исходникам - да, нужен сдвиг влево
    //http://microtechnics.ru/stm32-ispolzovanie-i2c/#comment-8109
    I2Cx->DR = slaveAddress & ~I2C_OAR1_ADD0;
    // А теперь у нас два варианта развития событий - в зависимости от выбранного направления обмена данными
    if(transmissionDirection== I2C_Direction_Transmitter) {
    	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    }
    if(transmissionDirection== I2C_Direction_Receiver) {
    	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }

    return 0;
}

/*******************************************************************/
void ssd1306_I2C_WriteData(I2C_TypeDef* I2Cx, uint8_t data)
{
    // Просто вызываем готоваую функцию из SPL и ждем, пока данные улетят
    I2C_SendData(I2Cx, data);
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

void ssd1306_I2C_Write(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t data) {
	ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE);
	ssd1306_I2C_WriteData(I2Cx, reg);
	ssd1306_I2C_WriteData(I2Cx, data);
	ssd1306_I2C_Stop(I2Cx);
}


uint8_t ssd1306_I2C_Stop(I2C_TypeDef* I2Cx) {
	/* Wait till transmitter not empty */
	//while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE));
	//sleepMs(20);

	/* Generate stop */
	I2Cx->CR1 |= I2C_CR1_STOP;
	return 0;
}

uint8_t ssd1306_I2C_IsDeviceConnected(I2C_TypeDef* I2Cx, uint8_t address) {
	uint8_t connected = 0;
	/* Try to start, function will return 0 in case device will send ACK */
	if (!ssd1306_I2C_Start(I2Cx, address, I2C_TRANSMITTER_MODE)) {
		connected = 1;
	}

	/* STOP I2C */
	ssd1306_I2C_Stop(I2Cx);

	/* Return status */
	return connected;
}
