#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "driverlib2.h"
#include "utils/uartstdio.h"
#include "HAL_I2C.h"
#include "sensorlib2.h"
#include "FT800_TIVA.h"

//Definición de colores
#define blanco 240,240,240
#define gris 60,60,60
#define rojo 255,0,0
#define azul 31,75,171
#define fondo 111, 186, 209
#define negro 0,0,0
#define amarillo 199,204,51
#define amarillo_oscuro 144,148,13

//Definición de los valores máximos y mínimos para medidas de sensores
#define Gauge_max 1030
#define Gauge_min 1000
#define Temp_max 30
#define Temp_min 20
#define Hum_min 0
#define Hum_med 40
#define Hum_max 100
#define Luz_min 0
#define Luz_max 100

// Definiciones de la pantalla
#define dword long
#define byte char

#define PosMin 750
#define PosMax 1000

#define XpMax 286
#define XpMin 224
#define YpMax 186
#define YpMin 54

unsigned int Yp=120, Xp=245;

// Variables usadas en la pantalla
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

//#################################################################
//Definición de las funciones de los WIDGETS
//#################################################################

//GAUGE
void ComGauge( int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t major, uint16_t minor,  uint16_t val, int16_t min, uint16_t max)
{
    ComColor(blanco);                     //Color aguja y major y minor
    ComBgcolor(gris);                    //Color de la esfera del reloj
    //Sacar el gauge predeterminado
    EscribeRam32(CMD_GAUGE);
    EscribeRam16(x);
    EscribeRam16(y);
    EscribeRam16(r);
    EscribeRam16(options);
    EscribeRam16(major);
    EscribeRam16(minor);
    EscribeRam16(val-min);
    EscribeRam16(max-min);                  //corresponde a range del CMD_GAUGE

    //Texto
    ComColor(negro);
    char c[5];
    sprintf(c, "%d", max);
    ComTXT(x+r*3/4, y+r*6/5, 26, OPT_CENTER, c);
    sprintf(c, "%d", min);
    ComTXT(x-r*3/4, y+r*6/5, 26, OPT_CENTER, c);
    sprintf(c, "mbar");
    ComTXT(x, y+r*3/5, 26, OPT_CENTER, c);
}

//PROGRESS BAR
//referencia->  pag 191 "FT800 Series ProgrammerGuide"

void ComProgress(int16_t x, int16_t y, int16_t w, int16_t h, int16_t options, int16_t val, int16_t min, int16_t mitad, int16_t max)
{
    char c[8];
    ComColor(negro);
    sprintf(c,"HUMEDAD");
    ComTXT(x+w/2, y+2*h, 26, OPT_CENTER, c);
    int range, value;
    //División del rango total en 2
    if (val>=mitad) {
        //Reasignación del rango y el valor dentro del subrango
        range=max-mitad;
        value=val-mitad;
        //Impresión de valor mínimo y maximo del subrango
        sprintf(c,"%d", mitad);
        ComTXT(x, y+2*h, 26, OPT_CENTER, c);
        sprintf(c,"%d", max);
        ComTXT(x+w, y+2*h, 26, OPT_CENTER, c);
        }
    else {
        //Reasignación del rango y el valor dentro del subrango
        range=mitad-min;
        value=val;
        //Impresión de valor mínimo y maximo del subrango
        sprintf(c,"%d", min);
        ComTXT(x, y+2*h, 26, OPT_CENTER, c);
        sprintf(c,"%d", mitad);
        ComTXT(x+w, y+2*h, 26, OPT_CENTER, c);
        }
    ComBgcolor(azul);
    ComColor(blanco);
    EscribeRam32(CMD_PROGRESS);
    EscribeRam16(x);
    EscribeRam16(y);
    EscribeRam16(w);
    EscribeRam16(h);
    EscribeRam16(options);
    EscribeRam16(value);
    EscribeRam16(range);
    EscribeRam16(0);

}

