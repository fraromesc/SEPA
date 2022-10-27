#include <stdint.h>
#include <stdbool.h>
//Incluimos las librerías necesarias

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
#define MaxEst 3 //Definimos globalmente el estado máximo = 3, considerando cada uno de los estados que se especifican en el enunciado
//Definimos tambien como variables globales los botones B1 y B2 para aumentar y disminuir el estado, respectivamente
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))

uint32_t reloj=0;

int main(void)
{
    int estado;
    //Fijar la velocidad del reloj a 120MHz
    reloj=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //Habilitar los periféricos implicados en el eje: GPIOF, GPIOJ, GPION
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //Definir tipo de pines, los botones como entradas y los leds como salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4);	//F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1);	//N0 y N1: salidas
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);	//J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1

    estado=1; // Variable para cada uno de los modos de funcionamientos: E1, E2, E3.

    while(1){
        if(!(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))){     //Si se aprieta el botón 1
                    SysCtlDelay(10*MSEC);
                    while(!(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)));  //Debouncing...
                    SysCtlDelay(10*MSEC);
                    estado++; if(estado>MaxEst) estado=1;       //Incrementa el estado. Si estado > 3, vuelve al estado 1
                }
                if( !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))){        //Si se aprieta el botón 2
                    SysCtlDelay(10*MSEC);
                    while( !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))); //Debouncing...
                    SysCtlDelay(10*MSEC);
                    estado--; if(estado<1) estado=3;        //Decrementa el estado. Si menor que uno, redirige al estado 3
                }

       if (estado == 1) //Si estamos en el estado 1:
       {
           //Encendemos todos los pines de los leds
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
           //Esperamos 0.1s
           SysCtlDelay(100*MSEC);
           //Apagamos todos los pines de los leds
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
           //Esperamos los 0.9s restantes, posteriormente se volverá al inicio del ciclo
           SysCtlDelay(900*MSEC);

       }
       else if (estado == 2) //Si estamos en el estado 2:
       {
           //Comenzamos con todos los pines de los leds apagados
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
           SysCtlDelay(1000*MSEC);
           //Encendemos progresivamente cada pin cada segundo
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
           SysCtlDelay(1000*MSEC);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
           SysCtlDelay(1000*MSEC);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
           SysCtlDelay(1000*MSEC);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
           //Espera final de 3 segundos antes de volver a empezar el ciclo
           SysCtlDelay(3000*MSEC);
       }
       else if (estado == 3)//Si estamos en el estado 3:
       {
           //Establecemos la primera configuracion de pines (1010)
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);
           //Esperamos 0.5s
           SysCtlDelay(500*MSEC);
           //Establecemos la segunda configuracion de pines (0101)
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
           //Esperamos los 0.5s restantes antes de repetir el ciclo
           SysCtlDelay(500*MSEC);

       }
       else  //En caso de error
           estado == 1;
}}



