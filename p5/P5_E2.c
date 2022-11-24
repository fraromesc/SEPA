#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "driverlib2.h"
#include "utils/uartstdio.h"
#include "HAL_I2C.h"
#include "sensorlib2.h"

//VARIABLES DE TIMER & PWM
int RELOJ, PeriodoPWM, tiempo=0, Flag_ints;

//CABECERA FUNCION SLEEP
void Timer0IntHandler(void);


//VARIABLES  PARA EL SENSOR
int DevID=0;

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

uint8_t Sensor_OK=0;
uint8_t Bmi_OK;
#define BP 2


//VARIABLES DE INTERFAZ
bool l0=0, l1=0, l2=0, l3=0;            //Estado de los leds. 0=apagado. 1=encendido
int angulo=50;                          //Angulo del servo.
int brujula=0;                          //Dato para la brujula
int oriX=0, oriY=0;                     //Orientación de la placa calculada con el acelerómetro

//VARIABLES UART
char string[80];

//FUNCIÓN PARA EL GIRO DEL SERVO: entrada en porcentaje de giro
void giro (int pos)
{
    //Valores máximo y mínimo que llegarán PWM, y serán utilizados en la función giro
    int Max_Pos = 4700;//4200; //3750
    int Min_Pos = 1000; //1875
    int posicion=Min_Pos+((Max_Pos-Min_Pos)*pos)/100;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, posicion);   //Inicialmente, 1ms
}
//Función del SLEEP fake, utilizar en caso de que dé problemas al usar el debugger
#define SLEEP SysCtlSleep()
//#define SLEEP SysCtlSleepFake()

void SysCtlSleepFake(void)
{
 while(!Flag_ints);
 Flag_ints=0;
 tiempo++;
}
uint32_t g_ui32SysClock;
int main(void) {

    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    //HABILITACIÓN LEDS
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 |GPIO_PIN_4);                                     //F0 y F4: salidas
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 |GPIO_PIN_1);                                     //N0 y N1: salidas

    //CONFIGURACIÓN PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);                                       //J0 y J1: entradas
    GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0|GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);    //Pullup en J0 y J1

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);                                                       //Habilitación Timer0
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);                                               //Timer0 a 120MHz
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);                                                    //Timer0 periodico y conjunto (32b)
    TimerLoadSet(TIMER0_BASE, TIMER_A, RELOJ/20 -1);
    TimerIntRegister(TIMER0_BASE,TIMER_A,Timer0IntHandler);
    IntEnable(INT_TIMER0A);                                                                             //Habilitación las interrupciones globales de los timers
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                                                    //Habilitación las interrupciones de timeout
    IntMasterEnable();                                                                                  //Habilitacion global de interrupciones
    TimerEnable(TIMER0_BASE, TIMER_A);                                                                  //Habilitar Timer0, 1, 2A y 2B

    PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_64);                                                           // PWM llega un reloj de 1.875MHz

    GPIOPinConfigure(GPIO_PG0_M0PWM4);                                                                  //Configuración pin del servo al PWM
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0);

    //CONFIGURACIÓN PWM4: contador descendente y sin sincronización (actualización automática)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    PeriodoPWM=37499;                                                                                    // 50Hz  a 1.875MHz
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PeriodoPWM);                                                   //frec:50Hz
    giro(50);                                                                                            //Posicion inicial del servo
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);                                                                  //Habilitación del generador 0
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT , true);                                                     //Habilitación de la salida 1
    //ACTIVACIÓN DE LOS PERIFÉRICOS EN EL MODO SLEEP
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);

    //CONFIGURACIÓN UART
    //Configuración del reloj para la UART
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    //Configuración para el uso de la UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, g_ui32SysClock);

    //CONFIGURACIÓN BOOSTERPACK
    if(Detecta_BP(1))   Conf_Boosterpack(1, RELOJ);
    else if(Detecta_BP(2))  Conf_Boosterpack(2, RELOJ);
    else return 0;

    //COMPROBACIÓN SENSOR BMI160_I2C_ADDR2
    Sensor_OK=Test_I2C_Dir(BMI160_I2C_ADDR2);
    if(!Sensor_OK)    Bmi_OK=0;
    else
     {
         bmi160_initialize_sensor();
         bmi160_config_running_mode(APPLICATION_NAVIGATION);
         readI2C(BMI160_I2C_ADDR2,BMI160_USER_CHIP_ID_ADDR, &DevID, 1);
         Bmi_OK=1;
     }

    while(1)
    {

        //SLEEP;


        //ENCENDIDO/APAGADO LEDS
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_0*l0);
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_PIN_4*l1);
        GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,GPIO_PIN_0*l2);
        GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_1,GPIO_PIN_1*l3);
        //GIRO DEL SERVO
        giro(angulo);
        //LECTURA DE SENSORES
        if(Bmi_OK){
            bmi160_bmm150_mag_compensate_xyz(&s_magcompXYZ);
            bmi160_read_accel_xyz(&s_accelXYZ);
            bmi160_read_gyro_xyz(&s_gyroXYZ);}
        //CALCULO DE ORIENTACIÓN
        brujula = atan2(s_magcompXYZ.y, s_magcompXYZ.x)*(180.0/3.14);
        if(brujula<0)    brujula=brujula+360;
        oriX = -atan(s_accelXYZ.x/sqrt(pow(s_accelXYZ.y,2) + pow(s_accelXYZ.z,2)))*(180.0/3.14);
        oriY = -atan(s_accelXYZ.y/sqrt(pow(s_accelXYZ.x,2) + pow(s_accelXYZ.z,2)))*(180.0/3.14);

        UARTprintf("------------------------------------------------\n");
        UARTprintf("  BMM:  X:%6d\tY:%6d\tZ:%6d  \n",s_magcompXYZ.x,s_magcompXYZ.y,s_magcompXYZ.z);
        UARTprintf("  ACCL: X:%6d\tY:%6d\tZ:%6d  \n",s_accelXYZ.x,s_accelXYZ.y,s_accelXYZ.z);
        UARTprintf("  GYRO: X:%6d\tY:%6d\tZ:%6d  \n",s_gyroXYZ.x,s_gyroXYZ.y,s_gyroXYZ.z);
        UARTprintf("  BRUJ: %d\n", brujula);
        UARTprintf("  GIRO:  X:%d\tY:%d\nANG:%d", oriX, oriY, angulo);
        UARTprintf("\n------------------------------------------------\n");
    }

    return 0;
}


void Timer0IntHandler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Borra flag
	tiempo++;
	Flag_ints=1;
}

