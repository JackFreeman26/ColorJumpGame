/*
 * ColorPWM.h
 *
 *  Created on: May 3, 2025
 *      Author: jwf47
 */

#ifndef COLORPWM_H_
#define COLORPWM_H_
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////////////////
void initColorPWM(); // init led

void setLEDColor(uint8_t rPct, uint8_t gPct, uint8_t bPct); // change color

#endif /* COLORPWM_H_ */
