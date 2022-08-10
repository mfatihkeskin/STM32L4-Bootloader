/*********************************************************************
 |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 ||||||||||||||||||->file    : main.c 				||||||||||||||||||
 ||||||||||||||||||->author  : M.Fatih KESKİN		||||||||||||||||||
 ||||||||||||||||||->version : v.1.0 				||||||||||||||||||
 ||||||||||||||||||->date    : 29.07.22				||||||||||||||||||
 ||||||||||||||||||->brief   : usart	 			||||||||||||||||||
 ||||||||||||||||||->company : TBTK			 		||||||||||||||||||
 |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*********************************************************************/

/**************************INCLUDES*****************************************/
#include "stm32l4xx.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "main.h"

/**************************DEFINES******************************************/
#define FLASH_JUMP_APP_1 0x08040000
#define FLASH_JUMP_APP_2 0x08080000
#define FLASH_BOOT 0x08000000

/**************************VARIABLES 32BIT**********************************/
volatile uint32_t ReceivedDataArr[] = {0x00000000};										//as pointer
volatile uint32_t app1_address = FLASH_JUMP_APP_1;										//app1 address
volatile uint32_t app2_address = FLASH_JUMP_APP_2;										//app2 address
volatile uint32_t UploadProgramByteCtr = 0;												//count the upload file byte
volatile uint32_t UartInterruptCtr = 0;													//count the interrupt from terminal
volatile uint32_t FlashProgramCtr = 0;
volatile uint32_t TempDataArr[14336];													//ram data
volatile uint32_t FlashCtr = 0;
volatile uint32_t RamRow = 0;
volatile uint32_t tick=0;																//for systick handler
/**************************VARIABLES 8BIT***********************************/
volatile uint8_t UploadModeTrigger=0;													//file upload mode check
volatile uint8_t TempReceiveData=0;														//template for receive data
volatile uint8_t ReceivedData = 0;														//usart receive data
volatile uint8_t RamColumn = 0;
volatile uint8_t bootCtr=0;

/****************************BACKUP FUNCTION********************************/

void delay_ms(uint32_t time)															// for 4MHz: time=400000 is 1s delay
{
	while(time--);
}

void SysTickHandler(void){																//Systcick Handler for any error
	if(tick>0){
		--tick;
	}
}

/******************************TASKS**********************************/

void app1_green_led_blink(void){														// pc7 --> 1 second green led blink function
	RCC->AHB2ENR = 0x00000004;
	GPIOC-> MODER   = 1U << 2*7;
	GPIOC->ODR = 1U << 7;
	delay_ms(400000);
	GPIOC->ODR = 0U << 7;
	delay_ms(400000);
}

void app2_red_led_blink(void){															//pb14 --> 1 second red led blink function
	RCC->AHB2ENR = 0x00000002;
	GPIOB-> MODER   = 1U << 2*14;
	GPIOB->ODR = 1U << 14;
	delay_ms(400000);
	GPIOB->ODR = 0U << 14;
	delay_ms(400000);
}

/**************************USART COMMUNICATION INIT*************************/


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
		USART_WriteText(USART3,"RCC Selection Error!,",20);
		tick++;																			//Systcick Handler for any error
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
		USART_WriteText(USART3,"GPIO Selection Error",20);
		tick++;																			//Systcick Handler for any error
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

	NVIC_EnableIRQ(USART3_IRQn);
	NVIC_SetPriority(USART3_IRQn , 1);
}

uint8_t USART_ReadChar (USART_TypeDef * USARTx){
	//USARTx->ISR |= 1; USART_ISR_RXNE |= 1;
	while (!(USARTx->ISR | USART_ISR_RXNE));  											// Wait until RXNE (RX not empty) bit is set
																						// USART resets the RXNE flag automatically after reading DR
	return ((uint8_t)(USARTx->RDR & 0x1FF));											// Reading USART_RDR automatically clears the RXNE flag
}