//TERMÓMETRO
void ComTermometro(int x, int y, int H, int R,float val, int min, int max)
{
    int r = 0.6*R;                  //Radio menor
    int r_int=0.4*R;                //Tamaño principal interior
    int hval=H*(val-min)/(max-min); //altura de la temperatura

    //Fondo
    ComColor(149,254,232);          //Azulito para el fondo
    ComLineWidth(0);
    ComCirculo(x,y,R);
    ComRect(x-r, y-H, x+r, y, 1);
    ComCirculo(x, y-H-80/16, r+80/16);

    //Saturación de temperatura y selección de color
    if (hval<0) {
        hval=0;
        ComColor(0,0,255);          //Color azul para temperatura por debajo de la minima
        }
    else if (hval>H){
        hval=H;
        ComColor(255,0,0);          //Color rojo para temperatura por encima de la máxima
        }
    else ComColor(255,165,0);       //Color naranja por defecto

    //"Mercurio"
    ComCirculo(x,y,R*0.8);
    ComRect(x-r_int, y-hval, x+r_int, y, 1);
    ComCirculo(x, y-hval, r_int+80/16);

    //Texto
    char c[8];
    sprintf(c, "%.1f C", val);
    ComColor(negro);
    ComTXT(x+2*R, y-H/2, 27, OPT_CENTER,c);
}

//SLIDER
//Solo lo dibuja
void ComSlider( int16_t x, int16_t y, int16_t w, uint16_t h, uint16_t options, uint16_t val , uint16_t range)
{

        ComBgcolor(gris);       //Color fondo
        ComFgcolor(amarillo_oscuro);       //Color bola
        ComColor(amarillo);       //Color barra
        EscribeRam32(CMD_SLIDER);
        EscribeRam16(x);
        EscribeRam16(y);
        EscribeRam16(w);
        EscribeRam16(h);
        EscribeRam16(options);
        EscribeRam16(val);
        EscribeRam16(range);
        EscribeRam16(0);
        ComColor(negro);
        char c[10];
        sprintf(c, "LUZ=%d", val);
        ComTXT(x+w/2, y+2*h, 26, OPT_CENTER, c);
}
//Lee y dibuja el Slider
//Función iterativa que va actualizando el valor al pulsar el slider, y si no lo mantiene
int slider(int16_t x, int16_t y, int16_t w, uint16_t h, uint16_t options, int16_t value, uint16_t min, uint16_t max)
{
    Lee_pantalla();
    if (POSX>x && POSX<(x+w) && POSY>y && POSY<(y+h))       //Detectar si alguna parte del slider está siendo pulsada
        value=(POSX-x)*(max-min)/w;                         //Calcular el valor de la posición pulsada
    ComSlider(x,y,w,h,options, value-min, max-min);         //Dibujar Slider
    return value;                                           //Devolver el valor válido
}


#define NUM_SSI_DATA        3

int RELOJ, Flag_ints;


//Función del SLEEP fake, utilizar en caso de que dé problemas al usar el debugger
//#define SLEEP SysCtlSleep()
#define SLEEP SysCtlSleepFake()

void SysCtlSleepFake(void)
{
 while(!Flag_ints);
 Flag_ints=0;
}

void Timer0IntHandler(void);

//Matrices utilizadas para enceder los led en funcion de [luz medida][luz deseada]
bool l1[5][5]={{0,1,1,1,1},
               {0,0,1,1,1},
               {0,0,0,1,1},
               {0,0,0,0,1},
               {0,0,0,0,0}};
bool l2[5][5]={{0,0,1,1,1},
               {0,0,0,1,1},
               {0,0,0,0,1},
               {0,0,0,0,0},
               {0,0,0,0,0}};
bool l3[5][5]={{0,0,0,1,1},
               {0,0,0,0,1},
               {0,0,0,0,0},
               {0,0,0,0,0},
               {0,0,0,0,0}};
bool l4[5][5]={{0,0,0,0,1},
               {0,0,0,0,0},
               {0,0,0,0,0},
               {0,0,0,0,0},
               {0,0,0,0,0}};

//Variables para comprobar cuanta luz hay y cuanta queremos
int estadoLuz=4, estadoSlider=4;


char Cambia=0;
float lux;
char string[80];
int DevID=0;

int16_t T_amb, T_obj;

 float Tf_obj, Tf_amb;
 int lux_i, T_amb_i, T_obj_i;
 int luzDes=Luz_min;                        //Luz deseada, medida a traves del Slider. Empieza como Luz_min


 // BME280
 int returnRslt;
 int g_s32ActualTemp   = 0;
 unsigned int g_u32ActualPress  = 0;
 unsigned int g_u32ActualHumity = 0;

 unsigned int actValores=0;

 float T_act,P_act,H_act;
 bool BME_on = true;

 int T_uncomp,T_comp;
 char mode;
 long int inicio, tiempo;

