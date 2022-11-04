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

int Fin_Rx=0;
char Buffer_Rx;
unsigned long POSX, POSY, BufferXY;
unsigned long POSYANT=0;
unsigned int CMD_Offset = 0;
unsigned long REG_TT[6];
const unsigned long REG_CAL[6]={21959,177,4294145463,14,4294950369,16094853};
const int32_t REG_CAL5[6]={32146, -1428, -331110, -40, -18930, 18321010};


int estado=0;

#define NUM_SSI_DATA            3

int RELOJ;

volatile int pul_1=0, pul_2=0;

int r_c, g_c, b_c;

int main(void) {

	int i;
	RELOJ=SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
	HAL_Init_SPI(2, RELOJ);  //Boosterpack a usar, Velocidad del MC
	Inicia_pantalla();
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
	    ComTXT(HSIZE/2,VSIZE/5, 22, OPT_CENTERX,"Seleccion de colores RGB");
	    ComTXT(HSIZE/2,50+VSIZE/5, 22, OPT_CENTERX," SEPA GIERM. 2021 ");
	    ComTXT(HSIZE/2,100+VSIZE/5, 20, OPT_CENTERX,"M.A.P.E.");

	    ComRect(40,40, HSIZE-40, VSIZE-40, false);
	    Dibuja();
	    Espera_pant();


#ifdef VM800B35
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL[i]);
#endif
#ifdef VM800B50
    for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
#endif

	r_c=128;
    g_c=128;
    b_c=128;

	while(1){
    	Lee_pantalla();
    	if(POSX!=0x8000){
    		Nueva_pantalla(60,60,60);

    		/* Pintar Scrollbars y leer valores rgb*/

    		//R
    		ComFgcolor(200,10,10);
    		ComBgcolor(150,5,5);
    		if(POSX>35 && POSX<HSIZE-35 && POSY>130 && POSY <155) r_c=((POSX-35)*255)/(HSIZE-70);
    		ComScrollbar(35,130,HSIZE-70,25,0,r_c,0,255);
    		//G
    		ComFgcolor(10,200,10);
    		ComBgcolor(5,150,5);
    		if(POSX>35 && POSX<HSIZE-35 && POSY>170 && POSY <195) g_c=((POSX-35)*255)/(HSIZE-70);
    		ComScrollbar(35,170,HSIZE-70,25,0,g_c,0,255);
    		//B
    		ComFgcolor(10,10,200);
    		ComBgcolor(5,5,150);
    		if(POSX>35 && POSX<HSIZE-35 && POSY>210 && POSY <235) b_c=((POSX-35)*255)/(HSIZE-70);
    		ComScrollbar(35,210,HSIZE-70,25,0,b_c,0,255);

    		/*Dibujar c�rculo con ese color y borde negro*/

    		ComColor(0,0,0);
            ComCirculo(HSIZE/2, 65, 60);

    		ComColor(r_c,g_c,b_c);
            ComCirculo(HSIZE/2, 65, 58);

    		/* Mostrar valor num�rico del color elegido */
    		ComColor(200,200,200);
    		ComTXT(HSIZE/2+70,20,28,0,"R:");
    		ComTXT(HSIZE/2+70,50,28,0,"G:");
    		ComTXT(HSIZE/2+70,80,28,0,"B:");

    		ComNum(HSIZE/2+100,20,28,0,r_c);
    		ComNum(HSIZE/2+100,50,28,0,g_c);
    		ComNum(HSIZE/2+100,80,28,0,b_c);


    		Dibuja();
    	}




    }

}






