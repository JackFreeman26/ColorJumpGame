/*
 * FSM.h
 *
 *  Created on: Apr 24, 2025
 *      Author: jwf47
 */

#ifndef FSM_H_
#define FSM_H_
////////////////////////////////////////////////////////////////////////////////////////////
#include "HAL/Button.h"
#include <ti/grlib/grlib.h>
////////////////////////////////////////////////////////////////////////////////////////////
// FSM states
typedef enum {
    ST_TITLE,
    ST_MENU,
    ST_INSTR,
    ST_GAME
} AppState;

extern AppState current_state;
////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
  COL_RED   = 0,
  COL_GREEN = 1,
  COL_BLUE  = 2
} ColorChannel;
////////////////////////////////////////////////////////////////////////////////////////////

void FSM_init(); // init the fsm with the starting values

void FSM_step(const buttons_t *btn, Graphics_Context *ctx); // fsm state logic

void processTitle(const buttons_t *button,Graphics_Context *ctx); // title screen logic

void processMenu(const buttons_t *button,Graphics_Context *ctx); // menu screen logic

void processInstr(const buttons_t *button,Graphics_Context *ctx); // instr screen logic

void processGame(const buttons_t *button,Graphics_Context *ctx); // game screen logic



#endif /* FSM_H_ */
