//#define PART_TM4C1294NCPDT
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"


/***********************************
 * Ejemplo de manejo de un motor paso a paso conectado en el boosterpack 1 (pines PF1, PF2, PF3, PG0)
 * Si se pulsa un botón gira en un sentido, y si se pulsa el otro botón gira en sentido contrario.
 * Si no se pulsa ninguno, permanece en reposo.
 *************************************/

#define MSEC 40000
#define MaxEst 10


uint32_t Puerto[]={
        GPIO_PORTF_BASE,
        GPIO_PORTF_BASE,
        GPIO_PORTF_BASE,
        GPIO_PORTG_BASE,

};
uint32_t Pin[]={
        GPIO_PIN_1,
        GPIO_PIN_2,
        GPIO_PIN_3,
        GPIO_PIN_0,
        };

int Step[4][4]={1,0,0,0,
                0,0,0,1,
                0,0,1,0,
                0,1,0,0
};

#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))



int RELOJ;


volatile int secuencia=0;
volatile int posicion=0;     //Posicion relativa desde el arranque. entre 0 y 513
volatile int angulo=0;      //angulo relativo, en grados (para GUI_COMPOSER)
int paso=0;
volatile bool gui_cw=false, gui_ccw=false;   //variables para controlar desde GUI_COMPOSER

#define FREC 200 //Frecuencia en hercios del tren de pulsos: 5ms

void IntTimer(void);



int main(void)
{
	RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);       //Habilita T0

    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);   //T0 a 120MHz

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    //T0 periodico y conjunto (32b)

    TimerLoadSet(TIMER0_BASE, TIMER_A, (RELOJ/FREC)-1);

    TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer);


    IntEnable(INT_TIMER0A); //Habilitar las interrupciones globales de los timers

    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Habilitar las interrupciones de timeout

    IntMasterEnable();  //Habilitacion global de interrupciones

    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer0, 1, 2A y 2B



    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_0);


	while(1){

		if(B1_ON || gui_cw){	//Si se pulsa B1
		    paso=1;
		}
		else if(B2_ON || gui_ccw){	//Si se pulsa B2
			paso=-1;
		}
		else{
		    paso=0;
		}

	}
	return 0;
}


void IntTimer(void)
{
    int i;
    if(paso!=0){
    secuencia+=paso;
    if(secuencia==-1) secuencia=3;
    if(secuencia==4) secuencia=0;
    for(i=0;i<4;i++)  GPIOPinWrite(Puerto[i],Pin[i],Pin[i]*Step[secuencia][i]);
    posicion+=paso;
    if(posicion==-1) posicion=513;
    if(posicion==514) posicion=0;
    angulo=(int)((posicion*360)/513);
    }
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

}
