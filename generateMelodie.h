#ifndef GENERATEMELODIE_H
#define GENERATEMELODIE_H

void initPiezo();

/* floate nHoehe        -> struct pitch
 * int bpm              -> beats per minute, Metronom
 * floate nLaenge       -> struct nLength
 * float nVeraenderung  -> struct dot n: normal, p: punktiert, t:triolisch
 * bool isNotPause      -> struct isNote n: Note, p: Pause
 * pinOut               -> Arduino pinOut für Output
 * Achtung: Note B1 muss B_1 geschrieben werden!
 */
void generate(float nHoehe, float nLaenge, float nVeraendert, uint8_t isNotPause, int16_t bpm);
void playMelodie1();
void stopMelodie();

#endif // GENERATEMELODIE_H
