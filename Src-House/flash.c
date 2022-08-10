/*********************************************************************
 |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 ||||||||||||||||||->file    : flash.c				||||||||||||||||||
 ||||||||||||||||||->author  : M.Fatih KESKİN		||||||||||||||||||
 ||||||||||||||||||->version : v.1.0 				||||||||||||||||||
 ||||||||||||||||||->date    : 29.07.22				||||||||||||||||||
 ||||||||||||||||||->brief   : flash		 		||||||||||||||||||
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

/*
 * ----------------------------------
 * |								|
 * |	BOOTLOADER(0X08000000)		|
 * |                                |
 * ----------------------------------
 * |			EMPTY				|
 * ----------------------------------
 * |								|
 * |	   APP-1(0X08040000)		|
 * |                                |
 * ----------------------------------
 * |			EMPTY				|
 * ----------------------------------
 * |								|
 * |		APP-2(0X08080000)		|
 * |                                |
 * ----------------------------------*/

/**************************ENUMARATOR**********************************/
 
typedef enum
{
  FLASH_CACHE_DISABLED = 0,
  FLASH_CACHE_ICACHE_ENABLED,
  FLASH_CACHE_DCACHE_ENABLED,
  FLASH_CACHE_ICACHE_DCACHE_ENABLED
} FLASH_CacheTypeDef;
typedef enum
{
  HAL_UNLOCKED = 0x00,
  HAL_LOCKED   = 0x01
} HAL_LockTypeDef;
typedef struct
{
  HAL_LockTypeDef             Lock;              /* FLASH locking object */
  __IO uint32_t               ErrorCode;         /* FLASH error code */
  __IO uint32_t               Address;           /* Internal variable to save address selected for program in IT context */
  __IO uint32_t               Bank;              /* Internal variable to save current bank selected during erase in IT context */
  __IO uint32_t               Page;              /* Internal variable to define the current page which is erasing in IT context */
  __IO uint32_t               NbPagesToErase;    /* Internal variable to save the remaining pages to erase in IT context */
  __IO FLASH_CacheTypeDef     CacheToReactivate; /* Internal variable to indicate which caches should be reactivated */
}FLASH_ProcessTypeDef;

FLASH_ProcessTypeDef pFlash = {.ErrorCode = 0U, \
                               .Address = 0U, \
                               .Page = 0U, \
                               .NbPagesToErase = 0U, \
                               .CacheToReactivate = FLASH_CACHE_DISABLED};




#define __HAL_LOCK(__HANDLE__)                                           \
                              do{                                        \
                                  if((__HANDLE__)->Lock == HAL_LOCKED)   \
                                  {                                      \
                                     return HAL_BUSY;                    \
                                  }                                      \
                                  else                                   \
                                  {                                      \
                                     (__HANDLE__)->Lock = HAL_LOCKED;    \
                                  }                                      \
                                }while (0)
#define __HAL_UNLOCK(__HANDLE__)                                          \
                                do{                                       \
                                    (__HANDLE__)->Lock = HAL_UNLOCKED;    \
                                  }while (0)
typedef enum
{
  HAL_OK       = 0x00,
  HAL_ERROR    = 0x01,
  HAL_BUSY     = 0x02,
  HAL_TIMEOUT  = 0x03
} HAL_StatusTypeDef;


/**************************DEFINES**********************************/

#define __HAL_FLASH_INSTRUCTION_CACHE_DISABLE() CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICEN)
#define __HAL_FLASH_INSTRUCTION_CACHE_RESET()   do { SET_BIT(FLASH->ACR, FLASH_ACR_ICRST);   \
                                                     CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICRST); \
                                                   } while (0)
#define __HAL_FLASH_INSTRUCTION_CACHE_ENABLE()  SET_BIT(FLASH->ACR, FLASH_ACR_ICEN)
#define __HAL_FLASH_DATA_CACHE_RESET()          do { SET_BIT(FLASH->ACR, FLASH_ACR_DCRST);   \
                                                     CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCRST); \
                                                   } while (0)
#define __HAL_FLASH_DATA_CACHE_ENABLE()         SET_BIT(FLASH->ACR, FLASH_ACR_DCEN)


/*************************************LOCK/UNLOCK*************************************/

void BSP_FLASH_Lock(void)
{
	FLASH->CR |= 0x80000000;
}


void BSP_FLASH_UnLock(void)
{
	while ((FLASH->SR & 0x00010000) != 0 );

	if(FLASH->CR & 0x80000000)
	{
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}
}

