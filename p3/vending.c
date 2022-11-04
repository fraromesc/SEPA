/*
 * main.c
 */


#include <stdint.h>
#include <stdbool.h>
#include "driverlib2.h"

#include "FT800_TIVA.h"

//DEFINICIÓN COMPROBACION BOTONES
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))

//DEFINICION DE ESTADOS ENCENDIDO Y APAGADO DE LOS LEDS
#define L3_ON GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,GPIO_PIN_1)
#define L3_OFF GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,0)
#define L2_ON GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,GPIO_PIN_0)
#define L2_OFF GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,0)

#define L1_ON GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_PIN_4)
#define L1_OFF GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,0)
#define L0_ON GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_0)
#define L0_OFF GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0)

//DEFINICION DE COLORES
#define rojo 163,47,41
#define amarillo 255,255,153
#define gris 128,128,128
#define verde 128,255,0
#define verde2 153,255,51
#define verdeClaro 191,255,179
#define negro 0,0,0
#define blanco 255,255,255
#define azulClaro 201,255,229
#define amarilloClaro 255,255,204
#define rojo2 255,0,0
#define naranja 230,115,0
#define divHor HSIZE/20
#define divVer VSIZE/17

//DEFINICIÓN ESTADOS DE LA MAQUINA DE ESTADOS
#define inicio 0
#define contadorDinero 1
#define expulsionProducto1 2
#define expulsionProducto2 7
#define calculoVuelta 3
#define devolucionVuelta 4
#define creditoInsuficiente 5
#define monedaIntroducida 6

#define NotAProduct 5
//BOTONES PANTALLA
#define botProd0 Boton(divHor, 3*divVer, 4*divHor, 2*divVer,27, productos[0])
#define botProd1 Boton(divHor, 6*divVer, 4*divHor, 2*divVer,27, productos[1])
#define botProd2 Boton(divHor, 9*divVer, 4*divHor, 2*divVer,27, productos[2])
#define botProd3 Boton(divHor, 12*divVer, 4*divHor, 2*divVer,27, productos[3])
#define botDev  Boton(HSIZE-6*divHor, VSIZE-4*divVer, 5*divHor, 2*divVer, 27, "Devolver")
//#define bot20 ComColor(amarilloClaro);ComCirculo(10.5*divHor, 10*divVer, divHor);    ComColor(negro); ComTXT(10.5*divHor, 10*divVer,27,OPT_CENTER, "20c")
//#define bot5 ComColor(naranja);ComCirculo(10.5*divHor, 13*divVer, 0.8*divHor);ComColor(negro); ComTXT(10.5*divHor, 13*divVer,27,OPT_CENTER, "5c")
#define bot20 ComCirculo(10.5*divHor, 10*divVer, divHor)
#define bot5 ComCirculo(10.5*divHor, 13*divVer, 0.8*divHor)
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



#define NUM_SSI_DATA        3

int RELOJ, Flag_ints,PeriodoPWM;

int modo =inicio;                                                   //Control de maquina de estado
int precios[]={30, 35, 40, 45};                                     //Vector con los precios de los productos
char productos[4][10]={"kit bdsm", "chicle", "pipas", "phoskitos"};   //Cadena con los nombres de los productos
char credito[]="CREDITO";                                           //Cadena para mostrar por la ventana de credito
int contCredito=0;                                                  //Contador de crédito
int prod=NotAProduct;                                                        //Variable para saber el producto seleccionado
char mensaje[2][35]={"", "TOTAL: 0c"};                              //Cadena con mensaje para la pantalla del Vending
int m20=0;                                                          //Contador monedas de 20c
int m5=0;                                                           //Contador monedas de 5c
char monedas[3];
char carPrecios[3];
//Función del SLEEP fake, utilizar en caso de que dé problemas al usar el debugger
//#define SLEEP SysCtlSleep()
#define SLEEP SysCtlSleepFake()
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

void int2char(int number, char *c){

    int i, digit=10;

    for(i=0; number>0; digit*=10, i++)
    {
        c[i]='0'+(number%digit)/(digit/10);
        number-=((c[i]-'0')*(digit/10));
    }

    c[i]='\0';
    char aux[i-1];
    int j;
    for (j=0;j<i; j++){
        //printf("hola");
        aux[j]=c[j];
    }
    for (j=0; j<i;j++)
    {
        c[j]=aux[i-1-j];
    }

}

void giro (int pos)
{//Valores máximo y mínimo que llegarán PWM, y serán utilizados en la función giro
    int Max_Pos = 4700;//4200; //3750
    int Min_Pos = 1000; //1875
    int posicion=Min_Pos+((Max_Pos-Min_Pos)*pos)/100;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, posicion);   //Inicialmente, 1ms
}

