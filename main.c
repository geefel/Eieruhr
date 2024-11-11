#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "tft.h"
#include "digicoFont.h"
#include "generateMelodie.h"
#include "printS_uart_hard.h"
#include "sleep.h"

#define TASTE_1 PINDEF(C, 1)    //Uno A1
#define TASTE_2 PINDEF(C, 2)    //..
#define TASTE_3 PINDEF(C, 3)
#define TASTE_4 PINDEF(C, 4)
//#define TASTE_5 PINDEF(C, 5)

#define ALARM_MAX 4
#define DIM_MAX   7
#define DIM_MIN   1

#define T_OCR       4
#define T_PRESCALE 16           //1 MHz = 1, 2 MHz = 2, 4 MHz = 4, usw.
#define T_COUNT_FOR_1_SEC T_OCR * T_PRESCALE
#define T_COUNT_FOR_SLEEP 30
#define T_COUNT_FOR_DIM 10

enum _aktion{
    stop = 0,
    laufen,
    warten
};
enum _status{
    uhr = 0,
    setting,
    alarm,
    schlafen
};
enum _setting{
    dec = -1,
    inc = 1
};
enum _color{
    schwarz  = 0x0000,
    weiss    = 0xFFFF,
    blau     = 0x002B,
    hellblau = 0x007C
};
struct {
    int8_t mm;
    int8_t m;
    int8_t ss;
    int8_t s;
} editTime = {0, 0, 0, 0};

struct {
    char *text;
    uint16_t zeit;
} storedTime[] = {{"00:00", 0}, {"03:30", 210}, {"09:30", 570}, {"10:00", 600}, {"15:00", 900}, {"43:00", 2580}};

volatile int8_t timerCountFor1Sec = T_COUNT_FOR_1_SEC;
volatile int8_t timerCountForSleep = T_COUNT_FOR_SLEEP;
volatile int8_t timerCountForDim = T_COUNT_FOR_DIM;
volatile uint16_t theTime;
volatile uint16_t zeit;
volatile uint8_t aktion = stop;
volatile uint8_t status = uhr;
volatile int8_t storedTimeSel = 1;
volatile int8_t editValNr = 1;
volatile uint8_t alarmTime = ALARM_MAX;

void init_PCINT_Interrupt();
void init_1sec_Timer();
void paintTime(uint16_t t);
void paint_set_StoredTimes(uint8_t sel);
void paintMenu(uint8_t set);
void paintSetting();
void playAlarm();
void selNumber(uint8_t n);
void setNumbers(uint8_t n, int8_t inc_dec);


void init_PCINT_Interrupt() {
    setInputPullup(TASTE_1);
    setInputPullup(TASTE_2);
    setInputPullup(TASTE_3);
    setInputPullup(TASTE_4);
//    setInputPullup(TASTE_5);
    PCICR  |=  1 << PCIE1;	 //Interruptfreigabe für PCINT1
    PCMSK1 |= (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11) | (1 << PCINT12) | (1 << PCINT13); //Pegelwechsel an PCINTn löst Interrupt aus
}

void init_1sec_Timer() {
    TCCR2A = 1 << WGM21;
    TIMSK2 = 1 << OCIE2A;
    OCR2A  = 244;
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
}

void start_1sec_Timer() {
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
}

void stop_1sec_Timer() {
    TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
}

void sleeping() {
    sei();
    status = schlafen;
    switchTFT_OFF();
    stop_1sec_Timer();

    sleepEnable();
    gotoSleep();
    sleepDisable();

    switchTFT_ON();
    start_1sec_Timer();
    drawWindow(1, 2, 160, 129, blau);
    paint_set_StoredTimes(storedTimeSel);
    paintMenu(uhr);
    timerCountForSleep = T_COUNT_FOR_SLEEP;
    status = uhr;
}

void dimDown() {
        setBacklightDim(DIM_MIN);
}

void dimUp() {
    setBacklightDim(DIM_MAX);
}

