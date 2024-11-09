#ifndef PAINT_TFT_H
#define PAINT_TFT_H

#include <stdint.h>

//void initForTFT();
void drawRect(uint16_t x, uint16_t y, uint16_t breit, uint16_t hoch, uint16_t dick, uint16_t color);
void drawFilledRect(uint16_t x, uint16_t y, uint16_t breit, uint16_t hoch, uint16_t color);
void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void drawFilledCircle(int16_t x, int16_t y, int16_t radius, uint16_t color);
void drawVLine(int16_t x, int16_t y, int16_t lang, uint16_t color);
void drawHLine(int16_t x, int16_t y, int16_t breit, uint16_t color);
void fillScreen(uint16_t color);
//void getxy(uint16_t *x, uint16_t *y, uint16_t *z);


#endif	//PAINT_TFT_H