uint8_t USART3_ReadChar (void){
	while (!(USART3->ISR & (1U << 5)));  												// Wait until RXNE (RX not empty) bit is set
	uint8_t temp = (USART3->RDR );																					// USART resets the RXNE flag automatically after reading DR
	return temp;																		// Reading USART_RDR automatically clears the RXNE flag
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

void USART3_SendChar(char Tx)
{
	while(!(USART3->ISR&0x80));  														// if Tx buffer is full wait
	USART3->TDR=Tx;
}

void USART3_SendTxt(char *Adr)															//send Tx buffer until txne empty
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

void USART3_IRQHandler(void){
	UartInterruptCtr ++;																//hold the how many interrupt 
	ReceivedData = (uint8_t)USART3->RDR;
	USART3->ICR |= 0x8;
	USART3->ICR |= 0x1000;

	if(UartInterruptCtr==3){															//if 2 selection mode upload mode enable
		TempReceiveData = ReceivedData;
		if(TempReceiveData == '6' | TempReceiveData == '7')
			UploadModeTrigger = 1;
	}
	else if(UartInterruptCtr>3){														//upload mode enabled and u have been upload .bin file
		UploadProgramByteCtr++;
		ReceivedDataArr[0] |= ReceivedData;
		UploadModeEnable();
	}
}


/****************************** WRITE TO RAM RECEİVED FILE FROM USART**********************************/

void UploadModeEnable(void){															//fill the ram by traveling row by column
	if(RamColumn<4){
		switch(RamColumn){
			case 0:
				//TempDataArr[RamRow] &=0x00000000;
				TempDataArr[RamRow] |= ReceivedDataArr[0]<<0;
				ReceivedDataArr[0] &=0x00000000;
				break;
			case 1:
				//TempDataArr[RamRow] &=0x000000FF;
				TempDataArr[RamRow] |= ReceivedDataArr[0]<<8;
				ReceivedDataArr[0] &=0x00000000;
				break;
			case 2:
				//TempDataArr[RamRow] &=0x0000FFFF;
				TempDataArr[RamRow] |= ReceivedDataArr[0]<<16;
				ReceivedDataArr[0] &=0x00000000;
				break;
			case 3:
				//TempDataArr[RamRow] &=0x00FFFFFF;
				TempDataArr[RamRow] |= ReceivedDataArr[0]<<24;
				ReceivedDataArr[0] &=0x00000000;
				break;
		}
	RamColumn++;
	}
	if(RamColumn==4){
		RamColumn=0;
		RamRow++;
	}
}


/**************************JUMP TO APP**********************************/


void bootloader_default_mode(void)																//let default mode blinky
{
	printMessages("BL BOOTLOADER MSG: Hello from the default mode\r\n");
	for (int i=0; i<500;i++){
		app1_green_led_blink();
		app2_red_led_blink();
	}
}

void bootloader_jump_to_user_app1(void)                                                         //************  APP1  *************//
{

	void (*bootloader_application_reset_handler)(void);                                         //1. holding to reset handler address
	//printMessages("BL DEBUG MSG: Called bootloader_jump_to_user_application() \n");

	uint32_t mspValue = *(volatile uint32_t*) FLASH_JUMP_APP_1;                                 //3. flash jump adress = msp value
	//printMessages("BL DEBUG MSG: MSP Value: %#x \n", mspValue);
	__set_MSP(mspValue);

	uint32_t resetValue = *(volatile uint32_t*) (FLASH_JUMP_APP_1 + 4U);                        //2. Reset Handler = msp+4
	//printMessages("BL DEBUG MSG: Reset Value: %#x \n", resetValue);

	bootloader_application_reset_handler = (void*) resetValue;                                  //5. new operation with reset handler

	bootloader_application_reset_handler();                                                     //6. Reset Handler called and jump the user app

}

void bootloader_jump_to_user_app2(void)                                                         //************  APP2  *************//
{
	void (*bootloader_application_reset_handler)(void);                                         //1. holding to reset handler address
	//printMessages("BL DEBUG MSG: Called bootloader_jump_to_user_application() \n");

	uint32_t mspValue = *(volatile uint32_t*) FLASH_JUMP_APP_2;                                 //3. flash jump adress = msp value
	//printMessages("BL DEBUG MSG: MSP Value: %#x \n", mspValue);
	__set_MSP(mspValue);

	uint32_t resetValue = *(volatile uint32_t*) (FLASH_JUMP_APP_2 + 4U);                        //2. Reset Handler = msp+4
	//printMessages("BL DEBUG MSG: Reset Value: %#x \n", resetValue);

	bootloader_application_reset_handler = (void*) resetValue;                                  //5. new operation with reset handler

	bootloader_application_reset_handler();                                                     //6. Reset Handler called and jump the user app

}

void go_app(void){
	void (*bootloader_reset_handler1)(void) = (void*)(*((volatile uint32_t*) (FLASH_JUMP_APP_1 + 4U)));
	void (*bootloader_reset_handler2)(void) = (void*)(*((volatile uint32_t*) (FLASH_JUMP_APP_2 + 4U)));
	void (*bootloader_reset_handler)(void) = (void*)(*((volatile uint32_t*) (FLASH_BOOT + 4U)));


	switch(bootCtr){
	case 1:
		bootloader_reset_handler1();
	case 2:
		bootloader_reset_handler2();
	}
}

/******************************TERMINAL INTERFACE**********************************/

/*
 * Fist touch u want see the selection terminal. if u click any key for 10 second going to default mode.
 * Second touch is u can choose any mode for selection menu.
 * Third touch if u was choose menu 5, upload mode has been enable. select for instead of any app for new app
 * */

void SelectionTerminalScreen(uint32_t time){
	for(int i=0; i<time; i++){
		if(ReceivedData!=0){
			USART_WriteText(USART3,"This screen send to you information for this program \r\n",56);
			USART_WriteText(USART3,"*********************************************************************************************\r\n",95);
			USART_WriteText(USART3,"****************************** Click to 0 for BOOT-DEFAULT **********************************\r\n",95);
			USART_WriteText(USART3,"****************************** Click to 1 for jump APP-1   **********************************\r\n",95);
			USART_WriteText(USART3,"****************************** Click to 2 for jump APP-2   **********************************\r\n",95);
			USART_WriteText(USART3,"****************************** Click to 3 for delete APP-1 **********************************\r\n",95);
			USART_WriteText(USART3,"****************************** Click to 4 for delete APP-2 **********************************\r\n",95);
			USART_WriteText(USART3,"****************************** Click to 5 for add bin file **********************************\r\n",95);
			USART_WriteText(USART3,"*********************************************************************************************\r\n",95);
			USART_WriteText(USART3,"Please choose between 0-5 on the menu, then click keypad\r\n",60);
			ReceivedData=0;
			while(!ReceivedData);
			if(ReceivedData!=0){
				break;
			}
		}
		delay_ms(400000);
	}
	switch(ReceivedData){
	case 0:
		printMessages("BL DEBUG MESG: Not pressed and going to default selection\r\n");
		bootloader_default_mode();
		break;
	case '0':
		printMessages("BL DEBUG MESG: 0 is pressed and going to bootloader\r\n");
		bootloader_default_mode();
		break;
	case '1':
		printMessages("BL DEBUG MESG: 1 is pressed and going to user APP 1\r\n");
		bootCtr=1;
		//bootloader_reset_handler1();
		//bootloader_jump_to_user_app1();
		break;
	case '2':
		printMessages("BL DEBUG MESG: 2 is pressed and going to user APP 2\r\n");
		bootCtr=2;
		//bootloader_reset_handler2();
		//bootloader_jump_to_user_app2();
		break;
	case '3':
		printMessages("BL DEBUG MESG: 3 is pressed and going to delete APP-1\r\n");
		BSP_FLASH_EraseRoutine(1);
		printMessages("BL DEBUG MESG: APP-1 is deleted\r\n");
		break;
	case '4':
		printMessages("BL DEBUG MESG: 4 is pressed and going to delete APP-2\r\n");
		BSP_FLASH_EraseRoutine(2);
		printMessages("BL DEBUG MESG: APP-2 is deleted\r\n");
		break;
	case '5':
		printMessages("BL DEBUG MESG: 5 is pressed and now u can add bin file\r\n");
		printMessages("BL DEBUG MESG: Please choose where you want to upload this file\r\n");
		printMessages("BL DEBUG MESG: Click 6 for APP-1\r\n");
		printMessages("BL DEBUG MESG: Click 7 for APP-2\r\n");
		ReceivedData=0;
		while(!(ReceivedData));
		if(ReceivedData=='6'){
			printMessages("BL DEBUG MESG: Now u can add ur bin file for APP-1\r\n");
			while(!UploadModeTrigger);
		}
		else if(ReceivedData=='7'){
			printMessages("BL DEBUG MESG: Now u can add ur bin file for APP-2\r\n");
			while(!UploadModeTrigger);
		}
		else{
			BSP_FLASH_Lock();
		}
		break;
	default:
		printMessages("BL DEBUG MESG: PLEASE RESET THE CHIP AND SELECT THE MENU WHAT YOU WANT --> SELECTION ERROR DETECTED!!!!!\r\n");
		break;
	}
	ReceivedData=0;
}


/******************************UPLOAD MODE WRITE FLASH**********************************/

void WriteToAddressApp1(uint32_t *WritingData){												//write app1 to app1 address
	BSP_FLASH_UnLock();
	while (FlashCtr <= FlashProgramCtr){
		BSP_FlashProgram(app1_address,WritingData[FlashCtr]);
		app1_address = app1_address + 4;
		FlashCtr++;
	}
	FLASH->CR |= 0 << 0;
	BSP_FLASH_Lock();
}

void WriteToAddressApp2(uint32_t *WritingData){												//write app2 to app2 address
	BSP_FLASH_UnLock();
	while (FlashCtr <= FlashProgramCtr){
		BSP_FlashProgram(app2_address,WritingData[FlashCtr]);
		app2_address = app2_address + 4;
		FlashCtr++;
	}
	FLASH->CR |= 0 << 0;
	BSP_FLASH_Lock();
}


/******************************MAIN FUNCTION**********************************/

int main(void){
	uint32_t ByteCtr=0;
	RCCInitForUSART('C');
	GPIOInitForUSART('C', 5, 4);
	BSP_USART_Config(USART3);
	printMessages(" Press any key to selection terminal\r\n");
	USART_WriteText(USART3,"  If there is no input 10 sec, the system default will be activated.\r\n",72);
	SelectionTerminalScreen(15);
	go_app();
	while(1){
		if(UartInterruptCtr>2 & UploadProgramByteCtr!=0){
			delay_ms(2000000);
			switch((UploadProgramByteCtr/4)%8){														//4 is flash memory column, 8 is doubleword
				case 0:
					FlashProgramCtr=(UploadProgramByteCtr/4);
					break;
				case 1:
					FlashProgramCtr=(UploadProgramByteCtr/4)+7;
					break;
				case 2:
					FlashProgramCtr=(UploadProgramByteCtr/4)+6;
					break;
				case 3:
					FlashProgramCtr=(UploadProgramByteCtr/4)+5;
					break;
				case 4:
					FlashProgramCtr=(UploadProgramByteCtr/4)+4;
					break;
				case 5:
					FlashProgramCtr=(UploadProgramByteCtr/4)+3;
					break;
				case 6:
					FlashProgramCtr=(UploadProgramByteCtr/4)+2;
					break;
				case 7:
					FlashProgramCtr=(UploadProgramByteCtr/4)+1;
					break;
			}
			switch(TempReceiveData){
				case '6':
					WriteToAddressApp1(TempDataArr);
					if(ByteCtr==(UartInterruptCtr/64)){
						printMessages("Upload completed...\n\r");
					}
					ByteCtr++;
					break;
				case '7':
					WriteToAddressApp2(TempDataArr);
					if(ByteCtr==(UartInterruptCtr/64)){
						printMessages("Upload completed...\n\r");
					}
					ByteCtr++;
					break;
				default:
					printMessages("BL ERR MESG : Flash Writing Error\r\nPlease reset this chip\r\n");
					break;
			}
		}

	}
	return 0;
}