void playAlarm() {
    if (alarmTime == ALARM_MAX)
        paintMenu(alarm);

    playMelodie1();

    if (--alarmTime == 0) {         //automatische Ruhen Nach ALARM_MAX mal
        alarmTime = ALARM_MAX;
        stopMelodie();
        status = uhr;
        aktion = stop;
        paintTime(theTime);
        paintMenu(uhr);
    }
}

int main() {
    initPrintUartHW(115200LU);

    init_PCINT_Interrupt();
    init_1sec_Timer();

    init_TFT();
    dimUp();
    set_TFT_rotation(270);

    initPiezo();

    initSleep(POWER_DOWN);   //TODO: alles mögliche aus machen, mehr als ILE_MODE einstellen

    drawWindow(1, 2, 160, 129, blau);
    paint_set_StoredTimes(storedTimeSel);
    paintMenu(uhr);

    sei();

    status = uhr;
    aktion = stop;

    while (1) {
        while (status == uhr) {
            while (aktion == warten) {    //wenn sec voll (timer2) dann wird auf laufen geschaltet
                timerCountForSleep = T_COUNT_FOR_SLEEP;
            }
            while (aktion == stop) {
                ;
            }
            while (aktion == laufen) {
                timerCountForSleep = T_COUNT_FOR_SLEEP;
                zeit--;
                zeit = zeit == 0xFFFF ? 0 : zeit;
                paintTime(zeit);
                if (zeit == 0) {
                    zeit = theTime;
                    status = alarm;
                    aktion = stop;
                }
                else
                    aktion = warten;        //Uhr läuft und nichts soll zw. den Sekunden passieren: also warten
            }
        }
        while (status == alarm) {
            timerCountForSleep = T_COUNT_FOR_SLEEP;
            timerCountForDim = T_COUNT_FOR_DIM;
            playAlarm();
        }
        while (status == setting) {
            timerCountForSleep = T_COUNT_FOR_SLEEP;
            timerCountForDim = T_COUNT_FOR_DIM;
        }
    }
    return 0;
}

ISR(PCINT1_vect) {  //negative Logig
    cli();
    timerCountForDim = T_COUNT_FOR_DIM;
    dimUp();
//=============== uhr ============================================
    if (status == uhr) {
        if (getPin(TASTE_1) == 0) {
            if (aktion == stop) {
                aktion = laufen;
                drawWindow(9, 107, 50, 126, blau);
                drawStringDigicoFont(10, 108, 1, weiss, blau, 0, "stop");
                timerCountFor1Sec = T_COUNT_FOR_1_SEC;
            }
            else {
                aktion = stop;
                drawWindow(9, 107, 50, 126, blau);
                drawStringDigicoFont(10, 108, 1, weiss, blau, 0, "los");
            }
        }
        else if (getPin(TASTE_2) == 0) {
            storedTimeSel = --storedTimeSel < 1 ? 6 : storedTimeSel;
            paint_set_StoredTimes(storedTimeSel);
        }
        else if (getPin(TASTE_3) == 0) {
            storedTimeSel = ++storedTimeSel > 6 ? 1 : storedTimeSel;
            paint_set_StoredTimes(storedTimeSel);
        }
        else if (getPin(TASTE_4) == 0) {
            status = setting;
            editValNr = 1;
            paintSetting();
        }
    }
//==================== alarm ======================================
    else if (status == alarm) { //alle Tasten
        stopMelodie();
        status = uhr;
        aktion = stop;
        paintTime(theTime);
        paintMenu(uhr);
        timerCountForDim = T_COUNT_FOR_DIM;
    }
//================ setting =========================================
    else if (status == setting) {
        if (getPin(TASTE_1) == 0) {
            editValNr++;
            editValNr = editValNr > 4 ? 1 : editValNr;
            selNumber(editValNr);
        }
        else if (getPin(TASTE_2) == 0) {
            setNumbers(editValNr, inc);
        }
        else if (getPin(TASTE_3) == 0) {
            setNumbers(editValNr, dec);
        }
        else if (getPin(TASTE_4) == 0) {
            storedTime[0].zeit = editTime.mm * 600 + editTime.m * 60 + editTime.ss * 10  + editTime.s;
            storedTime[0].text[0] = editTime.mm + 48;
            storedTime[0].text[1] = editTime.m  + 48;
            storedTime[0].text[3] = editTime.ss + 48;
            storedTime[0].text[4] = editTime.s  + 48;
            storedTimeSel = 1;

            drawWindow(1, 2, 160, 129, blau);

            paint_set_StoredTimes(storedTimeSel);

            status = uhr;
            paintMenu(uhr);
        }
    }
//=============== sleep ==================================================
    else if (status == schlafen) {
        status = uhr;
    }
    _delay_ms(200); //Tastenentprellen
    PCIFR |= 1 << PCIF1; //evtl. Interruptanzeige löschen
    sei();
}

