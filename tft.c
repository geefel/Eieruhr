
/*
 * Bildbereich 0° und 180°: 2, 1, 129, 160
 * Bildbereich 90° und 270°: 1, 2, 160, 129
*/

#include <stdint.h>
#include <util/delay.h>
#include "tft.h"

void init_Pins();
void set_MOSI(uint8_t mosi);
void set_Data_Command(uint8_t lcd_dc);
void set_LCD_CS(uint8_t lcd_cs);
void set_LCD_BL(uint8_t lcd_bl);

#if defined(SPI_hard) 	//in tft.h
uint8_t send_LCD_Command_Data(uint8_t command_data, uint8_t val);
#else
void send_LCD_Command_Data(uint8_t command_data, uint8_t val);
#endif

enum {
    command = 0,
    data
};

enum {
    low = 0,
    high
};

void init_Pins() {
    setOutput(SCK_PIN);
	setOutput(MOSI_PIN);
	setOutput(LCD_CS_PIN);
	setOutput(LCD_BL_PIN);
    setOutput(LCD_DC_PIN);
}

#if defined(SPI_hard)//in tft.h
/* SPDR
	 * Datenregister für zu sendende und empfangende Daten
	 * SPCR-Register
	 * CPOL = 0, Flanke steigend, dann fallend, sonst umgekehrt
	 * CPHA = 0, steigende Flanke abtasten, fallende Flanke setzen, sonst umgekehrt
	 * SPIE = 0 (Interupt disable)
	 * SPE = 1, SPI enable
	 * DORD = 0, MSB first
	 * MSTR = 1,Master-Mode
	 * SPR0, SPR1, Teiler für clock
	 * SPRX (im Register SPSR) Cloxk * 2
	 */
void init_SPI() {
	//ZUERST die Pins definieren!!!!!
	SPCR = (1<<SPE)|(1<<MSTR)/*|(1<<SPR0)*/;
	//SPSR = 1<<SPI2X;
}
#endif

/*
	Memory Access Control MADCTE
	D7 MY Page Address Order: 0 = Top to Botton, 1 = umgekehrt
	D6 MX Column Address Order: 0 Left to Right, 1 = umgekehrt
	D5 MV Page/Column Order: 0 = normal, 1 = revers
	D4 ML Line Address Order: 0 = Refresh top to Bottom,1 = umgekehrt
	D3 RGB RGB/BGR Order: 0 = RGB,1 = BGR
	D2 MH Display Data Latch Order: 0 = Refresh Left to Right, 1 = umgekehrt
	MY MX MV 
	0  0  0  normal				
	0  1  1  90° drehen
	1  1  0  180° drehen
	1  0  1  270° drehen
	send_LCD_Register(0x36, 0b01100000);
	*/
void init_TFT() {   //init für ST7735
    init_Pins();

    _delay_ms(100);
    clrPin(LCD_RESET);
    _delay_ms(100);
    setPin(LCD_RESET);
    _delay_ms(100);

    send_LCD_Command_Data(command, 0x36);		//MADCTL
    send_LCD_Command_Data(data, 0x06);

    send_LCD_Command_Data(command, 0x3A);       //COLMOD: Pixel Format Set
    send_LCD_Command_Data(data, 0x05);

    send_LCD_Command_Data(command, 0x11);		//SLPOUT Sleep Out & Booster On
    //send_LCD_Command_Data(command, 0x10);		//SLPIN Sleep In & Booster Off

    send_LCD_Command_Data(command, 0x29);		//DISPON Main screen turn on
    //send_LCD_Command_Data(command, 0x28);		//Main screen turn off
    set_LCD_BL(high);
}

//def ist Rotation = 0°
void set_TFT_rotation(uint16_t grad) {
	switch (grad) {
        case 0:
            send_LCD_Command_Data(command, 0x36);
            send_LCD_Command_Data(data, 0);
			break;
        case 90:
            send_LCD_Command_Data(command, 0x36);
            send_LCD_Command_Data(data, 0b01100000);
			break;
        case 180:
            send_LCD_Command_Data(command, 0x36);
            send_LCD_Command_Data(data, 0b11000000);
			break;
        case 270:
            send_LCD_Command_Data(command, 0x36);
            send_LCD_Command_Data(data, 0b10100000);
			break;
		default: break;
	};
}

inline void set_MOSI(uint8_t mosi) {
	if (mosi)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
}

inline void set_Data_Command(uint8_t lcd_dc) {
	if (lcd_dc)
		setPin(LCD_DC_PIN);
	else
		clrPin(LCD_DC_PIN);
}

inline void set_LCD_CS(uint8_t lcd_cs) {
	if (lcd_cs)
		setPin(LCD_CS_PIN);
	else
		clrPin(LCD_CS_PIN);
}

inline void set_LCD_BL(uint8_t lcd_bl) {
	if (lcd_bl)
		setPin(LCD_BL_PIN);
	else
		clrPin(LCD_BL_PIN);
}


