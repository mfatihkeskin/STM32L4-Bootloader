/*********************************************************************
 |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 ||||||||||||||||||->file    : App1Main.c 			||||||||||||||||||
 ||||||||||||||||||->author  : M.Fatih KESKİN		||||||||||||||||||
 ||||||||||||||||||->version : v.1.0 				||||||||||||||||||
 ||||||||||||||||||->date    : 29.07.22				||||||||||||||||||
 ||||||||||||||||||->brief   : main		 			||||||||||||||||||
 ||||||||||||||||||->company : TBTK			 		||||||||||||||||||
 |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*********************************************************************/

/**************************INCLUDES**********************************/
#include "stm32l4xx.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "main.h"

/**************************ENUMARATOR**********************************/
typedef enum{
	False,
	True
}boolean;

/**************************VARIABLES**********************************/
boolean JumpGuard;
char mesg[50]="BL USER APP-1 MSG: Hello from the User APP-1 \r\n";

/***********************USART COMMUNICATION INIT**********************/
void RCCInitForUSART(char char_type_port_name){
	switch(char_type_port_name){
	case 'A': //usart 1&2
		RCC->AHB2ENR &= 0x0;         													//reset value for protection
		RCC->AHB2ENR |= (1U << 2*1);													//GPIOA Clock Enable

		RCC->APB2ENR |= 1U << 14;														//usart1 clk enable
		RCC->APB2ENR |= 1U << 17;														//usart2 clk enable
		break;
	case 'C': //usart3
		RCC->AHB2ENR &= 0x0;         													//reset value for protection
		RCC->AHB2ENR |= (1U << 2);														//GPIOC Clock Enable

		RCC->APB1ENR1 |= 1U << 18;														//usart3 clk enable
		break;
	default:

		USART3_SendTxt("RCC Selection Error!,");
	}
}

void GPIOInitForUSART(char char_type_port_name, uint8_t int_type_Rx_number, uint8_t int_type_Tx_number)
{
	RCCInitForUSART(char_type_port_name);												//RCC config called for gpio implementation

	switch(char_type_port_name){
	case 'A': //usart 1
		if(int_type_Rx_number > 7 || int_type_Tx_number > 7){ 							//AFRL and AFRH condition
			GPIOA->MODER = 0xABFFFFFF;													//reset value for protection
			GPIOA->MODER |= (2U << int_type_Rx_number) | (2U << int_type_Tx_number); 	//pa9 and pa10
			GPIOA->AFR[1] |= (7U << 4) | (7U << 8); 									//selected af7 from mux pa9 and pa10
		}
		else{
			GPIOA->MODER = 0xABFFFFFF;													//reset value for protection
			GPIOA->MODER  |= (2U << int_type_Rx_number) | (2U << int_type_Tx_number); 	//pa2 and pa3
			GPIOA->AFR[0] |= (7U << 8) | (7U << 4); 									//selected af7 from mux pa3 and pA2
		}
		break;
	case 'C': //usart3
		GPIOC->MODER  = 0xFFFFFFFF;														//reset value for protection
		GPIOC->MODER  &= (2U << 2*int_type_Rx_number) | (2U << 2*int_type_Tx_number);	//pc4 and pc5 alternate func
		GPIOC->AFR[0] |= (7U << 16) | (7U << 20); 										//selected af7 from mux for pc4 and pc5
		break;
	default:
		USART3_SendTxt("GPIO Selection Error");
	}
}

void BSP_USART_Config(USART_TypeDef * USARTx){
	//USART1->BRR = (uint16_t)(SystemCoreClock/USART_BAUD);								// 4MHz/115200 = 0x23, 4MHz/9600 = 0x1A1
	USARTx->BRR = 0x23;																	// BaudRate 115200
	USARTx->CR1 |= 1U << 2;																// Rx enable
	USARTx->CR1 |= 1U << 3;																// Tx enable
	USARTx->CR1 |= 1U << 5;																// Rx interrupt enable
	USARTx->CR1 |= 0U << 10;															// Parity control disable
	USARTx->CR1 |= 0U << 12;															// Word length 8bit
	USARTx->CR1 |= 0U << 28;															// Start bit 1 ???
	USARTx->CR2 |= 0U << 12;															// Stop bit 1
	USARTx->CR1 |= 1U << 0;																// Usart enable
}

void USART3_SendChar(char Tx)
{
	while(!(USART3->ISR&0x80));  														// if Tx buffer is full wait
	USART3->TDR=Tx;
}

void USART3_SendTxt(char *Adr)
{
	while(*Adr)
	{
		USART3_SendChar(*Adr);
		Adr++;
	}
}

void printMessages(char *format, ...)
{
	char comingMessage[100];

	va_list vaList;
	va_start(vaList, format);
	vsprintf(comingMessage, format, vaList);
	//USART_WriteText(USART3, (uint8_t*)comingMessage, strlen(comingMessage));
	USART3_SendTxt((uint8_t*)comingMessage);
	va_end(vaList);
}

void USART_WriteChar(USART_TypeDef * USARTx, uint8_t ch) {								//it can write only one bit char to usartx
	USARTx->TDR = (ch & 0x1FF);
	while (!(USARTx->ISR & USART_ISR_TXE));   											// wait until TXE bit is set
}

void USART_WriteText(USART_TypeDef * USARTx, char *TransmitData, int length){			//it can write text to usartx
	for(int i=0; i<length ; ++i){														//write one by one to TDR
		USART_WriteChar(USARTx, TransmitData[i]);
	}
}

/******************************TASKS**********************************/

void delay_ms(uint32_t time)															// for 4MHz: time=400000 is 1s delay
{
	while(time--);
}

void app1_green_led_blink(void){														// pc7 --> 1 second green led blink function
	RCC->AHB2ENR = 0x00000004;
	GPIOC-> MODER   = 1U << 2*7;
	GPIOC->ODR |= 1U << 7;
	delay_ms(400000);
	GPIOC->ODR &= 0U << 7;
	delay_ms(400000);
}


/*************************MAIN FUNCTION******************************/
int main(void){
	/*JumpGuard=True;
	RCCInitForUSART('C');
	GPIOInitForUSART('C', 5, 4);
	BSP_USART_Config(USART3);
	USART_WriteText(USART3, mesg, strlen(mesg));*/
	while(1){
		app1_green_led_blink();
	}
	return 0;
}