ISR(TIMER2_COMPA_vect) {    //1 Sekunde
    timerCountFor1Sec--;
    if(timerCountFor1Sec == 0) {
        timerCountFor1Sec = T_COUNT_FOR_1_SEC;
        timerCountForSleep--;
        timerCountForDim--;
        if (aktion == warten)   //im while(1) wird nach einmal aktion-laufen auf warten umgestellt
            aktion = laufen;
        if (timerCountForDim == 0)
            dimDown();
        if (timerCountForSleep == 0)
            sleeping();
    }
}


//========== Mainpage ============================================================================

/*
 * m, s als int, alle max 59, alle min 0
 * 0 = Ascii 48, 9 = Ascii 57
 * Darstellung t_text[] = mm:ss
 */
void paintTime(uint16_t t) {
    uint8_t m, s;
    char t_text[] = "00:00";
    m = t / 60;
    s = t % 60;
    t_text[0] = (m / 10) + 48;
    t_text[1] = (m % 10) + 48;
    t_text[3] = (s / 10) + 48;
    t_text[4] = (s % 10) + 48;
    drawWindow(18, 10, 145, 50, blau);
    drawStringDigicoFont(18, 10, 3, weiss, blau, 0, t_text);
}

void paint_set_StoredTimes(uint8_t sel) {
    drawWindow(8, 63, 52, 80, blau);
    drawStringDigicoFont(10, 65, 1, hellblau, blau, 1, storedTime[0].text);
    drawStringDigicoFont(60, 65, 1, hellblau, blau, 1, storedTime[1].text);
    drawStringDigicoFont(111, 65, 1, hellblau, blau, 1, storedTime[2].text);
    drawStringDigicoFont(12, 85, 1, hellblau, blau, 1, storedTime[3].text);
    drawStringDigicoFont(60, 85, 1, hellblau, blau, 1, storedTime[4].text);
    drawStringDigicoFont(109, 85, 1, hellblau, blau, 1, storedTime[5].text);
    switch (sel) {
    case 1: drawWindow(8, 63, 52, 80, blau); drawStringDigicoFont(10, 65, 1, weiss, blau, 1, storedTime[sel -1].text); break;
    case 2: drawStringDigicoFont(60, 65, 1, weiss, blau, 1, storedTime[sel -1].text); break;
    case 3: drawStringDigicoFont(111, 65, 1, weiss, blau, 1, storedTime[sel -1].text); break;
    case 4: drawStringDigicoFont(12, 85, 1, weiss, blau, 1, storedTime[sel -1].text); break;
    case 5: drawStringDigicoFont(60, 85, 1, weiss, blau, 1, storedTime[sel -1].text); break;
    case 6: drawStringDigicoFont(109, 85, 1, weiss, blau, 1, storedTime[sel -1].text); break;
    default: break;
    }
    zeit = storedTime[sel -1].zeit;     //Hauptzeitanzeige
    theTime = zeit;
    paintTime(zeit);

}

