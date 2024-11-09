/*
 * Vorgehen:
 * 1. initSleep() mit dem entsprechendem Sleepmodus
 * 2. direkt vor dem Schlafenlegen: sleepEnable()
 * 3. zum Schlafenlegen: gotoSleep()
 * 4. direkt nach dem Schlafengehen: sleepDisable()
 * Es muss die Interrupt-Funktion, die für das Aufwachen benutzt wird, vorhanden sein, kann aber leer sein. 
 * Nach dem Wecken wird diese Interrupt-Funktion abgearbeitet und dann geht es nach gotoSleep() weiter. 
 * D.h. das sleepDisable() evtl. als erstes in der Interrupt-Funktion aufgerufen.
 */

#include <avr/sleep.h>

void initSleep(uint8_t modus) {
	//ATtiny hat nur die ersten drei Modi
	//Standby-Modi sind nur verfügbar, wenn Quarze oder Resonatoren verwendet werden.
	switch (modus) {
        case 0: SMCR = 0; break;                                        //Idle-Mode
        case 1: SMCR |= (1 << SM0); break;                              //ADC Noise-reduction
        case 2: SMCR |= (1 << SM1); break;                              //Power-Down
        case 3: SMCR |= (1 << SM1) | (1 << SM0); break;                 //Power-Save
        case 4: SMCR |= (1 << SM2) | (1 << SM1); break;                 //Stand-By
        case 5: SMCR |= (1 << SM2) | (1 << SM1) | (1 << SM0); break;    //Stand-By extendet
		default: break;
	}
}

//Analog-Digital-Wandler, Analogvergleicher, Unterspannungsdetektor, Interne Spannungsreferenz, Watchdog-Zeitgeber, Portpins,
//PRR-Register, MCUCR-Register

void sleepEnable() {
	SMCR |= (1 << SE);
}

void sleepDisable() {
	SMCR &= ~(1 << SE);
}

void gotoSleep() {
  __asm__ __volatile__ ( "sleep" "\n\t" :: );
}
