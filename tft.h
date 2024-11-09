#ifndef TFT_H
#define TFT_H

#include "pin.h"
#include <avr/io.h>
																 //Arduino UNO Pin
#define SCK_PIN 	PINDEF(B, 5) //13
#define MOSI_PIN 	PINDEF(B, 3) //11
#define LCD_CS_PIN 	PINDEF(B, 2) //10
#define LCD_BL_PIN 	PINDEF(B, 1) //9
#define LCD_RESET   PINDEF(B, 0) //8
#define LCD_DC_PIN 	PINDEF(D, 7) //7

extern uint16_t xDispMax;
extern uint16_t yDispMax;
extern uint16_t dispRotation;


//#define SPI_hard

#if defined (SPI_hard)
void init_SPI();
#endif
void init_TFT();
void set_TFT_rotation(uint16_t grad);
void drawPix(uint16_t x, uint16_t y, uint16_t color);
void drawWindow(uint16_t x, uint16_t y, uint16_t x1, uint16_t y2, uint16_t color);
void switchTFT_OFF();
void switchTFT_ON();
int8_t setBacklightDim(int8_t dim);


#endif	//TFT_H
