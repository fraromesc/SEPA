
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "driverlib2.h"

#include "FT800_TIVA.h"

//Definición para los estados de comprobación de la pulsación de los botones
#define B2_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)
#define B2_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0))
#define B1_OFF GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)
#define B1_ON !(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1))

//Definición de los estados encendido y apagado de los LEDs
#define L3_ON GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,GPIO_PIN_1)
#define L3_OFF GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,0)
#define L2_ON GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,GPIO_PIN_0)
#define L2_OFF GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,0)

#define L1_ON GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_PIN_4)
#define L1_OFF GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,0)
#define L0_ON GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_0)
#define L0_OFF GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0)

//Definimos cada uno de los colores en que vamos a utilizar, en formato RGB
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
//Hacemos una división del ancho y del alto de la pantalla, de forma que multiplicando
//dichas divisiones por constantes, podamos establecer coordenadas dentro de la "cuadrícula" de la pantalla
#define divHor HSIZE/20
#define divVer VSIZE/17

//Definición de cada una de las instancias por las que pasará la máquina de estados
#define inicio 0
#define contadorDinero 1
#define expulsionProducto1 2
#define expulsionProducto2 7
#define calculoVuelta 3
#define devolucionVuelta 4
#define creditoInsuficiente 5
#define monedaIntroducida 6
//Tenemos 4 productos, un número de producto mayor a 3 (de 0 a 3), no es considerado producto
#define NotAProduct 5
//Definición de los botones de los productos con sus respectivas coordenadas para que queden
//ajustados y equiespaciados a la izquierda de la pantalla
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

int RELOJ, Flag_ints, PeriodoPWM;
//Control de maquina de estados
int modo =inicio;
//Vector con los precios de los productos
int precios[]={30, 35, 40, 45};
//Cadena con los nombres de los productos
char productos[4][10]={"Chicle", "Gominolas", "Pipas", "Phoskitos"};
//Cadena para mostrar por la ventana de credito
char credito[]="CREDITO";
//Contador de crédito
int contCredito=0;
//Variable para saber el producto seleccionado
int prod=NotAProduct;
//Cadena con el mensaje para el recuadro de la pantalla del Vending
char mensaje[2][35]={"", "TOTAL: 0c"};
//Contador monedas de 20c
int m20=0;
//Contador monedas de 5c
int m5=0;
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

//Función giro de la segunda práctica para convertir el ángulo de giro a un %
void giro (int pos)
{//Valores máximo y mínimo que llegarán PWM, y serán utilizados en la función giro
    int Max_Pos = 4700;//4200; //3750
    int Min_Pos = 1000; //1875
    int posicion=Min_Pos+((Max_Pos-Min_Pos)*pos)/100;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, posicion);   //Inicialmente, 1ms
}

int main(void)

