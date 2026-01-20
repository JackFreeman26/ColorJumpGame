// Starter code for Project 3. Good luck!
////////////////////////////////////////////////////////////////////////////////////////////
// TI includes
#include <LcdDriver/Crystalfontz128x128_ST7735.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
////////////////////////////////////////////////////////////////////////////////////////////
// includes
#include <stdio.h>
#include "HAL/Button.h"
#include "HAL/LED.h"
#include "HAL/Timer.h"
#include "FSM.h"
#include "ColorPWM.h"
////////////////////////////////////////////////////////////////////////////////////////////
// main functions declarations
void initialize(); // init everything
void initGraphics(Graphics_Context *g_sContext_p);
void main_loop(Graphics_Context *g_sContext_p); // main logic
void sleep(); // sleep mode logic
static void initADC();
static void initADCTimer();
////////////////////////////////////////////////////////////////////////////////////////////
extern volatile bool     adcEvent;      // global for adc interupt
extern volatile uint16_t lastADCReading;
#define AdcThres 200 // threshold under/over becasue adc oscialtes
////////////////////////////////////////////////////////////////////////////////////////////
// main
int main() {

  initialize();

  Graphics_Context g_sContext;
  initGraphics(&g_sContext);
  buttons_t noButtons = {0}; // fsm needs a button pointer, so i just create a blank one, this is just to print the title screen

  FSM_step(&noButtons, &g_sContext); // call fsm before loop so the title screen prints

  while (1) {
    sleep();
    main_loop(&g_sContext);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
// main logic loop
void main_loop(Graphics_Context *g_sContext_p) {

   buttons_t buttons = updateButtons(); // reset buttons
  // Do not remove this statement. This is the non-bocking code for this project
  if (buttons.JSBtapped){
      Toggle_LL1();
  }

  bool newADC = adcEvent; // adc flag
  adcEvent = false;

  // this nested if is because the button taps werent being recoginzed by the FSM, also helped with sleep mode effiecentcy
  if (current_state == ST_GAME) {
      if ( buttons.LB1tapped || buttons.LB2tapped || buttons.BB1tapped || buttons.BB2tapped || buttons.JSBtapped  || newADC) // interrupt
      {
          FSM_step(&buttons, g_sContext_p);
      }
  }
  else {
      FSM_step(&buttons, g_sContext_p);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
// lower power mode function
void sleep() {
  TurnOn_LLG();
  PCM_gotoLPM0();
  TurnOff_LLG();
}
////////////////////////////////////////////////////////////////////////////////////////////
// init everything
void initialize() {
  WDT_A_hold(WDT_A_BASE);
  InitSystemTiming();

  initLEDs();
  initButtons();

  initADC();
  FSM_init();
  initColorPWM();
  initADCTimer();

}
////////////////////////////////////////////////////////////////////////////////////////////
// init ADC
static void initADC() {

  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4 ,GPIO_PIN7, GPIO_TERTIARY_MODULE_FUNCTION);

  ADC14_enableModule();
  ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);
  ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
  ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A6, false);
  ADC14_enableConversion();

  ADC14_enableInterrupt(ADC_INT0);
  Interrupt_enableInterrupt(INT_ADC14);
}
////////////////////////////////////////////////////////////////////////////////////////////
// init graphics
void initGraphics(Graphics_Context *g_sContext_p) {
  Crystalfontz128x128_Init();
  Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
  Graphics_initContext(g_sContext_p, &g_sCrystalfontz128x128,
                       &g_sCrystalfontz128x128_funcs);
  Graphics_setFont(g_sContext_p, &g_sFontFixed6x8);
  Graphics_setForegroundColor(g_sContext_p, GRAPHICS_COLOR_WHITE);
  Graphics_setBackgroundColor(g_sContext_p, GRAPHICS_COLOR_BLACK);
  Graphics_clearDisplay(g_sContext_p);
}
////////////////////////////////////////////////////////////////////////////////////////////
// timer to check ADC
static void initADCTimer() {
    Timer32_initModule(TIMER32_1_BASE,TIMER32_PRESCALER_1,TIMER32_32BIT,TIMER32_PERIODIC_MODE);
    Timer32_setCount(TIMER32_1_BASE,SYSTEM_CLOCK / 200); // i played around with different values, this seemed to make the light blink the least

    Timer32_clearInterruptFlag(TIMER32_1_BASE);
    Timer32_enableInterrupt(TIMER32_1_BASE);
    Interrupt_enableInterrupt(INT_T32_INT2);
    Timer32_startTimer(TIMER32_1_BASE, false);
}
////////////////////////////////////////////////////////////////////////////////////////////
// adc interrupt handler
void ADC14_IRQHandler() {
  uint16_t v = ADC14_getResult(ADC_MEM0);
  ADC14_clearInterruptFlag(ADC_INT0);

  if ((v > lastADCReading + AdcThres) || (v + AdcThres < lastADCReading)) { // created a threshold becasue adc osccialtes and was always calling true to adc event
      lastADCReading = v;
      adcEvent = true;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
// timer 32 interrupt
void T32_INT2_IRQHandler() {
    Timer32_clearInterruptFlag(TIMER32_1_BASE);
    ADC14_toggleConversionTrigger();
}
////////////////////////////////////////////////////////////////////////////////////////////
