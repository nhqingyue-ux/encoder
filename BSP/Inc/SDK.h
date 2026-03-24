#ifndef __SDK_H
#define __SDK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32g0xx_ll_utils.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LCD_point {
    uint16_t X, Y;
};

struct LCD_frame {
    struct LCD_point start, end;
};

typedef enum
{
    RETURN_OK                   = 0,
    RETURN_PARAMETER_ERROR      = 1,
    RETURN_OUTPUT_RANGE         = 2,
    RETURN_SET_FAIL             = 3,
}RETURN_CODE;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

// == Interface Initial Function ============================
RETURN_CODE MX_SPI1_Init(void);
RETURN_CODE MX_SPI2_Init(void);
RETURN_CODE MX_GPIO_Init(void);
RETURN_CODE LCD_BrightnessInit(uint32_t duty);

// == LCD Function ==========================================
void LCD_init(void);
void LCD_set_frame(struct LCD_frame frame);
void LCD_Clear_Screen(uint16_t color);
void LCD_Clear_block(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_ShowImg(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *dat);

// == Brightness Function ===================================
RETURN_CODE LCD_BrightnessSetting(uint32_t duty);

// == Flash Function ========================================
bool Flash_Init(void);

void Flash_EraseChip(void);
void Flash_EraseSector(uint32_t SectorAddr);
void Flash_EraseBlock(uint32_t BlockAddr);

void Flash_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
void Flash_Write(uint8_t *pBuffer, uint32_t Page_Address, uint32_t Size);		
	
// == Encoder Function ======================================
void Encoder_Init(void);
void Enocde_Process(void);
void Encoder2Index(uint8_t *index, uint8_t index_Max, uint8_t index_Min, bool dir, bool cycle);

#ifdef __cplusplus
}
#endif

#endif
