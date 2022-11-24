
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "driverlib2.h"
#include "utils/uartstdio.h"
#include "HAL_I2C.h"
#include "sensorlib2.h"
#include "FT800_TIVA.h"

//Definimos cada uno de los colores en que vamos a utilizar, en formato RGB
#define gris 60,60,60
#define rojo 255,0,0
#define azul 0,0,255
#define blanco 255,255,255

//Divisiones del ancho y largo de la pantalla
#define divHor HSIZE/2
#define divVer VSIZE/9
//Definición de cadenas de texto

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

int RELOJ, Flag_ints;

//Función del SLEEP fake, utilizar en caso de que dé problemas al usar el debugger
//#define SLEEP SysCtlSleep()
#define SLEEP SysCtlSleepFake()

void SysCtlSleepFake(void)
{
 while(!Flag_ints);
 Flag_ints=0;
}
//Creamos cadenas para cada una de las variables que vamos a medir
void Timer0IntHandler(void);

char temp[]= "Temperatura = 000.00ºC";
char pres[]= "Presion = 000000 mbar";
char hum[]= "Humedad = 00.00 0/0";
char luz[]= "Luz = 0000 lux";

char Cambia=0;

float lux;
char string[80];
int DevID=0;

int16_t T_amb, T_obj;

 float Tf_obj, Tf_amb;
 int lux_i, T_amb_i, T_obj_i;

 // BME280
 int returnRslt;
 int g_s32ActualTemp   = 0;
 unsigned int g_u32ActualPress  = 0;
 unsigned int g_u32ActualHumity = 0;
// struct bme280_t bme280;

 // BMI160/BMM150
 int8_t returnValue;
 struct bmi160_gyro_t        s_gyroXYZ;
 struct bmi160_accel_t       s_accelXYZ;
 struct bmi160_mag_xyz_s32_t s_magcompXYZ;


 //Calibration off-sets
 int8_t accel_off_x;
 int8_t accel_off_y;
 int8_t accel_off_z;
 int16_t gyro_off_x;
 int16_t gyro_off_y;
 int16_t gyro_off_z;
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

    //Configuración del Timer0
    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/2 -1);
    TimerIntRegister(TIMER0_BASE, TIMER_A,Timer0IntHandler);
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);

    HAL_Init_SPI(1, RELOJ);  //Boosterpack a usar para la pantalla (1), Velocidad del MC
    Inicia_pantalla();       //Arranque de la pantalla

    SysCtlDelay(RELOJ/3);

    if(Detecta_BP(1))     Conf_Boosterpack(1, RELOJ);
    else if(Detecta_BP(2))   Conf_Boosterpack(2, RELOJ);
    else     return 0;

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
    SysTickIntRegister(IntTick);
    SysTickPeriodSet(12000);
    SysTickIntEnable();
    SysTickEnable();

    while(1)
    {
        SLEEP;

            if(Opt_OK)
            {//Luz
                lux=OPT3001_getLux();
                lux_i=(int)round(lux);
            }

            if(Bme_OK)
            {//Cálculo de la temperatura, presión y humedad
                returnRslt = bme280_read_pressure_temperature_humidity(&g_u32ActualPress, &g_s32ActualTemp, &g_u32ActualHumity);
                T_act=(float)g_s32ActualTemp/100.0;
                P_act=(float)g_u32ActualPress/100.0;
                H_act=(float)g_u32ActualHumity/1000.0;
            }

        sprintf(temp,"Temperatura = %.2f C",T_act);//Reescribimos la temperatura, con 2 decimales según la  medida cada ciclo de reloj
        sprintf(pres, "Presion = %.1f mbar",P_act);//Reescribimos la presión, con 1 decimal según la medida
        sprintf(hum,"Humedad =  %.1f %c",H_act,37);//Reescribimos la humedad, con 1 decimal según la  medida
        sprintf(luz, "Luz = %d lux",lux_i); //Reescribimos la intensidad luminosa, sin decimales según la medida
        Lee_pantalla();
        Nueva_pantalla(gris);

        //Decidir el color de T
        if (T_act<20)        ComColor(azul); //Si la temperatura es menor a 20ºC: azul
        else if (T_act>25)   ComColor(rojo); //Si la temperatura es mayor a 30ºC: rojo
        else                 ComColor(blanco); //Entre ambas temperaturas: blanco
        ComTXT(divHor,1.5*divVer, 29, OPT_CENTER, temp); //Texto centrado, 1º fila

        //Decidir el color de P
        if (P_act<1000)        ComColor(azul); //Si la presión es menor a 1000 mbar: azul
        else if (P_act>1015)   ComColor(rojo); //Si la presión es menor a 1015 mbar: rojo
        else                 ComColor(blanco); //Entre ambas presiones: blanco
        ComTXT(divHor,3.5*divVer, 29, OPT_CENTER, pres); //Texto centrado, 2º fila

        //Decidir el color de H
        if (H_act<30)        ComColor(azul); //Si la humedad es menor al 30%: azul
        else if (H_act>60)   ComColor(rojo); //Si la humedad es mayor al 60%: azul
        else                 ComColor(blanco); //Entre ambas, blanco
        ComTXT(divHor,5.5*divVer, 29, OPT_CENTER, hum); //Texto centrado, 3º fila

        //Decidir el color de L
        if (lux_i<100)        ComColor(azul); //Si la luz es menor a 100 lux: azul
        else if (lux_i>1000)   ComColor(rojo); //Si la luz es mayor a 1000 lux: azul
        else                 ComColor(blanco); //Entre ambas, blanco
        ComTXT(divHor,7.5*divVer, 29, OPT_CENTER, luz); //Texto centrado, 4º fila

        Dibuja();//Representar en pantalla lo anterior
 }

}
//Función de interrupción del Timer
void Timer0IntHandler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Borra flag
    Flag_ints=1;
	Cambia=1;
}

