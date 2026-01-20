/*
 * UI.h
 *
 *  Created on: Apr 24, 2025
 *      Author: jwf47
 */

#ifndef UI_H_
#define UI_H_
#include <ti/grlib/grlib.h>
#include "FSM.h"
////////////////////////////////////////////////////////////////////////////////////////////
void UI_drawTitle(Graphics_Context *ctx); // draw title screen

void UI_drawMenu(Graphics_Context *ctx); // draw menu screen

void UI_drawInstr(Graphics_Context *ctx); // draw instr

void UI_drawGame(Graphics_Context *ctx,ColorChannel cursor,const uint8_t colors[3]); // draw game screen

void UI_drawADCReading(Graphics_Context *ctx); // hw12, not used in project

void UI_drawPreview(Graphics_Context *ctx, uint8_t rPct,uint8_t gPct,uint8_t bPct); // update preview circle

void UI_drawCursor(Graphics_Context *ctx, ColorChannel cursor); // update cursor

void UI_updatePercent(Graphics_Context *ctx,ColorChannel channel,uint8_t pct); // draw percent for adc reading

#endif /* UI_H_ */
