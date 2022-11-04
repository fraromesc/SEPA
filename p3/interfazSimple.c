/*
 * main.c
 */


#include <stdint.h>
#include <stdbool.h>
#include "driverlib2.h"

#include "FT800_TIVA.h"


//DEFINICION DE COLORES
#define rojo 163,47,41
#define amarillo 255,255,153
#define gris 128,128,128
#define verde 128,255,0
#define verde2 153,255,51
#define negro 0,0,0
#define blanco 255,255,255
#define divHor HSIZE/26
#define divVer VSIZE/12

//DEFINICIÓN ESTADOS DE LA MAQUINA DE ESTADOS
#define espera 0
#define l1 1
#define l2 2
#define l3 3
#define l4 4
#define b1 5
#define b2 6

//DEFINICIÓN COMPROBACION BOTONES
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))

//DEFINICION DE ESTADOS ENCENDIDO Y APAGADO DE LOS LEDS
#define L4_ON GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,GPIO_PIN_1)
#define L4_OFF GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,0)
#define L3_ON GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,GPIO_PIN_0)
#define L3_OFF GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,0)

#define L2_ON GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_PIN_4)
#define L2_OFF GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,0)
#define L1_ON GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_0)
#define L1_OFF GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0)


#define dibL1 Boton((4)*divHor, VSIZE-4*divVer, 3*divHor, 2*divVer,27, "L1")
#define dibL2 Boton((5+4)*divHor, VSIZE-4*divVer, 3*divHor, 2*divVer,27, "L2")
#define dibL3 Boton((10+4)*divHor, VSIZE-4*divVer, 3*divHor, 2*divVer,27, "L3")
#define dibL4 Boton((15+4)*divHor, VSIZE-4*divVer, 3*divHor, 2*divVer,27, "L4")

//DEFINICION PARA DIBUJO
#define alturaCuadro 20
#define largoCuadro 50

// =======================================================================
// Function Declarations
// =======================================================================
#define dword long
#define byte char



#define PosMin 750
#define PosMax 1000

#define XpMax 286
#define XpMin 224
#define YpMax 186
#define YpMin 54

unsigned int Yp=120, Xp=245;
// =======================================================================
// Variable Declarations
// =======================================================================

char chipid = 0;                        // Holds value of Chip ID read from the FT800

unsigned long cmdBufferRd = 0x00000000;         // Store the value read from the REG_CMD_READ register
unsigned long cmdBufferWr = 0x00000000;         // Store the value read from the REG_CMD_WRITE register
unsigned int t=0;
// ############################################################################################################
// User Application - Initialization of MCU / FT800 / Display
// ############################################################################################################

unsigned long POSX, POSY, BufferXY;
unsigned long POSYANT=0;
unsigned int CMD_Offset = 0;
unsigned long REG_TT[6];
const int32_t REG_CAL[6]={21696,-78,-614558,498,-17021,15755638};
const int32_t REG_CAL5[6]={32146, -1428, -331110, -40, -18930, 18321010};



#define NUM_SSI_DATA  3

int RELOJ, Flag_ints;

int modo =espera; //Control de maquina de estado

