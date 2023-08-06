// SpaceInvaders.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 8/24/2022 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/ST7735.h"
#include "Random.h"
#include "TExaS.h"
#include "../inc/ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer1.h"
#include "Timer0.h"
#include "Key.h"

typedef struct bullet{
	int x;
	int y;
	int valid;
	
} bullet_t;

typedef struct player{
	int x;
	int y;
	int prev_x; // for slider glitch prevention
	int hp; 
	int valid;
} player_t;

typedef struct enemy{
	int x;
	int y;
	int prev_x;
	int prev_y;
	int hp;
	int valid;
	int enemy_type; 
	
} enemy_t;

int has_lost=0;
#define bullet_num 10
bullet_t bullet_array[bullet_num]; //array is used for storing bullets, chose 10 to allow user to shoot many bullets per second 
#define enemy_num 10
enemy_t enemy_array[enemy_num];
player_t player0;

uint64_t frame_number =0 ; //if frame number run out of bound, will cause bug, but it is almost impossible

int find_unused_bullet()
{
	for(int i = 0 ; i < bullet_num ; i++)
	{
		if(bullet_array[i].valid == 0)
		{
			return i;
		}
	}
	return 0;
}

void spawn_bullet(int x, int y)
{
	int i = find_unused_bullet();
	bullet_array[i].x = x;
	bullet_array[i].y = y;
	bullet_array[i].valid = 1;
}

void do_shoot(void)
{	
	static int next_frame_num=0;
	if( ((Key_In()& 0x01) == 0x01 ) && frame_number > next_frame_num ){
		spawn_bullet(player0.x,player0.y);
		Sound_Shoot();
		next_frame_num = frame_number+20; //   60/20 = 3, which allows for us to shoot it 3 times per second  (must be greater than 20 frames to shoot)
	}
}
void player_init()
{
	player0.x = 64;
	player0.y= 150;
	player0.hp = 1;
}

void bullet_init()
{
	for(int i =0 ; i < bullet_num ; i++)
	{
		bullet_array[i].valid = 0;
	}
}
void enemy_init()
{
	for(int i =0 ; i < enemy_num ; i++)
	{
		enemy_array[i].valid = 0;
	}
}



int find_unused_enemy()
{
	for(int i =0 ; i < enemy_num ; i++)
	{
		if(enemy_array[i].valid == 0)
		{
			return i;
		}
	}
	return 0;
}

void draw_player(int x, int y)
{
	if( ( x-9) < 0 )
		x = 0;
	else
		x +=-9;
	
	if( y + 4 > 160 )
		y = 150;
	else
		y +=4;
	
	ST7735_DrawBitmap(x, y, PlayerShip0, 18,8); // player ship bottom	
}



void draw_enemy(int x, int y, int type)
{
	if( ( x-8) < 0 )
		x = 0;
	else
		x +=-8;
	
	if( y + 5 > 160 )
		y = 150;
	else
		y +=5;
	
	switch(type){
	case 0: ST7735_DrawBitmap(x, y, SmallEnemy10pointA, 16,10);
	break;
	case 1: ST7735_DrawBitmap(x, y, SmallEnemy10pointB, 16,10);
	break;
	case 2: ST7735_DrawBitmap(x, y, SmallEnemy10pointA, 16,10);
	break;
	case 3: ST7735_DrawBitmap(x, y, SmallEnemy10pointB, 16,10);
	break;
	default: ST7735_DrawBitmap(x, y, SmallEnemy10pointA, 16,10);
	break;
	}
}




void enemy_spawn(int x, int y, int type_of_enemy)
{
	int i = find_unused_enemy();
	enemy_array[i].x = x; //arrays purpose 
	enemy_array[i].y = y;
	enemy_array[i].valid = 1;
	enemy_array[i].hp = 1;
	enemy_array[i].enemy_type = type_of_enemy; 
}