void paintMenu(uint8_t set) {  //TODO für Stop
    drawWindow(9, 107, 158, 127, blau);
    if (set == uhr) {
        drawStringDigicoFont(10, 108, 1, weiss, blau, 0, "los");
        drawStringDigicoFont(55, 108, 1, weiss, blau, 1, "<-");
        drawStringDigicoFont(90, 108, 1, weiss, blau, 1, "->");
        drawStringDigicoFont(125, 108, 1, weiss, blau, 0, "set");
    }
    else if (set == setting) {
        drawStringDigicoFont(10, 108, 1, weiss, blau, 0, "sel");
        drawStringDigicoFont(55, 98, 2, weiss, blau, 0, "+");
        drawStringDigicoFont(89, 98, 2, weiss, blau, 0, "-");
        drawStringDigicoFont(125, 108, 1, weiss, blau, 0, "gut");
    }
    else if (set == alarm) {
        drawStringDigicoFont(10, 108, 1, weiss, blau, 0, "stop alarm");
    }
}

//================ setting =============================================================

void paintSetting() {
    drawWindow(8, 10, 160, 130, blau);
    drawCharDigicoFont(18, 10, 3, weiss, blau, 0, editTime.mm + 48);
    drawCharDigicoFont(48, 10, 3, hellblau, blau, 0, editTime.m + 48);
    drawCharDigicoFont(78, 10, 3, hellblau, blau, 0, ':');
    drawCharDigicoFont(90, 10, 3, hellblau, blau, 0, editTime.ss + 48);
    drawCharDigicoFont(120, 10, 3, hellblau, blau, 0, editTime.s + 48);
    paintMenu(setting);

}

void selNumber(uint8_t n) {
    drawCharDigicoFont(18, 10, 3, hellblau, blau, 0, editTime.mm + 48);
    drawCharDigicoFont(48, 10, 3, hellblau, blau, 0, editTime.m + 48);
    drawCharDigicoFont(90, 10, 3, hellblau, blau, 0, editTime.ss + 48);
    drawCharDigicoFont(120, 10, 3, hellblau, blau, 0, editTime.s + 48);
    switch (n) {
    case 1: drawCharDigicoFont(18, 10, 3, weiss, blau, 0, editTime.mm + 48); break;
    case 2: drawCharDigicoFont(48, 10, 3, weiss, blau, 0, editTime.m + 48); break;
    case 3: drawCharDigicoFont(90, 10, 3, weiss, blau, 0, editTime.ss + 48); break;
    case 4: drawCharDigicoFont(120, 10, 3, weiss, blau, 0, editTime.s + 48); break;
    }
}

void setNumbers(uint8_t n, int8_t inc_dec) {
    switch(n) {
    case 1:
        editTime.mm += inc_dec;
        editTime.mm = editTime.mm < 0 ? 9 : editTime.mm;
        editTime.mm = editTime.mm > 9 ? 0 : editTime.mm;
        drawWindow(18, 10, 45, 80, blau);
        drawCharDigicoFont(18, 10, 3, weiss, blau, 0, editTime.mm + 48);
        break;
    case 2:
        editTime.m += inc_dec;
        editTime.m = editTime.m < 0 ? 9 : editTime.m;
        editTime.m = editTime.m > 9 ? 0 : editTime.m;
        drawWindow(48, 10, 75, 80, blau);
        drawCharDigicoFont(48, 10, 3, weiss, blau, 0, editTime.m + 48);
        break;
    case 3:
        editTime.ss += inc_dec;
        editTime.ss = editTime.ss < 0 ? 5 : editTime.ss;
        editTime.ss = editTime.ss > 5 ? 0 : editTime.ss;
        drawWindow(90, 10, 120, 80, blau);
        drawCharDigicoFont(90, 10, 3, weiss, blau, 0, editTime.ss + 48);
        break;
    case 4:
        editTime.s += inc_dec;
        editTime.s = editTime.s < 0 ? 9 : editTime.s;
        editTime.s = editTime.s > 9 ? 0 : editTime.s;
        drawWindow(120, 10, 150, 80, blau);
        drawCharDigicoFont(120, 10, 3, weiss, blau, 0, editTime.s + 48);
        break;
    }
}
