#include <LPC17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "Dependencies/GLCD.h"
#include "Dependencies/random.h"
#include "Bitmaps/bitmap.h"

//  p = [int(0xFFFF/0xFF*i) for i in v]

void displayLED(uint32_t num){
	LPC_GPIO1->FIODIR |= 0xB0000000;
	LPC_GPIO2->FIODIR |= 0x0000007C;
	
	LPC_GPIO1->FIOCLR |= 0xB0000000;
	LPC_GPIO2->FIOCLR |= 0x0000007C;
	
	uint32_t bit_28 = (num & 0x00000080);
	bit_28 = bit_28 << 21;
	LPC_GPIO1->FIOSET |= bit_28;
	
	uint32_t bit_29 = (num & 0x00000040);
	bit_29 = bit_29 << 23;
	LPC_GPIO1->FIOSET |= bit_29;
	
	uint32_t bit_31 = (num & 0x00000020);
	bit_31 = bit_31 << 26;
	LPC_GPIO1->FIOSET |= bit_31;
	
	uint32_t bit_2 = (num & 0x00000010);
	bit_2 = bit_2 >> 2;
	LPC_GPIO2->FIOSET |= bit_2;
	
	uint32_t bit_3 = (num & 0x000000008);
	LPC_GPIO2->FIOSET |= bit_3;
	
	uint32_t bit_4 = (num & 0x00000004);
	bit_4 = bit_4 << 2;
	LPC_GPIO2->FIOSET |= bit_4;
	
	uint32_t bit_5 = (num & 0x00000002);
	bit_5 = bit_5 << 4;
	LPC_GPIO2->FIOSET |= bit_5;

	uint32_t bit_6 = (num & 0x00000001);
	bit_6 = bit_6 << 6;
	LPC_GPIO2->FIOSET |= bit_6;

}

void GLCD_Display(void *arg) {
	char c[1]="|";
	GLCD_Init();
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);
	GLCD_SetTextColor(White);
	//for (int col = 0; col < 8; col++) {
		
	//}
	
	int i=0;
	char i_c[10];
	while (true) {
		sprintf(i_c,"%d",i);
		osDelay(osKernelGetTickFreq()*1);
		//for (int col = 0; col < 8; col++) {
			//GLCD_DisplayString(i,3,1,(unsigned char *)c);
		//}
		//GLCD_Bitmap(320-24,240-26,24,26,(unsigned char*)user_bm); //gets you to top right
		GLCD_Bitmap(0,106,24,26,(unsigned char*)user_bm); //gets you to top right
		GLCD_Bitmap(40,0,4,240,(unsigned char*)line);
		GLCD_Bitmap(26,119,20,1,(unsigned char*)line);
		GLCD_Bitmap(320-24,240-25,24,25,(unsigned char*)enemy_bm);
		//GLCD_DisplayString(0,0,1,(unsigned char *)c);
		//GLCD_DisplayString(6,8,1,(unsigned char *)i_c);
		//if(i<10)
		//	i++;
		//if(i>10)
		//	break;
	}

}

int main(void) {
	printf("BEGIN\n");
	SystemInit(); 
	osKernelInitialize();
	osThreadNew(GLCD_Display, NULL, NULL);
	osKernelStart();
	while(true);
	
	
	
}
