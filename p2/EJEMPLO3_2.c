#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"

/***********************************
 * Ejemplo de manejo del PWM. Se configura para que el LED en PF0 cambie
 * cada vez que se pulsa un botón, sube o baja la intensidad luminosa
 * Se usa para ello el PWM0, con un periodo de 1KHz, y un duty cycle entre 10% y 90%
 *
 *************************************/

#define MSEC 40000
#define MaxEst 10


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))




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
    MaxDuty=0.95*PeriodoPWM;
    MinDuty=0.05*PeriodoPWM;

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PeriodoPWM); //frec:1kHz
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, DutyPWM); 	//Inicialmente, un 50%

    PWMGenEnable(PWM0_BASE, PWM_GEN_0);		//Habilita el generador 0

    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT , true);	//Habilita la salida 0

	while(1){
		if(B1_ON){	//Si se pulsa B1
			SysCtlDelay(10*MSEC);
			while(B1_ON);
			SysCtlDelay(10*MSEC);
			if(DutyPWM<MaxDuty) DutyPWM+=PeriodoPWM/25; 	//Incrementa el duty cycle, saturando a 95%
		    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, DutyPWM);
		}
		if(B2_ON){	//Si se pulsa B2
			SysCtlDelay(10*MSEC);
			while(B2_ON);
			SysCtlDelay(10*MSEC);
            if(DutyPWM>MinDuty) DutyPWM-=PeriodoPWM/25;    //Decrementa el duty cycle, saturando a 5%
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, DutyPWM);
		}
	}
	return 0;
}


