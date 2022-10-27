#include <stdint.h>
#include <stdbool.h>

#include "driverlib2.h"


#define MSEC 40000// Multiplicador para que los delays estén en ms
//Definición de los pines de los botones
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))
//Definicion estados, que utilizaremos dentro del while 1
#define reposo 0
#define ang_min 1
#define ang_max 2
#define espera 3
//Definición de SLEEP para poder debugear el programa
#define SLEEP SysCtlSleep()
//#define SLEEP SysCtlSleepFake()
//Valores máximo y mínimo que llegarán PWM, y serán utilizados en la función giro
volatile int Max_pos = 4700;//4200; //3750
volatile int Min_pos = 1000; //1875

int RELOJ, PeriodoPWM, Flag_ints;
//Función que coloca el servo en el ángulo deseado, introduciendo como parametro el porcentaje del rango posible.
void giro (int pos)
{
    int posicion=Min_pos+((Max_pos-Min_pos)*pos)/100;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, posicion);   //Inicialmente, 1ms
}
//Función del SLEEP fake, utilizar en caso de que dé problemas al usar el debugger
void SysCtlSleepFake(void)
{
 while(!Flag_ints);
 Flag_ints=0;
}

int contador; //Variable que utlizamos en la rutina de interrupción del timer para contar las veces que se activa
//Interrupción del Timer0
void IntTimer0(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    Flag_ints=1;
    contador ++;
}

int main(void)
{   //Inicialización de la variable estado para la máquina de estados
    int estado = reposo;

    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    //Activación de los pines necesarios
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1


    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);       //Habilita T0
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);   //T0 a 120MHz
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    //T0 periodico y conjunto (32b)
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/20 -1);
    TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer0);
    IntEnable(INT_TIMER0A); //Habilitar las interrupciones globales de los timers
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Habilitar las interrupciones de timeout
    IntMasterEnable();  //Habilitacion global de interrupciones
    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer0

    PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_64);   // al PWM le llega un reloj de 1.875MHz

    GPIOPinConfigure(GPIO_PG0_M0PWM4);          //Configurar los pines a PWM
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0);

    //Configurar el PWM4, contador descendente y sin sincronización (actualización automática)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    PeriodoPWM=37499; // 50Hz  a 1.875MHz
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PeriodoPWM); //frec:50Hz
    giro(50); //Posicion inicial del servo

    PWMGenEnable(PWM0_BASE, PWM_GEN_2);     //Habilita el generador 0
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT , true);    //Habilita la salida 1
    //Activación de los periféricos necesarios durante el modo SLEEP
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);

    while(1){

        SLEEP;
        //Máquina de estados
      switch(estado)
        {
        case reposo: //En el caso de que estemos en la posición inicial de reposo
            if(B1_ON) estado = ang_min; //Pulsando el botón 1 el servo se mueve a la izquierda
            else if (B2_ON) estado = ang_max; //Pulsando el botón 2 el servo se mueve a la derecha
            contador = 0;
            break;

        case ang_min: //En el caso de que estemos en la posición izquierda, de ángulo mínimo
            giro (0); //Llamada a la función giro
            estado = espera;
            contador = 0;
            break;
        case ang_max: //En el caso de que estemos en la posición derecha, de ángulo máximo
            giro (100);//Llamada a la función giro
            estado = espera;
            contador = 0;
            break;
        case espera: //Tras hacia la izquierda o hacia la derecha y esperar un tiempo, volvemos al reposo
            if (contador >= 20)
            {
                estado = reposo;
                giro(50);
            }
        }


    }
}


