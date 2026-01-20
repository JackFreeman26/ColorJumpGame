/*
 * ColorPWM.c
 *
 *  Created on: May 3, 2025
 *      Author: jwf47
 */
#include "ColorPWM.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define PWM_PERIOD 1000
static Timer_A_PWMConfig pwmConfig;
////////////////////////////////////////////////////////////////////////////////////////////
void initColorPWM() {
    // route led pin to timer
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);

    // pwm config
    pwmConfig.clockSource        = TIMER_A_CLOCKSOURCE_SMCLK;
    pwmConfig.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    pwmConfig.timerPeriod        = PWM_PERIOD;
    pwmConfig.compareOutputMode  = TIMER_A_OUTPUTMODE_RESET_SET;
    pwmConfig.dutyCycle          = 0;

    // red channel
    pwmConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    // green channel
    pwmConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    // blue channel
    pwmConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);
}

////////////////////////////////////////////////////////////////////////////////////////////
void setLEDColor(uint8_t rPct, uint8_t gPct, uint8_t bPct) {
   // set 100 as max
    if (rPct > 100) rPct = 100;
    if (gPct > 100) gPct = 100;
    if (bPct > 100) bPct = 100;

    // convert percent to timer ticks
    uint16_t rTicks = (uint32_t)rPct * PWM_PERIOD / 100;
    uint16_t gTicks = (uint32_t)gPct * PWM_PERIOD / 100;
    uint16_t bTicks = (uint32_t)bPct * PWM_PERIOD / 100;
    // compare ticks and output the result
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, rTicks);
    Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1, gTicks);
    Timer_A_setCompareValue(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1, bTicks);
}



