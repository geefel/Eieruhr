#include <util/delay.h>
#include "pin.h"
#include "generateMelodie.h"

#define PIEZO_1 PINDEF(D, 4)

//Tonhöhe = Frequenz.
static struct {
    const float p;
    const float A2;
    const float Ais2;
    const float B2;
    const float H2;
    const float C1;
    const float Cis1;
    const float Des1;
    const float D1;
    const float Dis1;
    const float Es1;
    const float E1;
    const float F1;
    const float Fis1;
    const float Ges1;
    const float G1;
    const float Gis1;
    const float As1;
    const float A1;
    const float Ais1;
    const float B1;
    const float H1;
    const float C;
    const float Cis;
    const float Des;
    const float D;
    const float Dis;
    const float Es;
    const float E;
    const float F;
    const float Fis;
    const float Ges;
    const float G;
    const float Gis;
    const float As;
    const float A;
    const float Ais;
    const float B;
    const float H;
    const float c;
    const float cis;
    const float des;
    const float d;
    const float dis;
    const float es;
    const float e;
    const float f;
    const float fis;
    const float ges;
    const float g;
    const float gis;
    const float as;
    const float a;
    const float ais;
    const float b;
    const float h;
    const float c1;
    const float cis1;
    const float des1;
    const float d1;
    const float dis1;
    const float es1;
    const float e1;
    const float f1;
    const float fis1;
    const float ges1;
    const float g1;
    const float gis1;
    const float as1;
    const float a1;
    const float ais1;
    const float b1;
    const float h1;
    const float c2;
    const float cis2;
    const float des2;
    const float d2;
    const float dis2;
    const float es2;
    const float e2;
    const float f2;
    const float fis2;
    const float ges2;
    const float g2;
    const float gis2;
    const float as2;
    const float a2;
    const float ais2;
    const float b2;
    const float h2;
    const float c3;
    const float cis3;
    const float des3;
    const float d3;
    const float dis3;
    const float es3;
    const float e3;
    const float f3;
    const float fis3;
    const float ges3;
    const float g3;
    const float gis3;
    const float as3;
    const float a3;
    const float ais3;
    const float b3;
    const float h3;
    const float c4;
    const float cis4;
    const float des4;
    const float d4;
    const float dis4;
    const float es4;
    const float e4;
    const float f4;
    const float fis4;
    const float ges4;
    const float g4;
    const float gis4;
    const float as4;
    const float a4;
    const float ais4;
    const float b4;
    const float h4;
    const float c5;
} pitch = {1,
    27.5, 29.1352, 29.1352, 30.8677, 32.7032, 34.6478, 34.6478, 36.7081, 38.8909, 38.8909, 41.2034,
    43.6535, 46.2493, 46.2493, 48.9994, 51.9131, 51.9131, 55.0, 58.2705, 58.2705, 61.7354, 65.4064,
    69.2957, 69.2957, 73.4162, 77.7817, 77.7817, 82.4069, 87.3071, 92.4986, 92.4986, 97.9989, 103.826,
    103.826, 110.0, 116.541, 116.541, 123.471, 130.813, 138.591, 138.591, 146.832, 155.563, 155.563,
    164.814, 174.614, 184.997, 184.997, 195.998, 207.652, 207.652, 220.0, 233.082, 233.082, 246.942,
    261.626, 277.183, 277.183, 293.665, 311.127, 311.127, 329.628, 349.228, 369.994, 369.994, 391.995,
    415.305, 415.305, 440.0, 466.164, 466.164, 493.883, 523.251, 554.365, 554.365, 587.33, 622.254,
    622.254, 659.255, 698.456, 739.989, 739.989, 783.991, 830.609, 830.609, 880.0, 932.328, 932.328,
    987.767, 1046.5, 1108.73, 1108.73, 1174.66, 1244.51, 1244.51, 1318.51, 1396.91, 1479.98, 1479.98,
    1567.98, 1661.22, 1661.22, 1760.0, 1864.66, 1864.66, 1975.53, 2093.0, 2217.46, 2217.46, 2349.32,
    2489.02, 2489.02, 2637.02, 2793.83, 2959.96, 2959.96, 3135.96, 3322.44, 3322.44, 3520.0, 3729.31,
    3729.31, 3951.07, 4186.01
};

//Notenwert: l1 = ganze Note; l2 = halbe; l4 = viertel; ...
static struct {
    const float l1;
    const float l2;
    const float l4;
    const float l8;
    const float l16;
    const float l32;
    const float l64;
} nLength = {1, 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.01563};

//Notenveränderliche: n = normal; p = punktiert; t = triolisch
static struct {
    const float n;
    const float p;
    const float t;
} dot = {1, 1.5, 0.66667};

//isNote: n = Note; p = Pause. Bei p irgendeine Tonhöhe angeben.
static struct {
    const uint8_t n;
    const uint8_t p;
} isNote = {1, 0};

uint8_t isPlay = 1;

void initPiezo() {
    setOutput(PIEZO_1);
    clrPin(PIEZO_1);
}

void stopMelodie() {
    isPlay = 0;
}

void playMelodie1() {
    uint8_t bpm = 	120	;
    uint8_t anz = 	13	;
    float ton_Pitch[] = {pitch.e1, pitch.fis1, pitch.e1, pitch.gis1, pitch.e1, pitch.h1 , pitch.C1 , pitch.h1 , pitch.a1, pitch.gis1, pitch.fis1, pitch.e1, pitch.c1  	};
    float ton_lang[] = {nLength.l8, nLength.l8, nLength.l8, nLength.l8, nLength.l8, nLength.l8 , nLength.l8 , nLength.l16 , nLength.l16, nLength.l16, nLength.l16, nLength.l4, nLength.l2  };
    float ton_Veraendert[] = {dot.n, dot.n, dot.n, dot.n, dot.n, dot.p , dot.n , dot.n , dot.n, dot.n, dot.n, dot.n, dot.n  	};
    uint8_t ton_istNote[] = {isNote.n, isNote.n, isNote.n, isNote.n, isNote.n, isNote.n , isNote.p , isNote.n , isNote.n, isNote.n, isNote.n, isNote.n, isNote.p  	};
    for (int8_t i = 0; i < anz; ++i) {
        if (!isPlay) break;
        generate(ton_Pitch[i], ton_lang[i], ton_Veraendert[i], ton_istNote[i], bpm);
    }
    isPlay = 1;
}

/*
 * floate nHoehe        -> struct pitch
 * int bpm              -> beats per minute, Metronom
 * floate nLaenge       -> struct nLength
 * float nVeraenderung  -> struct dot n: normal, p: punktiert, t:triolisch
 * bool isNotPause      -> struct isNote n: Note, p: Pause
 */
void generate(float nHoehe, float nLaenge, float nVeraendert, uint8_t isNotPause, int16_t bpm) {
    uint16_t f = (uint16_t) (1000000.0 / nHoehe);   //Schwingungsdauer in µs
    int16_t t = (int16_t) (4 * 60.0 * nLaenge * nVeraendert  * nHoehe / (float) bpm); //Tonlänge.

    f >>= 1;
    for(int i = 0; i < t; i++) {
        if(isNotPause) setPin(PIEZO_1);
        else           clrPin(PIEZO_1);
        for(uint16_t j = 0; j < f; ++j) _delay_us(1);
        clrPin(PIEZO_1);
        for(uint16_t j = 0; j < f; ++j) _delay_us(1);
    }
}
