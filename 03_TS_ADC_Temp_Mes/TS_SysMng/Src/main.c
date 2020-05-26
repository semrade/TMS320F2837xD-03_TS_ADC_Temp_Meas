/***********************************************************************************
 * File                     :main.c
 *
 * Title                    :
 *
 * Author                   :Tarik SEMRADE
 *
 * Description              :CPU temperature measurement tutorial project
 *
 * Version                  :
 *
 * Copyright (c) 2020 Tarik SEMRADE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 **********************************************************************************/

/**********************************************************************************
 *  Included Files
 *
 *********************************************************************************/
#include "F28x_Project.h"
#include "device.h"
#include "main.h"
#include "TS_SysMng_Adc.h"

/**********************************************************************************
 *  Defines
 *
 *********************************************************************************/
#define ADC_RESOLUTION_SIGNALMODE 4095

/**********************************************************************************
 *  Global Variables
 *
 *********************************************************************************/
Uint16 s_u16SensorSample;
int16 s_i16SensorTemp;

/**********************************************************************************
 * \function:       main
 * \brief           main `0` numbers
 * \param[in]       void
 * \return          void
 **********************************************************************************/
void main(void)
{
    /***************************CPU Initialization*********************************/
    /* Set up system flash and turn peripheral clocks */
    InitSysCtrl();

    /* GPIO Init */
    InitGpio();

    /* Globally disable maskable CPU interrupts */
    DINT;

    /* Clear and disable all PIE interrupts */
    InitPieCtrl();

    /* Individually disable maskable CPU interrupts */
    IER = 0x0000;

    /* Clear all CPU interrupt flags */
    IFR = 0x0000;

    /* Populate the PIE interrupt vector table with */
    InitPieVectTable();

    /***********************Interrupt linking functions*****************************/
    /* Timer0 ISR function linking */
    EALLOW;
    PieVectTable.TIMER0_INT = &Timer0_ISR;
    EDIS;
    /************************Peripheral Initialization*****************************/
    /* ADC Reset */
    TS_SysMng_AdcConfig();

    /* Initialize all timer with default values */
    InitCpuTimers();

    /* 200MHz CPU Freq, 50 uSeconds Period (in uSeconds) */
    ConfigCpuTimer(&CpuTimer0, 200, 50);

    /* Enable TINT0 in the PIE: Group 1 interrupt 7 */
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    /* Enable group 1 interrupts */
    IER |= M_INT1;

    /* Enable Global interrupt INTM */
    EINT;

    /* Initialize the temperature sensor */
    InitTempSensor(3.0);

    /* Start Timer0 */
    StartCpuTimer0();

    /* Main loop */
    while (1)
    {
        asm(" NOP");
    }

}
/**********************************************************************************
 * \function:       Timer0_ISR
 * \brief           `0` Param
 * \param[in]       void
 * \return          void
 **********************************************************************************/
interrupt void Timer0_ISR(void)
{

    /* Clear Timer interrupt flag */
    CpuTimer0Regs.TCR.bit.TIF = 0;

    /* Wait acquisition ending*/
    TS_SysMng_AdcAcqEnding();

    /* Store the ADC result Lsb */
    s_u16SensorSample = AdcaResultRegs.ADCRESULT0;

    /* Convert to temp using GetTemperaturesC function */
    s_i16SensorTemp = GetTemperatureC(s_u16SensorSample);

    /* Increment timer0 interrupt counter */
    CpuTimer0.InterruptCount++;

    /* Acknowledge this interrupt to receive more interrupts from group 1 */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}
