#ifndef SLEEP_H
#define SLEEP_H

#define IDLE_MODE		0
#define ADC_NOISE_RED	1
#define POWER_DOWN		2
#define POWER_SAVE		3
#define STAND_BY		4
#define STAND_BY_EXT	5

void initSleep(uint8_t modus);
void sleepEnable();
void sleepDisable();
void gotoSleep();

#endif  //SLEEP_H
