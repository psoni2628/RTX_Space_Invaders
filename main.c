#include <LPC17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "Dependencies/GLCD.h"
#include "Dependencies/random.h"
#include "Bitmaps/bitmap.h"

#define initialPositionY 0
#define CENTERUSERX 106


#define MIN_X 0
#define MIN_Y 0
#define MAX_X 240
#define MAX_Y 320

#define USER_W 26
#define USER_H 24
#define USER_COVER_W 10
#define USER_COVER_H 24

#define BULLET_W 1
#define BULLET_H 20
#define BULLET_COVER_W 1
#define BULLET_COVER_H 10

#define ENEMY_W 25
#define ENEMY_H 24

#define DIV_LINE_W MAX_X
#define DIV_LINE_H 4

#define NUM_ENEMIES 10

typedef struct{
	uint16_t y;
	uint8_t x;
	uint16_t width;
	uint16_t height;
	uint16_t *bm;
} displayObjects;

/*
 * Objects that will be displayed to screen
 * 
 * Dividing line
 * User ship
 * User cover
 * Bullet
 * Bullet cover
 * Enemy ships
 */

displayObjects divisionLine = {40,0,DIV_LINE_W,DIV_LINE_H,line};
displayObjects userShip = {0,CENTERUSERX,USER_W,USER_H,user_bm};
displayObjects userShipCover = {0,0,USER_COVER_W,USER_COVER_H,black};
displayObjects bullet = {26,120-1,BULLET_W,BULLET_H,line};
displayObjects bulletCover = {0,0,BULLET_COVER_W,BULLET_COVER_H,black};
displayObjects enemyShip = {0,0,ENEMY_W,ENEMY_H,enemy_bm};
bool bulletactive = false;

osMutexId_t bulletMutex;

displayObjects enemyArr[NUM_ENEMIES];
bool active[NUM_ENEMIES];


void userIO(void *arg){
	while (true) {
		uint32_t joystickLeft = (LPC_GPIO1->FIOPIN & 0x800000);
		uint32_t joystickRight = (LPC_GPIO1->FIOPIN & 0x2000000);
		uint32_t pushbutton = (LPC_GPIO2->FIOPIN & 0x400);
		
		if (!joystickLeft && userShip.x > MIN_X) {
			userShip.x--;
			osDelay(osKernelGetTickFreq()*0.005);
		}
		else if (!joystickRight && userShip.x < MAX_X-USER_H){
			userShip.x++;
			osDelay(osKernelGetTickFreq()*0.005);
		}
		
		osMutexAcquire(bulletMutex, osWaitForever);
		if(!pushbutton && !bulletactive){
			bulletactive = true;
			bullet.x = userShip.x + userShip.width/2 - bullet.width;
		}
		osMutexRelease(bulletMutex);
		
		osThreadYield();
	}
}

void bulletMove(void *arg){
	while(true){
		
		osMutexAcquire(bulletMutex, osWaitForever);
		if(bulletactive){
			osDelay(osKernelGetTickFreq()*0.05);
			bullet.y+=10;
			if (bullet.y > MAX_Y+10)  {
				bulletactive = false;
				bullet.y = userShip.height+2;
			}
		}
		osMutexRelease(bulletMutex);
		
		osThreadYield();
	}
}


void enemyGen(void *arg) {
	bool min=false;
	for (int i = 0; i < NUM_ENEMIES; i++) {
		enemyArr[i].x = i*24;
		enemyArr[i].y = 320-ENEMY_H;
		enemyArr[i].width = ENEMY_W;
		enemyArr[i].height = ENEMY_H;
		enemyArr[i].bm = enemy_bm;
	}
	
	while (true) {
		for (int i = 0; i < NUM_ENEMIES; i++) {
			enemyArr[i].y-=5;
		}
		osDelay(osKernelGetTickFreq()*1);
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
		GLCD_Bitmap(0, userShip.x, userShip.height, userShip.width, (unsigned char*)userShip.bm); // user ship
		GLCD_Bitmap(0, userShip.x+26, userShipCover.height, userShipCover.width,(unsigned char*)userShipCover.bm); // right ship cover
		GLCD_Bitmap(0, userShip.x-10, userShipCover.height, userShipCover.width,(unsigned char*)userShipCover.bm); // left ship cover
		GLCD_Bitmap(40, 0, divisionLine.height, divisionLine.width, (unsigned char*)divisionLine.bm); // division line
		
		osMutexAcquire(bulletMutex, osWaitForever);
		if (bulletactive) {
			GLCD_Bitmap(bullet.y, bullet.x, bullet.height, bullet.width, (unsigned char*)bullet.bm); // bullet
			GLCD_Bitmap(bullet.y-10, bullet.x, bulletCover.height, bulletCover.width, (unsigned char*)bulletCover.bm); // bullet cover
		}
		osMutexRelease(bulletMutex);
		for (int i = 0; i < NUM_ENEMIES; i++) {
			GLCD_Bitmap(enemyArr[i].y, enemyArr[i].x, enemyArr[i].height, enemyArr[i].width, (unsigned char*)enemyShip.bm);
		}
		//GLCD_Bitmap(320-24, 240-25, enemyShip.height, enemyShip.width, (unsigned char*)enemyShip.bm); // enemy ship
		//displayLED((uint32_t)1);
		//GLCD_Clear(Black);
		osThreadYield();
	}
}

int main(void) {
	printf("BEGIN\n");
	SystemInit();
	bulletMutex = osMutexNew(NULL);
	
	osKernelInitialize();
	
	osThreadNew(GLCD_Display, NULL, NULL);
	osThreadNew(userIO, NULL, NULL);
	osThreadNew(bulletMove, NULL, NULL);
	osThreadNew(enemyGen, NULL, NULL);
	
	osKernelStart();
	while(true);
}