/*************************************ERASE*************************************/

void BSP_FLASH_BankPageErase(uint8_t EraseBankNo, uint16_t ErasePage){//Per page is 2Kb and 256 page, per bank has 512Kb and 2 bank equal flash size

	FLASH->SR |= 0x00000001;										// End of Operation flag clear
	FLASH->SR |= 0x00000040;										// Programming Parallelism error flag clear
	FLASH->SR |= 0x00000010;										// Write protected error flag clear

	while((FLASH->SR & 0x00010000) != 0);                           //Check that no Flash memory operation is ongoing by checking the BSY bit
    FLASH->SR |= 1U << 7;                                           //Check and clear all error programming flags due to a previous programming. If not, PGSERR is set.
    FLASH->CR |= 1U << 1;											//Set the PER bit
	if(EraseBankNo==1){
		FLASH->CR |= 0U << 11;
		for(int i=ErasePage;i<160;i++){
			FLASH->CR |= i << 3;									//binary value shifted pnb regster
		    FLASH->CR |= 1U << 16;                                  //strt bit set. HSİ on so 16mhz.
			while((FLASH->SR & 0x00010000) != 0);                   //Check that no Flash memory operation is ongoing by checking the BSY bit
		}
	}
	else if(EraseBankNo==2){
		FLASH->CR |= 1U << 11;
		for(int i=ErasePage;i<288;i++){
			FLASH->CR |= i << 3;									//binary value shifted pnb regster
		    FLASH->CR |= 1U << 16;                                  //strt bit set. HSİ on so 16mhz.
			while((FLASH->SR & 0x00010000) != 0);                   //Check that no Flash memory operation is ongoing by checking the BSY bit
		}
	}
}

void BSP_FLASH_EraseRoutine(uint8_t EraseBankNo){
	BSP_FLASH_UnLock();
	if(EraseBankNo==1){
		BSP_FLASH_BankPageErase(EraseBankNo,128);
	}
	else if(EraseBankNo==2){
		BSP_FLASH_BankPageErase(EraseBankNo,256);
	}
	else{
		BSP_FLASH_Lock();
	}
	BSP_FLASH_Lock();
}

/*************************************FLASH PROGRAM INTO FLASH******************************/


