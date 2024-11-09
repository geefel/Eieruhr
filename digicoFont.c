/*
 * Font, bestehend aus Kreisen oder Rechtecken
 */

#include <stdlib.h>

#include "digicoFont.h"
//#include "tft.h"
#include "paint_tft.h"
//#include "uart_print.h"


void drawStringDigicoFont(uint16_t x, uint16_t y, uint8_t dicke, uint16_t color, uint16_t backColor, uint8_t kreis, char *str) {
	uint8_t strIndex = 0;
	uint16_t vers = 0;
	while (str[strIndex] != '\0') {
		vers += drawCharDigicoFont(x + vers, y, dicke, color, backColor, kreis, str[strIndex]);
		strIndex++;
	}
}

uint8_t drawCharDigicoFont(uint16_t x, uint16_t y, uint8_t dicke, uint16_t color, uint16_t backColor, uint8_t kreis, uint8_t ch) {
    (void) backColor;
	ch -= 32;
	uint16_t xVers, yVers;
	uint8_t breit = digicoFont[ch][0];
    uint8_t maske; // = 1 << breit;
    uint8_t spaz = dicke;

	for(uint8_t z = 1; z < 10; z++) {
		yVers = y + (z - 1) * (dicke + spaz);
		maske = 0b10000000;
		for(uint8_t sp = 0; sp < breit; sp++) {
			xVers = x + sp * (dicke + spaz);
			if(digicoFont[ch][z] & maske) {
				if(kreis)
                    drawFilledCircle(xVers, yVers, dicke, color); //dicke * 2
				else
                    drawFilledRect(xVers, yVers, dicke * 2, dicke * 1.5, color);    //hier die beiden dicken mit verschiedenen Faktoren ausprobieren, def = 1
				
			}
			maske >>= 1;
		}
	}
	return (breit + 1) * (dicke + spaz);
}
