/*
 * UI.c
 *
 *  Created on: Apr 24, 2025
 *      Author: jwf47
 */
#include "UI.h"
#include <LcdDriver/Crystalfontz128x128_ST7735.h>
#include <ti/grlib/grlib.h>
#include <stdint.h>
#include <stdio.h>
#include "HAL/LED.h"
#include "FSM.h"

////////////////////////////////////////////////////////////////////////////////////////////
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  128
////////////////////////////////////////////////////////////////////////////////////////////
// preview circle dim
static const int16_t prevX   =  64;
static const int16_t prevY   =  40;
static const int16_t prevR   =  25;
// 3 circle dim
static const int16_t circleX[3] = { 32, 64, 96 };
static const int16_t circleY      = 80;
static const int16_t circleR      = 12;

// percent display logic
static const int16_t labelYOffset = circleR + 8;
static const int16_t labelW       = 40;
static const int16_t labelH       = 16;
////////////////////////////////////////////////////////////////////////////////////////////
// title
void UI_drawTitle(Graphics_Context *ctx) {
    Graphics_clearDisplay(ctx);

    Graphics_setFont(ctx, &g_sFontCmss22b);
    Graphics_drawStringCentered(ctx,"Color Mixer", AUTO_STRING_LENGTH, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 10,OPAQUE_TEXT);

    Graphics_setFont(ctx, &g_sFontFixed6x8);
    Graphics_drawStringCentered(ctx, "by Jack Freeman", AUTO_STRING_LENGTH, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 20,  OPAQUE_TEXT);
}
////////////////////////////////////////////////////////////////////////////////////////////
// menu
void UI_drawMenu(Graphics_Context *ctx) {
    Graphics_clearDisplay(ctx);

    Graphics_setFont(ctx, &g_sFontCmss22b);
    Graphics_drawStringCentered(ctx, "Main Menu", AUTO_STRING_LENGTH, SCREEN_WIDTH/2, 30, OPAQUE_TEXT);

    Graphics_setFont(ctx, &g_sFontFixed6x8);
    Graphics_drawStringCentered(ctx, "BB1: Start Game", AUTO_STRING_LENGTH,SCREEN_WIDTH/2, 70,OPAQUE_TEXT);
    Graphics_drawStringCentered(ctx,"BB2: Instructions", AUTO_STRING_LENGTH,SCREEN_WIDTH/2, 90,OPAQUE_TEXT);
}
////////////////////////////////////////////////////////////////////////////////////////////
// list of string containing the instr
static const char *instrLines[] = {
    "RGB Mixer",
    "",
    "Use the potentiometer",
    "to adjust the booster",
    "pack light.",
    "",
    "Press BB1 to change",
    "which color gets",
    "adjusted.",
    "",
    "Press BB2 to display",
    "the current mix of",
    "colors on the BLED.",
    "",
    "BB2 to return to",
    "menu"
};
////////////////////////////////////////////////////////////////////////////////////////////
void UI_drawInstr(Graphics_Context *ctx) {
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_BLACK);
    Graphics_clearDisplay(ctx);

    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_WHITE);
    Graphics_setFont(ctx, &g_sFontFixed6x8);

    const int16_t x = 4;
    const int16_t y0 = 0;
    const int16_t lineHeight = 8;
    // i take no credit in figuring this out, i googled how to print these strings because doing like 14 draw string lines would be messier
    const int nLines = sizeof(instrLines)/sizeof(instrLines[0]); // print strings
    int i;
    for ( i = 0; i < nLines; i++) {
        Graphics_drawString(ctx, (int8_t*)instrLines[i], AUTO_STRING_LENGTH, x, y0 + i * lineHeight, OPAQUE_TEXT);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
void UI_drawGame(Graphics_Context *ctx,ColorChannel cursor,const uint8_t colors[3]){
  // title
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_WHITE);
    Graphics_setBackgroundColor(ctx, GRAPHICS_COLOR_BLACK);
    Graphics_clearDisplay(ctx);
    Graphics_drawStringCentered(ctx, (int8_t*)"Game Screen", AUTO_STRING_LENGTH, prevX, 5, OPAQUE_TEXT);

    // draw preview circle
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_WHITE);
    Graphics_fillCircle(ctx, prevX, prevY, prevR);
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_WHITE);
    Graphics_drawCircle(ctx, prevX, prevY, prevR);

    // draw the 3 rgb circles
    int i;
    for (i = 0; i < 3; i++) {
        switch ((ColorChannel)i) {
          case COL_RED:
            Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_RED);
            break;
          case COL_GREEN:
            Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_GREEN);
            break;
          case COL_BLUE:
            Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_BLUE);
            break;
        }
        Graphics_fillCircle(ctx, circleX[i], circleY, circleR);
        Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_BLACK);
        Graphics_drawCircle(ctx, circleX[i], circleY, circleR);

        // draw percent under
        UI_updatePercent(ctx, (ColorChannel)i, colors[i]);
    }
    // draw cursor on red
    UI_drawCursor(ctx, cursor);
}
////////////////////////////////////////////////////////////////////////////////////////////
// unused hw12 requirement
void UI_drawADCReading(Graphics_Context *ctx) {
    uint16_t raw = ADC14_getResult(ADC_MEM0);
    ADC14_clearInterruptFlag(ADC_INT0);

    uint8_t pct = rawToCalibratedPercent(raw);

    char buf[12];
    sprintf(buf, "ADC: %3u%", pct);
    Graphics_drawString(ctx, (int8_t*)buf, AUTO_STRING_LENGTH, 10, 110, OPAQUE_TEXT);
}
////////////////////////////////////////////////////////////////////////////////////////////
// update the preview circle
void UI_drawPreview(Graphics_Context *ctx, uint8_t rPct, uint8_t gPct, uint8_t bPct){
    // convert raw
    uint8_t r8 = (rPct * 255 + 50) / 100;
    uint8_t g8 = (gPct * 255 + 50) / 100;
    uint8_t b8 = (bPct * 255 + 50) / 100;

    uint32_t color24 = ((uint32_t)r8 << 16) | ((uint32_t)g8 << 8) |  (uint32_t)b8; // bit shift to 24 bit format

    // draw circle
    Graphics_setForegroundColor(ctx, color24);
    Graphics_fillCircle(ctx, prevX, prevY, prevR);
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_BLACK);
    Graphics_drawCircle(ctx, prevX, prevY, prevR);
}
////////////////////////////////////////////////////////////////////////////////////////////
// draw cursor
void UI_drawCursor(Graphics_Context *ctx, ColorChannel cursor) {
    int16_t labelY  = circleY + labelYOffset;
    int16_t caretY  = labelY + (labelH/2) + 4;
    int16_t caretH  = 8;
    int16_t halfH   = caretH / 2;
    int16_t caretW  = 12;
    int16_t halfW   = caretW / 2;

    // erase old cursor
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_BLACK);
    int i;
    for (i = 0; i < 3; i++) {
        Graphics_Rectangle eraseRect = {
            .xMin = circleX[i] - halfW,
            .yMin = caretY - halfH,
            .xMax = circleX[i] + halfW,
            .yMax = caretY + halfH
        };
        Graphics_fillRectangle(ctx, &eraseRect);
    }

    // draw new cursor
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_WHITE);
    Graphics_drawStringCentered(ctx, (int8_t*)"^", AUTO_STRING_LENGTH, circleX[cursor], caretY, OPAQUE_TEXT);
}
////////////////////////////////////////////////////////////////////////////////////////////
// update percent
void UI_updatePercent(Graphics_Context *ctx,  ColorChannel channel, uint8_t pct)
{
    int16_t cx = circleX[channel];
    int16_t cy = circleY + labelYOffset;

    // rect over old number
    int16_t x0 = cx - labelW/2;
    int16_t y0 = cy - labelH/2;
    Graphics_Rectangle rect = {
        x0,
        y0,
        x0 + labelW - 1,
        y0 + labelH - 1
    };

    // erase old percent
    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(ctx, &rect);

    // draw new percent
    char buf[8];
    sprintf(buf, "%3u%%", pct);

    Graphics_setForegroundColor(ctx, GRAPHICS_COLOR_WHITE);
    Graphics_drawStringCentered(ctx, (int8_t*)buf, AUTO_STRING_LENGTH, cx,  cy, OPAQUE_TEXT);
}
