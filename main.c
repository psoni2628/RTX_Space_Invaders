#include <LPC17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "Dependencies/GLCD.h"
#include "Dependencies/random.h"
#include "Bitmaps/bitmap.h"

#define initialPositionY 0
#define CENTERUSERX 106

#define MAX_X 240
#define MAX_Y 320

#define USER_W 26
#define USER_H 24

#define ENEMY_W 25
#define ENEMY_H 24

#define BULLET_W 1
#define BULLET_H 20


typedef struct{
	uint16_t y;
	uint8_t x;
} displayObjects;

displayObjects userShip = {0,CENTERUSERX};
displayObjects bullet = {26,120-1};
bool bulletactive = false;
	
void userIO(void *arg){
	while (true) {
		uint32_t joystickLeft = (LPC_GPIO1->FIOPIN & 0x800000);
		uint32_t joystickRight = (LPC_GPIO1->FIOPIN & 0x2000000);
		uint32_t pushbutton = (LPC_GPIO2->FIOPIN & 0x400);
		
		if (!joystickLeft && userShip.x>0) {
				userShip.x--;
		}
		else if (!joystickRight && userShip.x<240-24){
			userShip.x++;
		}
		if(!pushbutton && !bulletactive){
			bulletactive = true;
			bullet.x=userShip.x+26/2-1;
		}
		
	osDelay(osKernelGetTickFreq()*0.005);
	}
}

void bulletMove(void *arg){
		while(true){
			if(bulletactive){
				osDelay(osKernelGetTickFreq()*0.05);
				bullet.y+=10;
				if (bullet.y > 330)  {
					bulletactive = false;
					bullet.y = 26;
				}
			}
				osThreadYield();
		}
}

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
	GLCD_Init();
	GLCD_Clear(Black);
	GLCD_SetBackColor(Black);
	while (true) {
		GLCD_Bitmap(0,userShip.x,24,26,(unsigned char*)user_bm);
		GLCD_Bitmap(0,userShip.x+26,24,10,(unsigned char*)black); // right ship cover
		GLCD_Bitmap(0,userShip.x-10,24,10,(unsigned char*)black); // left ship cover
		GLCD_Bitmap(40,0,4,240,(unsigned char*)line);
		if (bulletactive) {
			GLCD_Bitmap(bullet.y,bullet.x,20,1,(unsigned char*)line); //bullet
			GLCD_Bitmap(bullet.y-10,bullet.x,10,1,(unsigned char*)black); //gets you to top right
		}
		GLCD_Bitmap(320-24,240-25,24,25,(unsigned char*)enemy_bm);
		displayLED((uint32_t)1);
		//GLCD_Clear(Black);
		osThreadYield();
	}
}

int main(void) {
	printf("BEGIN\n");
	SystemInit(); 
	osKernelInitialize();
	osThreadNew(GLCD_Display, NULL, NULL);
	osThreadNew(userIO, NULL, NULL);
	osThreadNew(bulletMove, NULL, NULL);
	osKernelStart();
	while(true);
}
