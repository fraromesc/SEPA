/*
 * main.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "driverlib2.h"
#include "utils/uartstdio.h"

#include "HAL_I2C.h"



// =======================================================================
// Function Declarations
// =======================================================================

#define NUM_SSI_DATA

int RELOJ;
#define MSEC 40000






int main(void) {
    char DIR=0;
    int Error=0;

    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, RELOJ);
    Error=Detecta_BP(1);
    if(Error){
        UARTprintf("\n BOOSTERPACK  detectado en posicion 1");

        Conf_Boosterpack(1, RELOJ);


        UARTprintf("\nHaciendo TEST en BOOSTERPACK 1\n");
        for(DIR=0;DIR<128;DIR++)
        {
            if(Test_I2C_Dir(DIR))
            {
                UARTprintf("\nSensor en 0x%02x \n",DIR);
            }
            else
            {
                UARTprintf(".");
            }
        }
        UARTprintf("\nHecho!");
    }
    else{
        UARTprintf("\n BOOSTERPACK no detectado en posicion 1");
    }
    Error=Detecta_BP(2);
    if(Error){
        UARTprintf("\n BOOSTERPACK  detectado en posicion 2");
        Conf_Boosterpack(2, RELOJ);


        UARTprintf("\nHaciendo TEST en BOOSTERPACK 2\n");
        for(DIR=0;DIR<128;DIR++)
        {
            if(Test_I2C_Dir(DIR))
            {
                UARTprintf("\nSensor en 0x%02x \n",DIR);

            }
            else
            {
                UARTprintf(".");
            }
        }
        UARTprintf("\nHecho!");
    }
    else{
        UARTprintf("\n BOOSTERPACK no detectado en posicion 1");
    }
}