void render(){
	
	//render bullet
	for(int i =0 ; i < bullet_num ; i++)
	{
		if( bullet_array[i].valid == 1){
		 ST7735_DrawPixel(bullet_array[i].x , bullet_array[i].y , ST7735_CYAN); //
		 ST7735_DrawPixel(bullet_array[i].x , bullet_array[i].y+1, ST7735_BLACK);
		}
		else
		{
			if(bullet_array[i].y == -1){
				ST7735_DrawPixel(bullet_array[i].x , 0 , ST7735_BLACK);
			}
		}
	}
	//render enemy
	for(int i =0 ; i < enemy_num ; i++)
	{
		if( enemy_array[i].hp != 0 && enemy_array[i].valid == 1){
			draw_enemy(enemy_array[i].x, enemy_array[i].y, enemy_array[i].enemy_type);
		}
		if( enemy_array[i].hp == 0 && enemy_array[i].valid == 1){
			ST7735_FillRect(enemy_array[i].x-8, enemy_array[i].y-5, 16, 10, ST7735_BLACK);
		}
	}
	
	
	//render player
	if( (player0.prev_x > player0.x) && (player0.prev_x - player0.x > 2))
		ST7735_FillRect(player0.prev_x, player0.y-3, 15, 8, ST7735_BLACK);
	if( (player0.prev_x < player0.x) && (player0.x - player0.prev_x > 2) )
		ST7735_FillRect(player0.prev_x-15, player0.y-3, 15, 8, ST7735_BLACK);
	draw_player(player0.x,player0.y);
}
//return 1 if touched, return zero if not touched
int touched_enemy(int enemy_x, int enemy_y, int w, int h, int bullet_x, int bullet_y)
{
	int top_y = enemy_y - h/2; //takes enemy position and calculates all four corners for bullet detection (ie. 50 is width, then bottom right would be 50, 50
 	int bottom_y= enemy_y + h/2; //25 + 50/2 = 50
	int left_x = enemy_x - w/2;  //bulled defined by center position 
	int right_x = enemy_x + w/2;
	
	if( (bullet_y > top_y )&& (bullet_y < bottom_y) && (bullet_x > left_x ) && (bullet_x < right_x )  )
		return 1;
	else
		return 0;
	
}
int score = 0; 
void update_bullet(){
	for(int i =0 ; i < bullet_num ; i++)
	{
		if(bullet_array[i].valid == 1)
		{
			bullet_array[i].y -=1;
			if(bullet_array[i].y == -1){
				bullet_array[i].valid =0;
			}
			for(int j =0; j < enemy_num ;  j++)
			{
					if(enemy_array[j].hp >0){
						if(touched_enemy( enemy_array[j].x, enemy_array[j].y , 16 , 10 ,bullet_array[i].x, bullet_array[i].y ))
						{
							Sound_Explosion();
							score += 10; 
							enemy_array[j].hp -=1;
							bullet_array[i].valid = 0;
							ST7735_DrawPixel(bullet_array[i].x , bullet_array[i].y+1 , ST7735_BLACK);
							ST7735_DrawPixel(bullet_array[i].x , bullet_array[i].y , ST7735_BLACK);
							ST7735_DrawPixel(bullet_array[i].x , bullet_array[i].y-1 , ST7735_BLACK);

						}
				}
			}
		}
		
	}
}

void update_enemy(){
	for(int i =0 ; i < enemy_num ; i++)
	{
		if(enemy_array[i].valid == 1 && enemy_array[i].hp > 0 )
		{
			if(frame_number % 10 == 0  && enemy_array[i].y  <= 145 ) //so that enemy is moving more slowly
				enemy_array[i].y +=1;
			if(enemy_array[i].y  == 145)
				has_lost=1;
		}
	}
}
int testdata;
void update_player(){
	do_shoot();

	player0.prev_x = player0.x;
	testdata = ADC_In();
	int x = ADC_In() *127 / 4096;
	if( ( x-9) < 0 )
		x = 0;
	else
		x +=-9;
	
	player0.x = x;
}

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds


void Timer1A_Handler(void){ // can be used to perform tasks in background
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
   // execute user task
}

int main0(void){
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Random_Init(1);

  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom

  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);

  Delay100ms(50);              // delay 5 sec at 80 MHz
//need to set the flag
//while(Flag = 0), Flag == 1
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  while(1){
  }

}


// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
      time--;
    }
    count--;
  }
}
typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

int main2(void){ char l;
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Delay100ms(30);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Delay100ms(20);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }  
}

void start_timer0(){
	Timer0_Init(1333333,0);
	
}
void init_everything(void){
	DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Output_Init();
	ADC_Init();
	Sound_Init();
	Key_Init();
	Timer0_Init(1333333,0); //60times per second // (1/60 * 10^9)  / 12/5
	EnableInterrupts();
	ST7735_FillScreen(0x0000);
	bullet_init();
	player_init();

	enemy_spawn(20,9, 0);
	enemy_spawn(40,9, 1);
	enemy_spawn(60,9, 2);
	enemy_spawn(80,9, 3);
	enemy_spawn(100,9, 4);
}

void beginning_screen(void){
	ST7735_DrawBitmap(50, 140, SmallEnemy30pointB, 16,10);
	ST7735_OutString("Welcome to ");
	ST7735_OutString("\n");
	ST7735_OutString("Space Invaders!");
	ST7735_OutString("\n");
	ST7735_OutString("\n");
	ST7735_OutString("¡Bienvenidos a los");
	ST7735_OutString("\n");	
	ST7735_OutString("invasores del");
	ST7735_OutString("\n");	
	ST7735_OutString("espacio!");	
	ST7735_OutString("\n");
	ST7735_OutString("\n");
	ST7735_OutString("Hit the second key");
	ST7735_OutString("\n");
	ST7735_OutString("to begin!");
	ST7735_OutString("\n");
	ST7735_OutString("\n");
	ST7735_OutString("Presione tecla dos");
	ST7735_OutString("\n");
	ST7735_OutString("empezar");
	

}

int is_paused = 0;
int main(void)
{
	init_everything();
	beginning_screen(); 
	while( (Key_In()& 0x01) == 0 ){
	}
	ST7735_FillScreen(0x0000);
	Timer0_start(); 
	while(1)
	{
		//uint32_t position=ADC_In();
		//ST7735_SetCursor(0,0);
		if(has_lost)
		{
			Timer0_stop();
			ST7735_FillScreen(0x0000);
			ST7735_SetCursor(0,0);
			ST7735_OutUDec(score);
			ST7735_OutString("\n");
			ST7735_OutString("Earth is Lost!");
			ST7735_OutString("\n");
			ST7735_OutString("¡La tierra esta"); 
			ST7735_OutString("\n");
			ST7735_OutString("perdida!");
			while(1){}
		}
		else if(score == 50)
		{
			Timer0_stop();
			ST7735_FillScreen(0x0000);
			ST7735_SetCursor(0,0);
			ST7735_OutUDec(score);
			ST7735_OutString("!");
			ST7735_OutString("\n");
			ST7735_OutString("You Saved Earth!");
			ST7735_OutString("\n");
			ST7735_OutString("¡Salvaste La Tierra!"); 
			while(1){}
		}
		if( (Key_In() & 0x01 == 0x01) && (is_paused == 1) )
		{
			ST7735_FillScreen(0x0000);
			Timer0_start();
			is_paused = 0; 		
		}
		
	}
}


void Timer0A_Handler(void){ // repeates 60 times per second to refresh all positions 
  TIMER0_ICR_R = 0x01;   // acknowledge TIMER0A timeout
  // output one value to DAC if a sound is active
	
	
	//do_once_every(do_shoot,20);
	

	
	
	//check_reach_end();
	update_enemy(); //update enemy position ;check if enemy touched bullet, if touched, killed it and remove from screen;
	update_bullet();
	update_player(); //update player position; do shooting
	
	render();
	
	frame_number++; //use this for delay purposes so that we can see bullets 
	
	if( (Key_In() & 0x02) == 0x02 && is_paused == 0 ){
		Timer0_stop(); 
		ST7735_FillScreen(0x0000);
		ST7735_SetCursor(0,0);
		ST7735_OutString("Game Paused");
		ST7735_OutString("\n");
		ST7735_OutString("Juego Pausado");
		is_paused = 1; 
		
	}
	
}