void FLASH_FlushCaches(void)
{
  FLASH_CacheTypeDef cache = pFlash.CacheToReactivate;

  /* Flush instruction cache  */
  if((cache == FLASH_CACHE_ICACHE_ENABLED) ||
     (cache == FLASH_CACHE_ICACHE_DCACHE_ENABLED))
  {
    /* Disable instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    /* Reset instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();
    /* Enable instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  }

  /* Flush data cache */
  if((cache == FLASH_CACHE_DCACHE_ENABLED) ||
     (cache == FLASH_CACHE_ICACHE_DCACHE_ENABLED))
  {
    /* Reset data cache */
    __HAL_FLASH_DATA_CACHE_RESET();
    /* Enable data cache */
    __HAL_FLASH_DATA_CACHE_ENABLE();
  }

  /* Reset internal variable */
  pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
}

void DoubleWordWriteInit(uint32_t Address, uint32_t Data){											//its not a doubleword. but its work
	FLASH->CR |= 1U << 0;
	*(__IO uint32_t*)Address = (uint32_t)Data;
	//__ISB();
	//*(__IO uint32_t*)(Address+4U) = (uint32_t)(Data >> 32);
}

void BSP_FlashProgram(uint32_t Address, uint32_t Data){
	uint32_t prog_bit = 0;
	//__HAL_LOCK(&pFlash);
	while((FLASH->SR & 0x00010000) != 0);
	if((FLASH->SR & 0x00010000) == 0){
		pFlash.ErrorCode = 0;
	}
	if((FLASH->ACR, FLASH_ACR_DCEN) != 0U){
		FLASH->ACR &= 0U << 10;
		pFlash.CacheToReactivate = FLASH_CACHE_DCACHE_ENABLED;
	}
	else{
	  pFlash.CacheToReactivate = FLASH_CACHE_DISABLED;
	}
	DoubleWordWriteInit(Address, Data);
	//prog_bit = FLASH_CR_PG;
	while((FLASH->SR & 0x00010000) != 0);
	//if (prog_bit != 0U){
	  //CLEAR_BIT(FLASH->CR, prog_bit);
	//}
	FLASH_FlushCaches();
	//__HAL_UNLOCK(&pFlash);
}





























/******************************BACKUP FUNCTION**********************************/

void BSP_FLASH_FastWrite(uint8_t WriteBankNo, uint32_t WriteAdress, uint32_t WritingData){
    FLASH->SR |= 1U << 7;                                           //Check and clear all error programming flags due to a previous programming. If not, PGSERR is set.
    while((FLASH->SR & 0x00010000) != 0);
    FLASH->CR |= 1U << 18;											//Set the fstpg
	if(WriteBankNo==1){
		*(__IO uint32_t*)WriteAdress = WritingData;
		while((FLASH->SR & 0x00010000) != 0);                   	//Check that no Flash memory operation is ongoing by checking the BSY bit
		FLASH->CR |= 1U << 24;										//eopie enable
		if((FLASH->SR & 0x00000001)){								//Check that EOP flag is set in the FLASH_SR register, and clear it
			//FLASH->SR |= 0x00000001;
			FLASH->CR |= 0U << 18;
		}
	}
}

void BSP_FLASH_BankErase(uint8_t BankNo){                           //1->bank1 erase, 2->bank2 erase, 3->both erase, others pass func
    while((FLASH->SR & 0x00010000) != 0);                           //Check that no Flash memory operation is ongoing by checking the BSY bit
    FLASH->SR |= 1U << 7;                                           //Check and clear all error programming flags due to a previous programming. If not, PGSERR is set.
    if (BankNo==1){
        FLASH->SR |= 1U << 2;                                       //Bank1 mass erase
    }
    else if(BankNo==2){
        FLASH->SR |= 1U << 15;                                      //Bank2 mass erase
    }
    else if(BankNo==3){                                             //bank1 and bank2 mass erase
        FLASH->SR |= 1U << 15;
        FLASH->SR |= 1U << 2;
    }
    FLASH->CR |= 1U << 16;                                          //strt bit set. HSİ on so 16mhz.
    //RCC->CR &= 0U << 8;                                           //HSİ off. so now 4 mhz
    while((FLASH->SR & 0x00010000) != 0);                           //Check that no Flash memory operation is ongoing by checking the BSY bit
}





/*************************************WRİTE*************************************

void BSP_FLASH_BankPageWrite(uint8_t WriteBankNo, uint32_t WriteAdress, uint32_t WritingData[]){
	while((FLASH->SR & 0x00010000) != 0);                           //Check that no Flash memory operation is ongoing by checking the BSY bit
    FLASH->SR |= 1U << 7;                                           //Check and clear all error programming flags due to a previous programming. If not, PGSERR is set.
    FLASH->CR |= 1U << 0;											//Set the PG bit
	if(WriteBankNo==1){
		for(int i=0; i<(strlen(WritingData)-1);i++){
			*(__IO uint32_t*)(WriteAdress+4U) = WritingData[i];
		}
		while((FLASH->SR & 0x00010000) != 0);                   	//Check that no Flash memory operation is ongoing by checking the BSY bit
		FLASH->CR |= 1U << 24;										//eopie enable
		FLASH->SR |= 0x00000001;
		if((FLASH->SR & 0x00000001)){								//Check that EOP flag is set in the FLASH_SR register, and clear it
			FLASH->SR &= 0;
			FLASH->CR |= ~0x00000001;								//Clear the PG bit in the FLASH_CR register if there no more programming request anymore.
		}
	}
	else if(WriteBankNo==2){
		*(__IO uint32_t*)WriteAdress = WritingData;
		while((FLASH->SR & 0x00010000) != 0);                   	//Check that no Flash memory operation is ongoing by checking the BSY bit
		if((FLASH->SR & 0x00000001)){								//Check that EOP flag is set in the FLASH_SR register, and clear it
			FLASH->SR &= 0U << 0;
			FLASH->CR &= 0U << 0;									//Clear the PG bit in the FLASH_CR register if there no more programming request anymore.
		}
	}
}

void BSP_FLASH_WriteRoutine(uint8_t WriteBankNo, uint32_t WritingData[]){
	BSP_FLASH_UnLock();
	if(WriteBankNo==1){
		BSP_FLASH_BankPageWrite(WriteBankNo,0x08040000,(uint32_t *)WritingData);
	}
	else if(WriteBankNo==2){
		BSP_FLASH_BankPageWrite(WriteBankNo,0x08080000,(uint32_t *)WritingData);
	}
	else{
		BSP_FLASH_Lock();
	}
	BSP_FLASH_Lock();
}*/
