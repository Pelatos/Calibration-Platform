/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: ert_main.c
 *
 * Code generated for Simulink model 'Calibration_procedure'.
 *
 * Model version                  : 1.79
 * Simulink Coder version         : 8.8.1 (R2015aSP1) 04-Sep-2015
 * TLC version                    : 8.8 (Sep  6 2015)
 * C/C++ source code generated on : Wed Feb 17 12:32:00 2016
 *
 * Target selection: realtime.tlc
 * Embedded hardware selection: Atmel->AVR
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "Calibration_procedure.h"
#include "Arduino.h"
#include "io_wrappers.h"
#define INIT_TIMER_VAL                 131
#define SETUP_PRESCALER                TCCR2B |= ((1<<CS22) | (1<<CS21)); TCCR2B &= ~(1<<CS20)
#define INTERRUPT_VECTOR               TIMER2_OVF_vect
#define DISABLE_TIMER                  TIMSK2 &= ~(1<<TOIE2)
#define ENABLE_TIMER                   TIMSK2 |= (1<<TOIE2)
#define TIMER_NORMAL_MODE              TCCR2A &= ~((1<<WGM21) | (1<<WGM20)); TCCR2B &= ~(1<<WGM22)
#define RESET_TIMER                    TCNT2 = INIT_TIMER_VAL

volatile uint8_T scheduler_counter = 0;

#define SCHEDULER_COUNTER_TARGET       5

volatile int IsrOverrun = 0;
boolean_T isRateRunning[2] = { 0, 0 };

boolean_T need2runFlags[2] = { 0, 0 };

/*
 * The timer interrupt handler (gets invoked on every counter overflow).
 */
ISR(INTERRUPT_VECTOR)
{
  RESET_TIMER;
  if ((++scheduler_counter) == SCHEDULER_COUNTER_TARGET) {
    scheduler_counter = 0;
    rt_OneStep();
  }
}

/*
 * Configures the base rate interrupt timer
 */
static void arduino_Timer_Setup()
{
  // Sets up the timer overflow interrupt.
  RESET_TIMER;

  // Initially disable the overflow interrupt (before configuration).
  DISABLE_TIMER;

  // Set the timer to normal mode.
  TIMER_NORMAL_MODE;

  // Set the prescaler.
  SETUP_PRESCALER;

  // Everything configured, so enable the overflow interrupt.
  ENABLE_TIMER;
}

void rt_OneStep(void)
{
  boolean_T eventFlags[2];

  /* Check base rate for overrun */
  if (isRateRunning[0]++) {
    IsrOverrun = 1;
    isRateRunning[0]--;                /* allow future iterations to succeed*/
    return;
  }

  sei();

  /*
   * For a bare-board target (i.e., no operating system), the
   * following code checks whether any subrate overruns,
   * and also sets the rates that need to run this time step.
   */
  Calibration_procedure_output();

  /* Get model outputs here */
  Calibration_procedure_update();
  cli();
  isRateRunning[0]--;
  if (eventFlags[1]) {
    if (need2runFlags[1]++) {
      IsrOverrun = 1;
      need2runFlags[1]--;              /* allow future iterations to succeed*/
      return;
    }
  }

  if (need2runFlags[1]) {
    if (isRateRunning[1]) {
      /* Yield to higher priority*/
      return;
    }

    isRateRunning[1]++;
    sei();
    switch (1) {
     default:
      break;
    }

    cli();
    need2runFlags[1]--;
    isRateRunning[1]--;
  }
}

int_T main(void)
{
  init();

#ifdef _RTT_USE_SERIAL0_

  Serial_begin(0, 9600);
  Serial_write(0, "***starting the model***", 26);

#endif

  Calibration_procedure_initialize();
  arduino_Timer_Setup();

  /* The main step loop */
  while (rtmGetErrorStatus(Calibration_procedure_M) == (NULL)) {
  }

  Calibration_procedure_terminate();

  /* Disable Interrupts */
  cli();
  return 0;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