{
    //Habilitar los periféricos implicados en el ejercio: leds, botones, la salida del servomotor y el PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    //Habilitar los periféricos implicados en el Sleep
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
    //Matriz con cada uno de los leds encendidos, tal y como en la primera práctica
    int LED[4][4]={0,0,0,1,
                   0,0,1,0,
                   0,1,0,0,
                   1,0,0,0};
    //Comenzamos con los leds apagados y el servo al 50% (hacia abajo)
    L0_OFF;
    L1_OFF;
    L2_OFF;
    L3_OFF;
    giro(50);


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

    //Calibración predeterminada de la pantalla
    int i;
 #ifdef VM800B35
     for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL[i]);
 #endif
 #ifdef VM800B50
     for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
 #endif

    while(1){
        //Ciclo de ejecución cada 50 ms
        SLEEP;
        //Máquina de estados
        switch(modo){
        case inicio://En el estado inicial:
            sprintf(credito, "CREDITO");
            if (B1_ON)//Si pulsamos B1, introducimos una moneda de 20c, aumentamos su contador y preparamos los mensajes correspondientes
            {
                modo=monedaIntroducida;
                m20++;
                sprintf(mensaje[0], "MONEDA INTRODUCIDA: 20c");
                contador=0;
                contCredito+=20;
                ComColor(verdeClaro);
            }else if (B2_ON) //Si pulsamos B2, introducimos una moneda de 5c, aumentamos su contador y preparamos los mensajes correspondientes
            {
                m5++;
                modo=monedaIntroducida;
                sprintf(mensaje[0],"MONEDA INTRODUCIDA:5c");
                contCredito+=5;
                contador=0;
                ComColor(verdeClaro);
            }
            break;
        case monedaIntroducida: //Esperamos 1 segundo y preparamos los mensajes relacionados con la cantidad de monedas de 20c
            if (contador>=20) {
                modo=contadorDinero;
            }

            sprintf(credito, "%d", contCredito);
            sprintf(mensaje[1], "TOTAL: %d c  ",contCredito);
            break;
        case contadorDinero: //Indicamos en el recuadro que se ha introducido una moneda de 20c al pulsar B1
            if (B1_ON) {modo=monedaIntroducida;
                        contador=0;
                        sprintf(mensaje[0], "MONEDA INTRODUCIDA: 20c");
                        m20++;}
                            //Indicamos en el recuadro que se ha introducido una moneda de 20c al pulsar B1
            else if (B2_ON)  {modo=monedaIntroducida;
                              contador=0;
                              sprintf(mensaje[0],"MONEDA INTRODUCIDA:5c");
                              m5++;}
            else if (botDev) {
                //Si queremos devolver el crédito introducido, devolvemos el mayor número de monedas de 20c posibles, e indicamos
                //cuantas monedas de 20c y de 5c hemos devuelto en total
                modo = devolucionVuelta;
                m20=contCredito/20;
                m5=(contCredito-20*m20)/5;
                contador=0;
                sprintf(mensaje[0], "DEVOLUCION DE %d c",contCredito);
                sprintf(mensaje[1], "%d x 20c, %d x 5c", m20,m5);
                sprintf(credito, "%d", 0);
            }//Posibles casos de producto, del 0 al 3, 4 en total
            else if (botProd0)  prod=0;
            else if (botProd1)  prod=1;
            else if (botProd2)  prod=2;
            else if (botProd3)  prod=3;

            if (prod<4)
                //Si elegimos uno de los 4 productos disponibles y tenemos crédito disponible, encendemos su led correspondiente,
                //y giramos el servomotor
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
                    ////Si elegimos uno de los 4 productos disponibles y no tenemos crédito disponible, se indica en la pantalla
                    modo=creditoInsuficiente;
                    sprintf(mensaje[0],"CREDITO INSUFICIENTE");
                }
                contador=0;}
            //El crédito introducido es el número de monedas de 20c * 20 + el número de monedas de 5c * 5
            contCredito=20*m20+5*m5;
            break;
        case creditoInsuficiente:
            //Si no tenemos crédito suficiente, reseteamos el contador y esperamos a que se introduzca más o se pida devolución
            if (contador>=20)   {
                contador=0;
                modo=contadorDinero;
                sprintf(mensaje[0],"");
                prod=NotAProduct;
            }
            break;
        case expulsionProducto1:
            //Caso de elegir un producto, sin devolución de crédito
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
            //Caso de elegir un producto, con devolución de crédito (con el mínimo número de monedas posible)
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
            //Tras devolver el crédito correspondiente, se resetea el número de monedas y se vuelve al estado inicial
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
                sprintf(credito,"CREDITO");
                prod=NotAProduct;
            }
            break;
            }

        //Dibujar la pantalla
        Lee_pantalla();
        Nueva_pantalla(azulClaro);
        //Fondo botones productos
        ComColor(amarilloClaro);
        ComRect(0.5*divHor, 2*divVer, 7*divHor, VSIZE-2*divVer, true);
        //Pantalla para mensajes
        ComColor(blanco);
        ComRect(8*divHor, 2*divVer, HSIZE-divHor, 5*divVer, true);
        //Pantalla con cantidad de crédito
        ComRect(HSIZE-5*divHor, VSIZE-8*divVer, HSIZE-divHor, VSIZE-6*divVer, true);
        //Fondo de la caja de las monedas
        ComColor(verdeClaro);
        ComRect(8*divHor, 8*divVer, 12*divHor, VSIZE-2*divVer, true);
        //BOTONES
        ComColor(verdeClaro);//Pintamos el texto de cada uno de los productos, y el de devolución
        botProd0;
        botProd1;
        botProd2;
        botProd3;
        botDev;
        //Pintamos el circulo que representa a la moneda de 20c
        ComColor(amarilloClaro);
        ComCirculo(10.5*divHor, 10*divVer, divHor);
        //Pintamos el circulo que representa a la moneda de 5c
        ComColor(naranja);
        ComCirculo(10.5*divHor, 13*divVer, 0.8*divHor);
        //Pintamos "20c" y "5c" dentro de cada moneda
        ComColor(negro);
        ComTXT(10.5*divHor, 10*divVer,27,OPT_CENTER, "20c");
        ComTXT(10.5*divHor, 13*divVer,27,OPT_CENTER, "5c");
        //Cantidad de monedas de 20c, representada junto al dibujo de la moneda
        ComColor(negro);
        sprintf(monedas, "%d x", m20);
        ComTXT(8.5*divHor, 10*divVer, 27, OPT_CENTER, monedas);
        //Cantidad de monedas de 5c, representada junto al dibujo de la moneda
        sprintf(monedas, "%d x", m5);
        ComTXT(8.5*divHor, 13*divVer, 27, OPT_CENTER, monedas);
        //Sacar por pantalla lo anterior
        ComTXT(HSIZE-3*divHor, VSIZE-7*divVer,22, OPT_CENTER, credito);
        ComTXT(HSIZE-6.5*divHor, 3*divVer,22, OPT_CENTER, mensaje[0]);
        ComTXT(HSIZE-6.5*divHor, 4*divVer,22, OPT_CENTER, mensaje[1]);
        //Vector de los precios de los productos
        int k;
        for (k=0; k<4; k++)
            {
                sprintf(carPrecios, "%dc", precios[k]);
                ComTXT(6*divHor, (3*k+4)*divVer,22, OPT_CENTER, carPrecios);
            }
        ComColor(negro);
        //Marcos negros para cada uno de los recuadros de la pantalla
        ComRect(8*divHor-4, 2*divVer-4, HSIZE-divHor+4, 5*divVer+4, false);
        ComRect(8*divHor-4, 8*divVer-4, 12*divHor+4, VSIZE-2*divVer+4, false);
        ComRect(HSIZE-5*divHor-4, VSIZE-8*divVer-4, HSIZE-divHor+4, VSIZE-6*divVer+4, false);
        ComRect(0.5*divHor-4, 2*divVer-4, 7*divHor+4, VSIZE-2*divVer+4, false);
        Dibuja();
    }

}





