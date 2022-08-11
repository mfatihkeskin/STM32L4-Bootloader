/*********************************************************************
 |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 ||||||||||||||||||->file    : bootjump.c 			||||||||||||||||||
 ||||||||||||||||||->author  : M.Fatih KESKİN		||||||||||||||||||
 ||||||||||||||||||->version : v.1.0 				||||||||||||||||||
 ||||||||||||||||||->date    : 29.07.22				||||||||||||||||||
 ||||||||||||||||||->brief   : bootjump		 		||||||||||||||||||
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

/**************************DEFINES*************************************/

#define FLASH_JUMP_APP_1 0x08040000
#define FLASH_JUMP_APP_2 0x08080000
#define FLASH_BOOT 0x08000000

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
//void (*bootloader_reset_handler)(void) = (void*)(*((volatile uint32_t*) (0x08000000 + 4U)));	//not working




