/*
 * main.c
 */


#include <stdint.h>
#include <stdbool.h>
#include "driverlib2.h"

#include "FT800_TIVA.h"


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



#define NUM_SSI_DATA            3

int RELOJ;


int main(void)
{


    int i;

    RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
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


    Nueva_pantalla(16,16,16);
    ComColor(21,160,6);
    ComLineWidth(5);
    ComRect(10, 10, HSIZE-10, VSIZE-10, true);
    ComColor(65,202,42);
    ComRect(12, 12, HSIZE-12, VSIZE-12, true);

    ComColor(255,255,255);
    ComTXT(HSIZE/2,VSIZE/5, 22, OPT_CENTERX,"EJEMPLO de PANTALLA");
    ComTXT(HSIZE/2,50+VSIZE/5, 22, OPT_CENTERX," SEPA GIERM. 2021 ");
    ComTXT(HSIZE/2,100+VSIZE/5, 20, OPT_CENTERX,"M.A.P.E.");

    ComRect(40,40, HSIZE-40, VSIZE-40, false);


    Dibuja();
    Espera_pant();

#ifdef VM800B35
    for(i=0;i<6;i++)	Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL[i]);
#endif
#ifdef VM800B50
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
#endif

    while(1){
        Lee_pantalla();

        Nueva_pantalla(16,16,16);

        ComGradient(0,0,GRIS_CLARO,0,240,GRIS_OSCURO);
        ComColor(255,0,0);
        ComFgcolor(200, 200, 10);

        if(Boton(10, 90, 50, 50, 28, "L"))
        {
            Xp--; if (Xp<=XpMin)  Xp=XpMin;
        }

        if(Boton(130, 90, 50, 50, 28, "R"))
        {
            Xp++; if (Xp>=XpMax) Xp=XpMax;
        }


        ComFgcolor(10, 100, 100);
        ComColor(200, 200, 200);
        if(Boton(70, 30, 50, 50, 28, "U"))
        {
            Yp--; if (Yp<=YpMin) Yp=YpMin;
        }
        if(Boton(70, 150, 50, 50, 28, "D"))
        {
            Yp++; if (Yp>=YpMax) Yp=YpMax;
        }

        if(POSX>XpMin && POSX<XpMax && POSY>YpMin && POSY<YpMax) //Pulsando en el rectangulo
        {
            Xp=POSX;    //Llevar la bola a la posición
            Yp=POSY;
            ComColor(0xff,0xE0,0xE0);   //Fondo rosa

        }
        else
        {
            ComColor(0xff,0xff,0xff);   //Fondo blanco
        }

        ComRect(200, 30, 310, 210, true);
        ComColor(0x30,0x50,0x10);
        ComLineWidth(3);

        ComRect(200, 30, 310, 210, false);
        ComCirculo(Xp, Yp, 20);

        Dibuja();
    }

}






