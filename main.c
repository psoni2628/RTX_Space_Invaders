#include <LPC17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "Dependencies/GLCD.h"
#include "Dependencies/random.h"
#include "Bitmaps/bitmap.h"

#define initialPositionY 0
#define CENTERUSERX 106

typedef struct{
	uint8_t y,x;
} ship;

ship userShip = {0,CENTERUSERX};

ship bullet = {26,120-1};
bool bulletactive = false;
	
void userIO(void *arg){
	while (true) {
		uint32_t joystickLeft = (LPC_GPIO1->FIOPIN & 0x800000);
		uint32_t joystickRight = (LPC_GPIO1->FIOPIN & 0x2000000);
		uint32_t pushbutton = (LPC_GPIO2->FIOPIN & 0x400);
		
		if (!joystickLeft && userShip.x>0) {
				userShip.x--;
				printf("%d\n",userShip.x);
		}
		else if (!joystickRight && userShip.x<240-24){
			userShip.x++;
			printf("RIGHT,%d",userShip.x);
		}
		if(!pushbutton){
			bulletactive = true;
			bullet.x=userShip.x;
			printf("posh");
		}
		
	osDelay(osKernelGetTickFreq()*0.005);
	}
}

void bulletMove(void *arg){
		while(true){
			if(bulletactive){
				bullet.y++;
				osDelay(osKernelGetTickFreq()*0.0005);
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
		GLCD_Bitmap(0,userShip.x,24,26,(unsigned char*)user_bm); //gets you to top right
		GLCD_Bitmap(0,userShip.x+26,24,10,(unsigned char*)black); //gets you to top right
		GLCD_Bitmap(0,userShip.x-10,24,10,(unsigned char*)black); //gets you to top right
		GLCD_Bitmap(40,0,4,240,(unsigned char*)line);
		GLCD_Bitmap(bullet.y,bullet.x,20,1,(unsigned char*)line); //bullet
		GLCD_Bitmap(bullet.y+20,bullet.x,5,1,(unsigned char*)black); //gets you to top right
		GLCD_Bitmap(bullet.y-5,bullet.x,5,1,(unsigned char*)black); //gets you to top right
		GLCD_Bitmap(320-24,240-25,24,25,(unsigned char*)enemy_bm);
		displayLED((uint32_t)1);
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