//Función del SLEEP fake, utilizar en caso de que dé problemas al usar el debugger
#define SLEEP SysCtlSleep()
//#define SLEEP SysCtlSleepFake()
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
{


    //Habilitar los periféricos implicados en el eje: GPIOF, GPIOJ, GPION
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //Habilitar los periféricos implicados en el eje en el Sleep: GPIOF, GPIOJ, GPION
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPION);

    //Definir tipo de pines, los botones como entradas y los leds como salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4); //F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1); //N0 y N1: salidas
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1

    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

    int i;

    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);       //Habilita T0
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);   //T0 a 120MHz
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);    //T0 periodico y conjunto (32b)
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/20 -1);
    TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer0);
    IntEnable(INT_TIMER0A); //Habilitar las interrupciones globales de los timers
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    // Habilitar las interrupciones de timeout
    IntMasterEnable();  //Habilitacion global de interrupciones
    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer0
    HAL_Init_SPI(1, RELOJ);  //Boosterpack a usar, Velocidad del MC
    Inicia_pantalla();       //Arranque de la pantalla

    // Note: Keep SPI below 11MHz here

    // =======================================================================
    // Delay before we begin to display anything
    // =======================================================================

    SysCtlDelay(RELOJ/3);

    // ================================================================================================================
    // PANTALLA INICIAL
    // ================================================================================================================

    Lee_pantalla();
    Nueva_pantalla(blanco);
    ComColor(rojo);
    ComRect(divHor, divVer, HSIZE-divHor, VSIZE-divVer, true);
    ComLineWidth(10);

    ComColor(blanco);
    ComTXT(HSIZE/2,1.8*divVer, 29, OPT_CENTERX,"BOTONES:");
    ComRect(4*divHor, 5.5*divVer, HSIZE-4*divHor, 3.5*divVer, true);

    //IMPRESIÓN DE BOTONES
    ComFgcolor(verde);
    int j;
    char textBot[]="L0";
    for(j=0;  j<4; j++)
    {
        textBot[1]++;
        Boton((j*5+4)*divHor, VSIZE-4*divVer, 3*divHor, 2*divVer,27, textBot);
    }

    Dibuja();
    Espera_pant();

#ifdef VM800B35
    for(i=0;i<6;i++)	Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL[i]);
#endif
#ifdef VM800B50
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
#endif

    while(1){
        SLEEP;
        char texto[]="              ";
     //   Lee_pantalla();
        Nueva_pantalla(blanco);
        ComColor(rojo);
        ComRect(divHor, divVer, HSIZE-divHor, VSIZE-divVer, true);
        ComLineWidth(10);
        ComColor(blanco);
        ComTXT(HSIZE/2,1.8*divVer, 29, OPT_CENTERX,"BOTONES:");
        ComRect(4*divHor, 5.5*divVer, HSIZE-4*divHor, 3.5*divVer, true);
        ComFgcolor(verde);
       /* for(j=0;  j<4; j++)
         {
             textBot[1]++;
             Boton((j*5+4)*divHor, VSIZE-4*divVer, 3*divHor, 2*divVer,27, textBot);
         }*/
        //MAQUINA DE ESTADO
        if(modo==espera)
        {
            if (B1_ON)  modo=b1;
            if (B2_ON) modo=b2;
            if (dibL1) modo=l1;
            if (dibL2) modo=l2;
            if (dibL3) modo=l3;
            if (dibL4) modo=l4;
            L1_OFF;
            L2_OFF;
            L3_OFF;
            L4_OFF;
        }
        else if (modo==l1)
            {
            dibL2;
            dibL3;
            dibL4;
            L1_ON;
            if(!dibL1) modo=espera;
            }
        else if (modo==l2)
                    {
                    dibL1;
                    dibL3;
                    dibL4;
                    L2_ON;
                    if(!dibL2) modo=espera;
                    }
        else if (modo==l3)
                    {
                    dibL1;
                    dibL2;
                    dibL4;
                    L3_ON;
                    if(!dibL3) modo=espera;
                    }
        else if (modo==l4)
                    {
                    dibL1;
                    dibL2;
                    dibL3;
                    L4_ON;
                    if(!dibL4) modo=espera;
                    }
        else if (modo==b1)
        {
            ComColor(negro);
            ComTXT(HSIZE/2, 4.5*divVer,29, OPT_CENTER, "HAS PULSADO B1");
            ComColor(blanco);
            dibL1;
            dibL2;
            dibL3;
            dibL4;
            if (B1_OFF) modo=espera;

        }
        else if (modo==b2)
                {

            ComColor(negro);
            ComTXT(HSIZE/2, 4.5*divVer,29, OPT_CENTER, "HAS PULSADO B2");
            ComColor(blanco);
            dibL1;
            dibL2;
            dibL3;
            dibL4;
            if (B2_OFF) modo=espera;
                }

        /*if(dibL1) L1_ON;
        else L1_OFF;
        if(dibL2) L2_ON;
        else L2_OFF;
        if(dibL3) L3_ON;
        else L3_OFF;
        if(dibL4) L4_ON;
        else L4_OFF;*/
           Dibuja();


    }

}







