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
 *
 * Variante del EJ2, pero manejando los pines por interrupción.
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
int estado;


/*********************************************************
 * Rutina de interrupción del puerto J.
 * Leo los pines, y en cada caso hago la función necesaria,
 * (incrementar o decrementar el estado)
 * Establezco 20ms de debouncing en cada caso
 * Y debo borrar el flag de interrupción al final.
 ********************************************************/
void rutina_interrupcion(void)
{
    if(B1_ON)
    {
        while(B1_ON);
        SysCtlDelay(20*MSEC);
        estado++;
        if(estado==MaxEst) estado=MaxEst-1;
        GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0);
    }
    if(B2_ON)
      {
          while(B2_ON);
          SysCtlDelay(20*MSEC);
          estado--; if(estado==-1) estado=0;      //Decrementa el estado. Si menor que cero, satura.
          GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_1);
      }

}


/*****
 * Función para encender los leds usando la codificación que aparece en la matriz LED
 * La matriz LED hace de "circuito combinacional de salida", calculando el valor
 * de las salidas en cada estado
 */

void enciende_leds(uint8_t Est)
{
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1*LED[Est][0]);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[Est][1]);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4*LED[Est][2]);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[Est][3]);
}

int main(void)
{
    int estado_ant;
    //Fijar velocidad a 120MHz
    reloj=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //Habilitar los periféricos implicados: GPIOF, J, N
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //Definir tipo de pines
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4);	//F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1);	//N0 y N1: salidas
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);	//J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1

    estado=0;
    estado_ant=0;
    enciende_leds(estado);
    GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_FALLING_EDGE); // Definir tipo int: flanco bajada
    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);  // Habilitar pines de interrupción J0, J1
    GPIOIntRegister(GPIO_PORTJ_BASE, rutina_interrupcion);  //Registrar (definir) la rutina de interrupción
    IntEnable(INT_GPIOJ);                                   //Habilitar interrupción del pto J
    IntMasterEnable();                                      // Habilitar globalmente las ints

    while(1){

        if(estado!=estado_ant)  //Para no estar continuamente accediendo a los puertos...
        {
            estado_ant=estado;
            enciende_leds(estado);
        }
    }
}



