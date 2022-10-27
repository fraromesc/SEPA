#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"

/***********************************
 * Ejemplo de manejo del PWM. Se configura para que un servo en PM0 cambie
 * Mientras se tenga pulsado un botón, cambia la consigna del servo
 * Se usa para ello el PWM del Tierm 2-A, con un periodo de 50Hz,
 * y un duty cycle entre 1ms y 2ms aprox
 * NOTA: se debe poner el BP en la posición de Boosterpack2,
 * y conectar el servo en S4.
 * Se usan variables globales para poder cambiarlo en GUiComposer
 *************************************/

#define MSEC 40000
#define MaxEst 10


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))


volatile int Max_pos = 200; //4200; //3750
volatile int Min_pos = 100; //1300; //1875

int RELOJ, PeriodoPWM;
volatile int pos_100, pos_1000;
volatile int posicion;

int main(void)
{
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);       //Habilita T2
    TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_PIOSC);    //T2 a 16MHz
    TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM);

    // preescalado en modo PWM: bits MÄS SIGNIFICATIVOS.
    /************************************
     * Para calcular los valores para 50Hz:
     * 16MHz/50Hz=320000 ->4E200
     * Preescaler: 4
     * Periodo: E200 (57856)
     * Para calcular los duty cycles, entre 1ms y 2ms, se puede usar el reloj de 16Mz directamente
     * 1ms: 16000, 2ms: 32000
     ************************************/
    TimerLoadSet(TIMER2_BASE, TIMER_A, 57855 );   //50Hz
    TimerPrescaleSet(TIMER2_BASE, TIMER_A, 4);     // T2 preescalado a 160: 100kHz

    GPIOPinConfigure(GPIO_PM0_T2CCP0);          //Configurar el pin a PWM
    GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_0);

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);


    pos_100=50; //Inicialmente, 50%

    TimerUpdateMode(TIMER2_BASE, TIMER_A,TIMER_UP_LOAD_TIMEOUT);
    TimerControlLevel(TIMER2_BASE, TIMER_A, 1);
    TimerEnable(TIMER2_BASE, TIMER_A);

    while(1){
        if(B1_ON){	//Si se pulsa B1
            SysCtlDelay(10*MSEC);
            if(pos_100<100) pos_100++;	//Incrementa el periodo, saturando
        }
        if(B2_ON){	//Si se pulsa B2
            SysCtlDelay(10*MSEC);
            if(pos_100>0) pos_100--;//Decrementa el periodo, saturando
        }
        posicion=16000+pos_100*160; //1ms+x*1ms (x entre 0 y 1)

        TimerMatchSet(TIMER2_BASE, TIMER_A, posicion);


    }
    return 0;
}