#if defined(SPI_hard)	//in tft.h
//SPI-Version
/*
 * CS und Command/Data werden von SPI nicht gesetzt, deshalb selber machen.
 * SPSR und SPDR müssen ausgelesen werden, sonst wird SPIF nicht gelöscht.
 */
uint8_t send_LCD_Command_Data(uint8_t is_command_data, uint8_t val) {
	set_LCD_CS(low);
	set_Data_Command(is_command_data);
  SPDR = val;
  while (!(SPSR & (1<<SPIF))){;}
	val = SPSR;
	set_LCD_CS(high);
	return SPDR;
}

#else
/*
 * Software-SPI
 * is_command_data: ob Command (auch bei Register-Nr.) oder Data gesendet wird
 * val: Command/Register oder Data
 */
void send_LCD_Command_Data(uint8_t is_command_data, uint8_t val) {	
	
	set_LCD_CS(low);
	clrPin(SCK_PIN);
	set_Data_Command(is_command_data);

	if (val & 0b10000000)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);
	
	if (val & 0b01000000)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);
	
	if (val & 0b00100000)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);
	
	if (val & 0b00010000)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);
	
	if (val & 0b00001000)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);
	
	if (val & 0b00000100)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);
	
	if (val & 0b00000010)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);
	
	if (val & 0b00000001)
		setPin(MOSI_PIN);
	else
		clrPin(MOSI_PIN);
	setPin(SCK_PIN);
	clrPin(SCK_PIN);

	set_LCD_CS(high);
}

#endif

inline uint8_t get_High_Byte(uint16_t val) {
	return (uint8_t)(val >> 8) & 0xFF;
}

inline uint8_t get_Low_Byte(uint16_t val) {
	return (uint8_t) val & 0xFF;
}

/*
 * uint8_t r, g, b;
 * uint16_t color = (r << 11) | (g << 5) | b;
 */

void drawWindow(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint16_t color) {
	uint32_t width = x1 - x + 1;
	uint32_t hight = y1 - y + 1;
	
    send_LCD_Command_Data(command, 0x2A);	//colum address set
    send_LCD_Command_Data(data, (x >> 8) & 0xFF);
    send_LCD_Command_Data(data, x & 0xFF);
    send_LCD_Command_Data(data, (x1 >> 8) & 0xFF);
    send_LCD_Command_Data(data, x1 & 0xFF);
    send_LCD_Command_Data(command, 0x2B);	//row address set
    send_LCD_Command_Data(data, (y >> 8) & 0xFF);
    send_LCD_Command_Data(data, y & 0xFF);
    send_LCD_Command_Data(data, (y1 >> 8) & 0xFF);
    send_LCD_Command_Data(data, y1 & 0xFF);
	
    send_LCD_Command_Data(command, 0x2C);	//hier speichern der Farbe
	for (uint32_t i = 0; i < width * hight; ++i) {
        send_LCD_Command_Data(data, (color >> 8) & 0xFF);
        send_LCD_Command_Data(data, color & 0xFF);
	}
}

void drawPix(uint16_t x, uint16_t y, uint16_t color) {
    send_LCD_Command_Data(command, 0x2A);	//colum address set
    send_LCD_Command_Data(data, (x >> 8) & 0xFF);
    send_LCD_Command_Data(data, x & 0xFF);
    send_LCD_Command_Data(command, 0x2B);	//row address set
    send_LCD_Command_Data(data, (y >> 8) & 0xFF);
    send_LCD_Command_Data(data, y & 0xFF);
    send_LCD_Command_Data(command, 0x2C);	//hier speichern der Farbe
    send_LCD_Command_Data(data, (color >> 8) & 0xFF);
    send_LCD_Command_Data(data, color & 0xFF);
}

void switchTFT_OFF() {
    send_LCD_Command_Data(command, 0x10);		//SLPIN Sleep In & Booster Off
    //send_LCD_Command_Data(command, 0x28);		//Main screen turn off
    setBacklightDim(0);
}

void switchTFT_ON() {
    send_LCD_Command_Data(command, 0x11);		//SLPOUT Sleep Out & Booster On
    //send_LCD_Command_Data(command, 0x29);		//DISPON Main screen turn on
    setBacklightDim(8);
}

uint8_t dimLevel[] = {0, 1, 2, 3, 4, 8, 16, 32, 64, 255};

int8_t setBacklightDim(int8_t dim) {
    if (dim) {
        dim = dim > 9 ? 9 : dim;
        dim = dim < 0 ? 0 : dim;
        TCCR1A |= (1<<COM1A1) | (1<<WGM10);  //Set Timer Register
        TCCR1B |= (1<<WGM12) | (1<<CS11);
        OCR1A = dimLevel[dim];
    }
    else {
        TCCR1B &= ~((1<<WGM12) | (1<<CS11));
        clrPin(LCD_BL_PIN);
    }
	return dim;
}
