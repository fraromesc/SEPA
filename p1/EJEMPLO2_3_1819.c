#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"
/************************************************************
 * Primer ejemplo de manejo de pines de e/s, usando el HW de la placa
 * Los pines se definen para usar los leds y botones:
 * 		LEDS: F0, F4, N0, N1
 * 		BOTONES: J0, J1
 * Cuando se pulsa (y se suelta)un botón, cambia de estado,
 * entre los definidos en la matriz LED. El primer botón incrementa el estado
 * y el segundo lo decrementa. Al llegar al final, se satura.
 ************************************************************/


#define MSEC 40000 //Valor para 1ms con SysCtlDelay()
#define MaxEst 7


#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))


int LED[MaxEst][4]={0,0,0,1,
		0,0,1,1,
		0,1,1,1,
		1,1,1,1,
		1,1,1,0,
		1,1,0,0,
		1,0,0,0

};



uint32_t reloj=0;

uint32_t tiempo=0;
uint8_t fin_periodo=0;

#define PERIODO 50  //Periodo=50ms
void TickInt(void)
{
    tiempo++;
    if(tiempo==PERIODO){
        tiempo=0;
        fin_periodo=1;
    }

}

int main(void)
{
    int estado;
    //Fijar velocidad a 120MHz
    reloj=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //Habilitar los periféricos implicados: GPIOF, J, N
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //Definir tipo de pines
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4);	//F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1);	//N0 y N1: salidas
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);	//J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1

    estado=0;

    SysTickPeriodSet(120000);       //Fijar periodo de Tick a 1ms
    SysTickIntRegister(TickInt);    //Apuntar a rutina de interrupcion
    SysTickEnable();                //Habilitar SysTick
    SysTickIntEnable();             //Habilitar interrupcion del SysTick
    IntMasterEnable();              //Habilitar globalmente las ints.

    while(1){
        if(fin_periodo)             //Solo se ejecuta una vez por periodo
        {
            fin_periodo=0;

            if(!(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))){		//Si se aprieta el boton 1
/* NO es necesario el DEBONCING al hacerlo de manera sincrona*/
//                SysCtlDelay(10*MSEC);
                while(!(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)));	//Debouncing...
//                SysCtlDelay(10*MSEC);
                estado++; if(estado==MaxEst) estado=MaxEst-1;		//Incrementa el estado. Si máximo, satura
            }
            if( !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))){		//Si se aprieta el botón 2
/* NO es necesario el DEBONCING al hacerlo de manera sincrona*/
//                SysCtlDelay(10*MSEC);
                while( !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)));	//Debouncing..
//                SysCtlDelay(10*MSEC);
                estado--; if(estado==-1) estado=0;		//Decrementa el estado. Si menor que cero, satura.
            }

            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1*LED[estado][0]);
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[estado][1]);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4*LED[estado][2]);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[estado][3]);
        }
    }
}



