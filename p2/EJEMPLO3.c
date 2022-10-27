#include <stdint.h>
#include <stdbool.h>
#include "driverlib2.h"

 void IntTimer0(void);
 void IntTimer1(void);
 void IntTimer2(void);
 void IntTimer3(void);


 int main(void)
{
	uint32_t periodo1,periodo2,periodo3,periodo4;
	uint32_t Reloj;

	Reloj = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);


	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);       //Habilita T0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);       //Habilita T1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);       //Habilita T2
	TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);   //T0 a 120MHz
	TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM);   //T1 a 120MHz
	TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_PIOSC);    //T2 a 16MHz

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    //T0 periodico y conjunto (32b)
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);    //T1 igual...
    TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR| TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC);
    //T2 configurado como Split (16bit), periodicos ambas mitades

	TimerPrescaleSet(TIMER2_BASE, TIMER_A,255);     // T2 preescalado a 256: 62.5kHz
	TimerPrescaleSet(TIMER2_BASE, TIMER_B,255);
    //  Cálculo de los periodos. T0 y T1 van a 120MHz, mientras que T2 va a 62.5k

    periodo1 = Reloj/2; //0.5s
    periodo2 = Reloj/4; //0.25s
    periodo3 = 15625;    // T/4:0.25s
    periodo4 = 31250;    // T/2: 0.5s

    TimerLoadSet(TIMER0_BASE, TIMER_A, periodo1 -1);
	TimerLoadSet(TIMER1_BASE, TIMER_A, periodo2 -1);
	TimerLoadSet(TIMER2_BASE, TIMER_A, periodo3 -1);
	TimerLoadSet(TIMER2_BASE, TIMER_B, periodo4 -1);
	TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer0);
	TimerIntRegister(TIMER1_BASE,TIMER_A,IntTimer1);
	TimerIntRegister(TIMER2_BASE,TIMER_A,IntTimer2);
	TimerIntRegister(TIMER2_BASE,TIMER_B,IntTimer3);


	IntEnable(INT_TIMER0A); //Habilitar las interrupciones globales de los timers
	IntEnable(INT_TIMER1A);
	IntEnable(INT_TIMER2A);
	IntEnable(INT_TIMER2B);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Habilitar las interrupciones de timeout
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT |TIMER_TIMB_TIMEOUT);

	IntMasterEnable();  //Habilitacion global de interrupciones

	TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer0, 1, 2A y 2B
	TimerEnable(TIMER1_BASE, TIMER_A);
	TimerEnable(TIMER2_BASE, TIMER_A);
	TimerEnable(TIMER2_BASE, TIMER_B);

	while(1)
	{
	    //Bucle infinito en el que no se hace nada
	}
}

void IntTimer0(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Borra flag
	// Si el pin está a 1 lo pone a 0, y viceversa
	if(GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1))
	{
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
	}
	else
	{
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
	}
}


void IntTimer1(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	// Read the current state of the GPIO pin and
	// write back the opposite state
	if(GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0))
	{
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
	}
	else
	{
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
	}
}

void IntTimer2(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	// Read the current state of the GPIO pin and
	// write back the opposite state
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
	}
	else
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
	}
}

void IntTimer3(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER2_BASE, TIMER_TIMB_TIMEOUT);

	// Read the current state of the GPIO pin and
	// write back the opposite state
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
	}
	else
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
	}
}

