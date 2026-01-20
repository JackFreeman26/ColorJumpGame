/*
 * FSM.c
 *
 *  Created on: Apr 24, 2025
 *      Author: jwf47
 */
#include "FSM.h"
#include "UI.h"
#include "HAL/LED.h"
#include "ColorPWM.h"
#include "HAL/Timer.h"
////////////////////////////////////////////////////////////////////////////////////////////
AppState current_state; // state
static ColorChannel selectedChannel; // r, g, or b
static uint8_t   colorValues[3]; // store the percent vals from each channel

volatile bool     adcEvent; // adc flag
static bool     state_drawn; // draw flag

volatile uint16_t lastADCReading; // store last adc

static SWTimer  titleTimer; // timer for title
static bool     titleTimerStarted = false; // flag for title
////////////////////////////////////////////////////////////////////////////////////////////
// init fsm
void FSM_init() {
    current_state = ST_TITLE;
    state_drawn   = false;
    selectedChannel = COL_RED;
}
////////////////////////////////////////////////////////////////////////////////////////////
// FSM logic, i broke up the functions to process to make it cleaner
void FSM_step(const buttons_t *button, Graphics_Context *ctx) {
    switch (current_state) {
        case ST_TITLE:
            processTitle(button, ctx);
            break;
        case ST_MENU:
            processMenu(button, ctx);
            break;
        case ST_INSTR:
            processInstr(button, ctx);
            break;
        case ST_GAME:
            processGame(button, ctx);
            break;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
void processTitle(const buttons_t *button, Graphics_Context *ctx) {
    if (!state_drawn) {
        UI_drawTitle(ctx);
        state_drawn = true;
        titleTimer = SWTimer_construct(2000); // 2 second sw timer for title screen
        SWTimer_start(&titleTimer);
        titleTimerStarted = true;
         }
         else {
             if (titleTimerStarted && SWTimer_expired(&titleTimer)) {
                 current_state      = ST_MENU;
                 state_drawn        = false;
                 titleTimerStarted  = false;
             }
         }
     }
////////////////////////////////////////////////////////////////////////////////////////////
//process main menu
void processMenu(const buttons_t *button, Graphics_Context *ctx) {
    if (!state_drawn) {
        UI_drawMenu(ctx);
        state_drawn = true;
    }
    else if (button->BB1tapped) { // bb1 goes to game
        current_state = ST_GAME;
        state_drawn   = false;
    }
    else if (button->BB2tapped) { // bb2 goes to instr
        current_state = ST_INSTR;
        state_drawn   = false;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
//process instr
void processInstr(const buttons_t *button, Graphics_Context *ctx) {
    if (!state_drawn) {
        UI_drawInstr(ctx);
        state_drawn = true;
    }
    else if (button->BB2tapped) {
        current_state = ST_MENU;
        state_drawn   = false;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////\
// process game, kinda clunky but its handling all the game logic
// im hard passing the color values because there was a bug with the parameters passing wrong, it was kinda there for debugging but the functionalty remains the same, so i left it
void processGame(const buttons_t *button, Graphics_Context *ctx) {
    if (!state_drawn) {
        UI_drawGame(ctx, selectedChannel, colorValues);
        setLEDColor(colorValues[COL_RED],colorValues[COL_GREEN],colorValues[COL_BLUE]); // set led, they are init to 0
        state_drawn = true;
    }
    else { // else is true on every call expect the first
        uint8_t livePct = rawToCalibratedPercent(lastADCReading); // read live adc value for the channel

        UI_updatePercent(ctx, selectedChannel, livePct); // update the number
        uint8_t preview[3] = { colorValues[COL_RED],colorValues[COL_GREEN],colorValues[COL_BLUE]}; // store the current arrray of %

        preview[selectedChannel] = livePct;
        UI_drawPreview(ctx,preview[COL_RED], preview[COL_GREEN],preview[COL_BLUE]); // draw preview circle

        if (button->BB1tapped) { // change the channel logic
            colorValues[selectedChannel] = livePct;
            UI_updatePercent(ctx, selectedChannel, livePct);
            selectedChannel = (ColorChannel)((selectedChannel + 1) % 3);
            UI_drawCursor(ctx, selectedChannel);
        }
        else if (button->BB2tapped) { // update the LED logic
            colorValues[selectedChannel] = livePct;
            UI_updatePercent(ctx, selectedChannel, livePct);
            setLEDColor( colorValues[COL_RED],colorValues[COL_GREEN],colorValues[COL_BLUE]
            );
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
// convert raw adc reading to a percent
// the max value that i read was 7450, so i made that 100
uint8_t rawToCalibratedPercent(uint16_t raw) {
    const uint16_t ADC_MIN = 0;
    const uint16_t ADC_MAX = 7450;

    if (raw <= ADC_MIN) return 0;
    if (raw >= ADC_MAX) return 100;

    uint32_t span = ADC_MAX - ADC_MIN;
    uint32_t offset = raw - ADC_MIN;
    return (uint8_t)((offset * 100 + span/2) / span); // convert to percent
}
