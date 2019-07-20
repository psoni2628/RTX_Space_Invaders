#include <LPC17xx.h>
#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "Dependencies/GLCD.h"
#include "Dependencies/random.h"
#include "Bitmaps/bitmap.h"

// Default thread stack size = 200 bytes
// Idle thread stack size = 72 bytes
// Stack size = 0x2000

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
#define DIV_LINE_H 1

#define NUM_ENEMIES 10
#define MAX_POINTS 255

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
bool gameOver = false;
int totalPoints = 0;

osMutexId_t bulletMutex;
osMutexId_t enemyArrMutex;
osMutexId_t activeArrMutex;
osMutexId_t totalPointsMutex;

displayObjects enemyArr[NUM_ENEMIES];
bool active[NUM_ENEMIES] = {0,0,0,0,0,0,0,0,0,0};


void userIO(void *arg){
	while (true) {
		uint32_t joystickLeft = (LPC_GPIO1->FIOPIN & 0x800000);
		uint32_t joystickRight = (LPC_GPIO1->FIOPIN & 0x2000000);
		uint32_t pushbutton = (LPC_GPIO2->FIOPIN & 0x400);
		
		if (!joystickLeft && userShip.x > MIN_X) {
			userShip.x--;
			osDelay(osKernelGetTickFreq()*0.008);
		}
		else if (!joystickRight && userShip.x < MAX_X-USER_H){
			userShip.x++;
			osDelay(osKernelGetTickFreq()*0.008);
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
			
			for(int i=0;i<NUM_ENEMIES;i++){
				if(bullet.x<=(i+1)*24 && bullet.x>=i*24 && bullet.y>=enemyArr[i].y){
					active[i]=false;
					bullet.y = MAX_Y+10;
					totalPoints+=1;
				}
			}
		}
		osMutexRelease(bulletMutex);
		
		osThreadYield();
	}
}

void addEnemy(displayObjects* ship, uint8_t col){
		ship->x = col*24;
		ship->y = 320-ENEMY_H;
		ship->width = ENEMY_W;
		ship->height = ENEMY_H; 
		ship->bm = enemy_bm;
}

void enemy_control(void *arg) {
	int enemy_num = (int)arg;
	while (true) {
		osMutexAcquire(enemyArrMutex, osWaitForever);
		osMutexAcquire(activeArrMutex, osWaitForever);
		
		if(active[enemy_num]){
			enemyArr[enemy_num].y-=5;
			if (enemyArr[enemy_num].y <= 44){
				gameOver = true;
			}
		}
		else{
			osDelay((osKernelGetTickFreq() * next_event() * 1) >> 16);
			addEnemy(&enemyArr[enemy_num],enemy_num);
			active[enemy_num]=true;
		}
		
		osMutexRelease(activeArrMutex);
		osMutexRelease(enemyArrMutex);
		
		osDelay(osKernelGetTickFreq()*0.5);
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
		osMutexAcquire(totalPointsMutex, osWaitForever);
		displayLED(totalPoints);
		osMutexRelease(totalPointsMutex);

		if (gameOver) {
			GLCD_Clear(Black);
			GLCD_Bitmap(139,102,21,36,(unsigned char*)gameOver_bm);
			break;
		}
		if (totalPoints >= MAX_POINTS) {
			GLCD_Clear(Black);
			GLCD_Bitmap(155,99,11,42,(unsigned char*)youWin_bm);
			break;
		}
		GLCD_Bitmap(0, userShip.x, userShip.height, userShip.width, (unsigned char*)userShip.bm); // user ship
		GLCD_Bitmap(0, userShip.x+userShip.width, userShipCover.height, userShipCover.width,(unsigned char*)userShipCover.bm); // right ship cover
		GLCD_Bitmap(0, userShip.x-userShipCover.width, userShipCover.height, userShipCover.width,(unsigned char*)userShipCover.bm); // left ship cover
		GLCD_Bitmap(40, 0, divisionLine.height, divisionLine.width, (unsigned char*)divisionLine.bm); // division line
		
		osMutexAcquire(bulletMutex, osWaitForever);
		if (bulletactive) {
			GLCD_Bitmap(bullet.y, bullet.x, bullet.height, bullet.width, (unsigned char*)bullet.bm); // bullet
			GLCD_Bitmap(bullet.y-bulletCover.height, bullet.x, bulletCover.height, bulletCover.width, (unsigned char*)bulletCover.bm); // bullet cover
		}
		osMutexRelease(bulletMutex);
		
		osMutexAcquire(enemyArrMutex, osWaitForever);
		osMutexAcquire(activeArrMutex, osWaitForever);
		for (int i = 0; i < NUM_ENEMIES; i++) {
			if(active[i]){
				GLCD_Bitmap(enemyArr[i].y, enemyArr[i].x, enemyArr[i].height, enemyArr[i].width, (unsigned char*)enemyShip.bm);
			}
			else {
				GLCD_Bitmap(enemyArr[i].y, enemyArr[i].x, 25, 24, (unsigned char*)enemyCover);
				GLCD_Bitmap(enemyArr[i].y-bullet.height, enemyArr[i].x, 25, 24, (unsigned char*)enemyCover);
			}
		}
		
		osMutexRelease(activeArrMutex);
		osMutexRelease(enemyArrMutex);
		
		osThreadYield();
	}
}

int main(void) {
	SystemInit();
	bulletMutex = osMutexNew(NULL);
	enemyArrMutex = osMutexNew(NULL);
	activeArrMutex = osMutexNew(NULL);
	totalPointsMutex = osMutexNew(NULL);
	
	osKernelInitialize();
	
	osThreadNew(GLCD_Display, NULL, NULL);
	osThreadNew(userIO, NULL, NULL);
	osThreadNew(bulletMove, NULL, NULL);
	for (int i = 0; i < NUM_ENEMIES; i++) {
		osThreadNew(enemy_control, (void *)i, NULL);
	}
	
	osKernelStart();
	while(true);
}
