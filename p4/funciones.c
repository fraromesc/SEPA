/*
	WIDGETS NECESARIOS	 funciones en pantalla	  func en libreria
	[c]-> Gauge (reloj)	|	x			|	-	
	[c] -> Termometro  	|	-			|	-
	[c] -> Progress Bar	|	x			|	-
	[c] -> Slider		|	x			|	-


*/

//FUNCION GAUGE
//ref   page 176 "FT800 Series ProgrammerGuide"
//Se ha cambiado 'range' por 'min' y 'max'. range=max-min
//El gauge parte desde 0, pero se ha cambiado para que el dato de partida 'min' sea otro parámtero
void ComGauge( int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t major, uint16_t minor , uint16_t val, int_t min, uint16_t max)
{
		//Sacar el gauge predeterminado
		EscribeRam32(CMD_GAUGE);
	    	EscribeRam16(x);
		EscribeRam16(y);
	    	EscribeRam16(r);
		EscribeRam16(options);
		EscribeRam16(major);
		EscribeRam16(minor);
		EscribeRam16(val-min); 
		EscribeRam16(max-min); //corresponde a range del CMD_GAUGE
		//Escribir valores max y min
		//AJUSTAR A OJO la FONT Y DONDE SE COLOCAN LOS NÚMEROS y la UNIDAD
		char c[5];
		sprintf(c, "%d", max); 
		ComTXT(x+r*3/4, y+r3/5, 27, OPT_CENTER, c);
		sprintf(c, "%d", min 
		ComTXT(x-y*3/4, y+r3/5, 27, OPT_CENTER, c);
		sprintf(c, "mBar"); 			
		ComTXT(x, y+r3/5, 23, OPT_CENTER, c);	
}


//PROGRESS BAR
//ref   page 191 "FT800 Series ProgrammerGuide"
//Se ha cambiado 'range' por 'min' y 'max'. range=max-min
//El gauge parte desde 0, pero se ha cambiado para que el dato de partida 'min' sea otro parámtero

void ComProgess(int16_t x, int16_t y, int16_t w, int16_t h, int16_t options, int16_t val, int16_t min, int16_t max)
{
		EscribeRam32(CMD_PROGRESS);
	    	EscribeRam16(x);
		EscribeRam16(y);
	    	EscribeRam16(w);
	    	EscribeRam16(h);
		EscribeRam16(options);
		EscribeRam16(val-min); 
		EscribeRam16(max-min); 	//corresponde a range del CMD_PROGESS
		EscribeRam16(0); 		//Completar memoria RAM
}

//FUNCIÓN SLIDE
void cmd_slide(int16_t x, int16_t y, int16_t w, int16_t h, int16_t options, int16_t val, int16_t range)
{		
		EscribeRam32(CMD_SLIDER);
	    	EscribeRam16(x);
		EscribeRam16(y);
	    	EscribeRam16(w);
	    	EscribeRam16(h);
		EscribeRam16(options);
		EscribeRam16(val);
		EscribeRam16(range); //corresponde a range del CMD_SLIDER
}

int ComSlide(int16_t x, int16_t y, int16_t w, int16_t h, int16_t options)
{
	int min=0, max=100, val=0; 
	Lee_pantalla(); 
	if (POSX>x && POSX<(x+w) && POSY>y &&POSY<(y+h))
		val=(POSX-x)*(max-min)*w; 
	cmd_slide(x,y,w,h,options, val-min, max-min); 
	return value; 

}


/*
val			POSX-x
-----------======--------------
(max-min)		w
*/


//TERMÓMETRO
void ComTermometro(int x, int y, int H, int val, int min, int man)
{
	int R=H/7; 		//Radio mayor
	int r=0.8*R; 		//Radio menor
	int W=H/5; 		//Ancho mayor
	int w=0.8; 		//Ancho menor
	int yC=y-R; 		//y del centro bajo
	int h=H*(4.0/5)-R; 			//Alto rectangulo interno 
	int hVal=val*h/(max-min);	//altura de la temperatura
 
	//Fondo
	ComColor(149,254,232);	//Azulito
	ComCirculo(x,yc,R); 
	ComRect(x-W/2, yc, x+W/2, yc-h, 1);
	ComCirculo(x,yc-h, W/2); 
	 
	//Parte temperatura
	ComColor(255,0,0);
	ComCircle(x,yc,r); 
	ComRect(x-w/2, yc, x+w/2,yc+hVal, 1); 
	ComCircle(x, yc+hVal, w/2); 		

}



/*
	val			hVal
-----------===------------
(max-min)			h

*/