volatile long int ticks=0;
uint8_t Sensor_OK=0;
#define BP 2
uint8_t Opt_OK, Tmp_OK, Bme_OK, Bmi_OK;

void IntTick(void){
    ticks++;
}
int main(void) {

    //Habilitar los leds,
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //Habilitar leds en el Sleep
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPION);

    //Definir los pines de los leds como salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4); //F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1); //N0 y N1: salidas

    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);


    //Configuración del Timer 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/20 -1);
    TimerIntRegister(TIMER0_BASE, TIMER_A,Timer0IntHandler);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);

    HAL_Init_SPI(1, RELOJ);  //Boosterpack a usar, Velocidad del MC
    Inicia_pantalla();       //Arranque de la pantalla

    if(Detecta_BP(1))       Conf_Boosterpack(1, RELOJ);
    else if(Detecta_BP(2))  Conf_Boosterpack(2, RELOJ);
    else return 0;

    SysCtlDelay(RELOJ/3);

 //Calibración predeterminada de la pantalla
        int i;
     #ifdef VM800B35
         for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL[i]);
     #endif
     #ifdef VM800B50
         for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
     #endif


     Sensor_OK=Test_I2C_Dir(OPT3001_SLAVE_ADDRESS);
     if(!Sensor_OK)   Opt_OK=0;
     else
     {
         OPT3001_init();
         DevID=OPT3001_readDeviceId();
         Opt_OK=1;
     }

     Sensor_OK=Test_I2C_Dir(BME280_I2C_ADDRESS2);
     if(!Sensor_OK)    Bme_OK=0;
     else
     {
         bme280_data_readout_template();
         bme280_set_power_mode(BME280_NORMAL_MODE);
         readI2C(BME280_I2C_ADDRESS2,BME280_CHIP_ID_REG, &DevID, 1);
         Bme_OK=1;
     }
    while(1)
    {
        SLEEP;
        if(ticks>=10){
            ticks=0;
            if(Opt_OK)
            {
                lux=OPT3001_getLux();
                lux_i=(int)round(lux);
            }

            if(Bme_OK)
            {
                returnRslt = bme280_read_pressure_temperature_humidity(
                        &g_u32ActualPress, &g_s32ActualTemp, &g_u32ActualHumity);
                T_act=(float)g_s32ActualTemp/100.0;
                P_act=(float)g_u32ActualPress/100.0;
                H_act=(float)g_u32ActualHumity/1000.0;
            }}

        //Comprobación cantidad de luz/estado luz
        if (lux<100)        estadoLuz=0;
        else if(lux<1000)   estadoLuz=1;
        else if(lux<10000)  estadoLuz=2;
        else if (lux<40000) estadoLuz=3;
        else                estadoLuz=4;

        //Comprobación luz deseada/estado Slider
        if (luzDes<20)      estadoSlider=0;
        else if (luzDes<40) estadoSlider=1;
        else if (luzDes<60) estadoSlider=2;
        else if (luzDes<80) estadoSlider=3;
        else                estadoSlider=4;

        //Escritura de los leds mediante las matrices l1,l2,l3,l4 y el estado de la luz y del Slider
        GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,GPIO_PIN_1*l1[estadoLuz][estadoSlider]);
        GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,GPIO_PIN_0*l2[estadoLuz][estadoSlider]);
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_PIN_4*l3[estadoLuz][estadoSlider]);
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_0*l4[estadoLuz][estadoSlider]);

        //Dibujo de la pantalla
        Lee_pantalla();
        Nueva_pantalla(fondo);
        //Dibujo de los widgets
        ComTermometro(HSIZE*2/10, VSIZE*0.75, VSIZE*0.5, HSIZE*0.06,T_act,Temp_min,Temp_max);
        ComGauge(HSIZE*7/10, VSIZE/4, VSIZE*0.8/4, 0, (Gauge_max-Gauge_min)/10, 2,  P_act, Gauge_min, Gauge_max);
        luzDes = slider(HSIZE/2,VSIZE*0.75, HSIZE*4/10, VSIZE*0.1, 0, luzDes, Luz_min, Luz_max);
        ComProgress(HSIZE/2, VSIZE*0.55, HSIZE*4/10, VSIZE*0.05, 0, H_act, Hum_min, Hum_med, Hum_max);

        Dibuja();
 }

}
void Timer0IntHandler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Borra flag
    Flag_ints=1;
	ticks++;
	//SysCtlDelay(100);
}



