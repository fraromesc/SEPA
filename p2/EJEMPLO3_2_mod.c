#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"

/***********************************
 * Ejemplo de manejo del PWM. Se configura para que el LED en PF0 cambie
 * cada vez que se pulsa un botón, sube o baja la intensidad luminosa
 * Se usa para ello el PWM0, con un periodo de 1KHz, y un duty cycle entre 10% y 90%
 * Se cambia el duty MIENTRAS estén los botones apretados, usando un timer
 *
 *************************************/

#define MSEC 40000
#define MaxEst 10


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))


void IntTimer0(void);


int RELOJ, PeriodoPWM, DutyPWM, MaxDuty, MinDuty;
int main(void)
{
	int i,j;
	RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

	PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_4);	// al PWM le llega un reloj de 30MHz

	GPIOPinConfigure(GPIO_PF0_M0PWM0);			 //Conectar el pin a PWM
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0); //Tipo de pin PWM

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

    //Configurar el pwm0, contador descendente y sin sincronización (actualización automática)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	PeriodoPWM=29999; // 1kHz a 30M
	DutyPWM=PeriodoPWM/2;
//	MaxDuty=PeriodoPWM-5*(PeriodoPWM/100);
//	MinDuty=5*(PeriodoPWM/100);
	MaxDuty=0.95*PeriodoPWM;
	MinDuty=0.05*PeriodoPWM;

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PeriodoPWM); //frec:1kHz
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, DutyPWM); 	//Inicialmente, un 50%

    PWMGenEnable(PWM0_BASE, PWM_GEN_0);		//Habilita el generador 0

    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT , true);	//Habilita la salida 0

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);       //Habilita T0

    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);   //T0 a 120MHz


    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    //T0 periodico y conjunto (32b)


    TimerLoadSet(TIMER0_BASE, TIMER_A,1199999 ); //10ms

    TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer0);


    IntEnable(INT_TIMER0A); //Habilitar las interrupciones globales de los timers

    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Habilitar las interrupciones de timeout


    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer0, 1, 2A y 2B

	while(1);
	return 0;
}



void IntTimer0(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Borra flag
    // Si el pin está a 1 lo pone a 0, y viceversa

    if(B1_ON)
    {
        if(DutyPWM<MaxDuty) DutyPWM+=PeriodoPWM/100;     //Incrementa el duty cycle, saturando a 90%
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, DutyPWM);
    }
    if(B2_ON)
       {
        if(DutyPWM>MinDuty) DutyPWM-=PeriodoPWM/100;    //Decrementa el duty cycle, saturando a 10%
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, DutyPWM);
       }
}


