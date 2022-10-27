#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"

/***********************************
 * Ejemplo de manejo del PWM. Se configura para que un servo en PF1 cambie
 * Mientras se tenga pulsado un botón, cambia la consigna del servo
 * Se usa para ello el PWM1, con un periodo de 50Hz, y un duty cycle entre 1ms y 2ms aprox
 * Se usan variables globales para poder cambiarlo en GUiComposer
 *************************************/

#define MSEC 40000
#define MaxEst 10


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))


volatile int Max_pos = 4200; //3750
volatile int Min_pos = 1300; //1875

int RELOJ, PeriodoPWM;
volatile int pos_100, pos_1000;
volatile int posicion;

int main(void)
{
	int i,j;
	RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

	PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_64);	// al PWM le llega un reloj de 1.875MHz

	GPIOPinConfigure(GPIO_PF1_M0PWM1);			//Configurar el pin a PWM
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

    //Configurar el pwm0, contador descendente y sin sincronización (actualización automática)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	PeriodoPWM=37499; // 50Hz  a 1.875MHz
    pos_100=50; //Inicialmente, 50%
    pos_1000=500;
    posicion=Min_pos+((Max_pos-Min_pos)*pos_100)/100;
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PeriodoPWM); //frec:50Hz
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, posicion); 	//Inicialmente, 1ms

    PWMGenEnable(PWM0_BASE, PWM_GEN_0);		//Habilita el generador 0

    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT , true);	//Habilita la salida 1

	while(1){
		if(B1_ON){	//Si se pulsa B1
			SysCtlDelay(10*MSEC);
			if(pos_100<100) pos_100++;	//Incrementa el periodo, saturando
		}
		if(B2_ON){	//Si se pulsa B2
			SysCtlDelay(10*MSEC);
			if(pos_100>0) pos_100--;//Decrementa el periodo, saturando
		}
//		pos_100=pos_1000/10;
		posicion=Min_pos+((Max_pos-Min_pos)*pos_100)/100;
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, posicion);
	}
	return 0;
}