int main(void)

{

    //Habilitar los periféricos implicados en el eje: GPIOF, GPIOJ, GPION
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    //Habilitar los periféricos implicados en el eje en el Sleep: GPIOF, GPIOJ, GPION
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_PWM0);

    //Definir tipo de pines, los botones como entradas y los leds como salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4); //F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1); //N0 y N1: salidas
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);   //J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //Pullup en J0 y J1

    PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_64);   // al PWM le llega un reloj de 1.875MHz

        GPIOPinConfigure(GPIO_PG0_M0PWM4);          //Configurar el pin a PWM
        GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0);

        //Configurar el PWM4, contador descendente y sin sincronización (actualización automática)
        PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

        PeriodoPWM=37499; // 50Hz  a 1.875MHz
        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PeriodoPWM); //frec:50Hz
        giro(50); //Posicion inicial del servo
        PWMGenEnable(PWM0_BASE, PWM_GEN_2);     //Habilita el generador 0
        PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT , true);    //Habilita la salida 1

    int LED[4][4]={0,0,0,1,
                   0,0,1,0,
                   0,1,0,0,
                   1,0,0,0};
    L0_OFF;
    L1_OFF;
    L2_OFF;
    L3_OFF;
    giro(50);

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

    //Lee_pantalla();
    //RECTANGULOS
    Nueva_pantalla(azulClaro);
    ComColor(amarilloClaro);
    ComRect(0.5*divHor, 2*divVer, 7*divHor, VSIZE-2*divVer, true);                  //Fondo botones productos
    ComColor(blanco);
    ComRect(8*divHor, 2*divVer, HSIZE-divHor, 5*divVer, true);                      //Pantalla para mensajes
    ComRect(HSIZE-5*divHor, VSIZE-8*divVer, HSIZE-divHor, VSIZE-6*divVer, true);    //Pantalla con cantidad de crédito
    ComColor(verdeClaro);
    ComRect(8*divHor, 8*divVer, 12*divHor, VSIZE-2*divVer, true);                   //Fondo de caja de monedas
    //BOTONES
    ComColor(verdeClaro);
    botProd0;
    botProd1;
    botProd2;
    botProd3;
    botDev;
    ComColor(amarilloClaro);
    ComCirculo(10.5*divHor, 10*divVer, divHor);
    ComColor(230,115,0);
    ComCirculo(10.5*divHor, 13*divVer, 0.8*divHor);
    ComColor(negro);
    ComTXT(10.5*divHor, 10*divVer,27,OPT_CENTER, "20c");
    ComTXT(10.5*divHor, 13*divVer,27,OPT_CENTER, "5c");
    //TEXTO
    ComColor(negro);
    sprintf(monedas, "%d x", m20);
    ComTXT(8.5*divHor, 10*divVer, 27, OPT_CENTER, monedas);
    sprintf(monedas, "%d x", m20);
    ComTXT(8.5*divHor, 13*divVer, 27, OPT_CENTER, monedas);
    ComTXT(HSIZE-3*divHor, VSIZE-7*divVer,22, OPT_CENTER, credito);
    ComTXT(HSIZE-6.5*divHor, 3*divVer,22, OPT_CENTER, mensaje[0]);
    ComTXT(HSIZE-6.5*divHor, 4*divVer,22, OPT_CENTER, mensaje[1]);
    int k;
    for (k=0; k<4; k++)
    {
        sprintf(carPrecios, "%dc", precios[k]);
        ComTXT(6*divHor, (3*k+4)*divVer,22, OPT_CENTER, carPrecios);
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

        //MAQUINA DE ESTADOS
        switch(modo){
        case inicio:
            sprintf(credito, "CREDITO");
            if (B1_ON)
            {
                modo=monedaIntroducida;
                m20++;
                sprintf(mensaje[0], "MONEDA INTRODUCIDA: 20c");
                contador=0;
                contCredito+=20;
            }else if (B2_ON)
            {
                m5++;
                modo=monedaIntroducida;
                sprintf(mensaje[0],"MONEDA INTRODUCIDA:5c");
                contCredito+=5;
                contador=0;
            }
            break;
        case monedaIntroducida:
            if (contador>=20) {
                modo=contadorDinero;
                sprintf(mensaje[1],"");
            }

            sprintf(credito, "%d", contCredito);
            sprintf(mensaje[1], "TOTAL: %d c  ",contCredito);
            break;
        case contadorDinero:
            if (B1_ON) {modo=monedaIntroducida;
                        contador=0;
                        sprintf(mensaje[0], "MONEDA INTRODUCIDA: 20c");
                        m20++;}
            else if (B2_ON)  {modo=monedaIntroducida;
                              contador=0;
                              sprintf(mensaje[0],"MONEDA INTRODUCIDA:5c");
                              m5++;}
            else if (botDev) modo = devolucionVuelta;
            else if (botProd0)  prod=0;
            else if (botProd1)  prod=1;
            else if (botProd2)  prod=2;
            else if (botProd3)  prod=3;

            if (prod<4)
            {   if(contCredito>=precios[prod]){
                    sprintf(mensaje[0],"DISPENSANDO PRODUCTO");
                    sprintf(mensaje[1],"");
                    modo=expulsionProducto1;
                    giro(0);
                    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1*LED[prod][0]);
                    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[prod][1]);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4*LED[prod][2]);
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0*LED[prod][3]);
                    }
                else if (contCredito<precios[prod]){
                    modo=creditoInsuficiente;
                    sprintf(mensaje[0],"CREDITO INSUFICIENTE");
                }
                contador=0;}
            contCredito=20*m20+5*m5;
            break;
        case creditoInsuficiente:
            if (contador>=20)   {
                contador=0;
                modo=contadorDinero;
                sprintf(mensaje[0],"");
                prod=NotAProduct;
            }
            break;
        case expulsionProducto1:
            if (contador >=40)
            {   modo=expulsionProducto2;
                sprintf(mensaje[0],"PRODUCTO DISPENSADO");
                contCredito-=precios[prod];
                giro(50);
                L0_OFF;
                L1_OFF;
                L2_OFF;
                L3_OFF;
                contador=0;
            }
            break;
        case expulsionProducto2:
            if (contador>=40){
                modo=calculoVuelta;
                m20=contCredito/20;
                m5=(contCredito-20*m20)/5;
                contador=0;
                sprintf(mensaje[0], "DEVOLUCION DE %d c",contCredito);
                sprintf(mensaje[1], "%d x 20c, %d x 5c", m20,m5);
                sprintf(credito, "%d", 0);
                contador=0;
            }
            break;
        case calculoVuelta:
            if(contador >=20)
            {modo = devolucionVuelta;}
            break;
        case devolucionVuelta:
            if (contador>=100) {
                modo =inicio;
                sprintf(mensaje[0],"");
                sprintf(mensaje[1],"TOTAL: 0c");
                m20=0;
                m5=0;
                contCredito=0;
            }
            break;
            }

        //DIBUJO PANTALLA
        Lee_pantalla();
        //RECTANGULOS
        Nueva_pantalla(azulClaro);
        ComColor(amarilloClaro);
        ComRect(0.5*divHor, 2*divVer, 7*divHor, VSIZE-2*divVer, true);                  //Fondo botones productos
        ComColor(blanco);
        ComRect(8*divHor, 2*divVer, HSIZE-divHor, 5*divVer, true);                      //Pantalla para mensajes
        ComRect(HSIZE-5*divHor, VSIZE-8*divVer, HSIZE-divHor, VSIZE-6*divVer, true);    //Pantalla con cantidad de crédito
        ComColor(verdeClaro);
        ComRect(8*divHor, 8*divVer, 12*divHor, VSIZE-2*divVer, true);                   //Fondo de caja de monedas
        //BOTONES
        ComColor(verdeClaro);
        botProd0;
        botProd1;
        botProd2;
        botProd3;
        botDev;
        ComColor(amarilloClaro);
        ComCirculo(10.5*divHor, 10*divVer, divHor);
        ComColor(230,115,0);
        ComCirculo(10.5*divHor, 13*divVer, 0.8*divHor);
        ComColor(negro);
        ComTXT(10.5*divHor, 10*divVer,27,OPT_CENTER, "20c");
        ComTXT(10.5*divHor, 13*divVer,27,OPT_CENTER, "5c");
        //TEXTO
        ComColor(negro);
        sprintf(monedas, "%d x", m20);
        ComTXT(8.5*divHor, 10*divVer, 27, OPT_CENTER, monedas);
        sprintf(monedas, "%d x", m5);
        ComTXT(8.5*divHor, 13*divVer, 27, OPT_CENTER, monedas);
        ComTXT(HSIZE-3*divHor, VSIZE-7*divVer,22, OPT_CENTER, credito);
        ComTXT(HSIZE-6.5*divHor, 3*divVer,22, OPT_CENTER, mensaje[0]);
        ComTXT(HSIZE-6.5*divHor, 4*divVer,22, OPT_CENTER, mensaje[1]);
        for (k=0; k<4; k++)
            {
                sprintf(carPrecios, "%dc", precios[k]);
                ComTXT(6*divHor, (3*k+4)*divVer,22, OPT_CENTER, carPrecios);
            }
        Dibuja();
    }

}

/*TAREAS:
 * -> dibujar en el while
 * -> corregir los textos
 */





