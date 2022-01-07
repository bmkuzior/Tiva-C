// LCD_main.c
// Runs on TM4C123
// Test the functions in ST7735.c by printing basic
// patterns to the LCD.
// 16-bit color, 128 wide by 128 high LCD
// Daniel Valvano
// March 30, 2015

// Edited: BRENDAN KUZIOR
// Date: 12/9/2021

//Libraries
#include <stdio.h>
#include <stdint.h>
#include "ST7735.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"
#include "SysTick.c"

//BIT SPECIFIC
#define DHT				      				(*((volatile unsigned long *)0x40005040))
#define RGB				      				(*((volatile unsigned long *)0x40045038))

#define	DIR_PIN4_In			~0x10
#define DIR_PIN4_Out 		0x10
#define Data_In 			~0x10
#define	High_In				0x10
#define Low_In 				~0x10
#define	High_Out			0x10
#define Low_Out 			0x00

//global vars
void PortF_Init(void);
void PortB_Init(void);
void DHT11_Start();
void DHT11_CheckResponse();
void DHT11_ReadData();
void cleanscreen(void);

unsigned long data[40];				//DATA String

uint32_t x = 4; 					//set LCD x coordinates
uint32_t y = 4;						//set LCD y coordinates

int main(void)
{

	int i=0,j=0;
	unsigned long I_RH,D_RH,I_Temp,D_Temp,CheckSum;
	unsigned long tem, hum, tf;

	PLL_Init();
	SysTick_Init();
	PortB_Init();
	PortF_Init();
	Output_Init();

	//LCD INTRODUCTION
	ST7735_SetCursor(x, y);
	printf("Starting program..");
	SysTick_Wait10ms(200);
	Output_Clear();
	ST7735_SetCursor(x, y);
	printf("This program \n will read \n the temperature \n and humidity\n of the room \n");
	SysTick_Wait10ms(200);
	Output_Clear();
	ST7735_SetCursor(x, y);


  while(1)
	{
		DHT11_Start();  /* send start pulses to DHT11 module */
		if(j++==0){										//Skips the First Iteration of the Data, this is due to error
			SysTick_Wait1ms(1000);
			continue;
		}
		DHT11_CheckResponse();  						// wait for responses from DHT11 module
		DHT11_ReadData(data);   						//READS the data points for the DHT

		I_RH = 	BtoD(0,7,data);							// Converts first 8 bits (Humidity Integrator bits) of the data into Decimal value
		D_RH 	= BtoD(8,15,data);						// Converts 8-16 bits of (Humidity DECIMAL bits) the data into Decimal value
		I_Temp = BtoD(16,23,data);						// Converts 17-24 bits of (Temperature Integrator bits) the data into Decimal value
		D_Temp =	BtoD(24,31,data);					// Converts 24-32 bits of (Temperature DECIMAL bits) the data into Decimal value
		CheckSum	=	BtoD(32,39,data);				// Converts last 8 bits of (Parity bits) the data into Decimal value
		tf = I_Temp*1.8+32;


		if(CheckSum != (I_RH + D_RH + I_Temp + D_Temp)){	//Function to check for errors in reads
				printf(" Error" );
			}
		else{
				printf(" No Error");
			}

		Output_Clear();
		ST7735_SetCursor(x, y);
		printf("\n Temp(C) %luC \n Temp(F) %luF", I_Temp, tf);
		printf("\n Humidity %lu %% \n", I_RH);

			if(I_Temp <= 25 && I_Temp >=20){		//Temp Range Check
				printf("\nTemp is OK!");
				GPIO_PORTF_DATA_R = 0x08;
			}
			else{
				printf("\nTemp is out of Range!");
				GPIO_PORTF_DATA_R = 0x02;
			}
			SysTick_Wait10ms(500);
			GPIO_PORTF_DATA_R = 0x00;
			if(I_RH >= 10 && I_RH <= 30){			//Humidity Range Check
				printf("\nHumidity is OK!");
				GPIO_PORTF_DATA_R= 0x0E;
			}
			else{
				printf("\nHumidity is out \n of Range!");
				GPIO_PORTF_DATA_R = 0x02;
			}

		SysTick_Wait10ms(500);
		GPIO_PORTF_DATA_R = 0x00;
		printf("\nNew Data... "); 							//Set LCD to prepare for NEW Data
		SysTick_Wait10ms(200);
		Output_Clear();
		ST7735_SetCursor(x, y);
		SysTick_Wait10ms(1);
	}
}


	// ports initialization functions
void DHT11_ReadData(void)
{
  int i=0;
	while(i<40){
	  while((GPIO_PORTB_DATA_R & 0x10) == Low_Out)			// Iterates untill the data line is low
		{}
		SysTick_Wait1us(45);
		if((GPIO_PORTB_DATA_R & 0x10) == Low_Out){			// Check after 45us
			data[i] = 0;
		}													// if data line is low // 0 bit by sensor
		else{												// else
			data[i] = 1;}									// 1 bit by sensor
		while((GPIO_PORTB_DATA_R & 0x10) == High_Out)		// Iterates till the data line is high
		{}
		i++;
	}
}

void DHT11_Start()
{
	GPIO_PORTB_DIR_R |= DIR_PIN4_Out;		// Set Pin B4 as output
	GPIO_PORTB_DATA_R &= Low_In;			// Set data line low for 1ms to awake the sensor
	SysTick_Wait1ms(20);
	GPIO_PORTB_DATA_R |= High_In;			// Set data line high for 30us
	SysTick_Wait1us(25);
	GPIO_PORTB_DIR_R &= DIR_PIN4_In;		// Set Pin B4 as Input
	}

void DHT11_CheckResponse()
{
	while((DHT & 0x10) == Low_Out)			// Iterates untill data line is low
		{}
	while((GPIO_PORTB_DATA_R & 0x10) == High_Out)	// Iterates untill data line is high
		{}
}


void PortB_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x02;      		// 1) B
	delay = SYSCTL_RCGC2_R;           	// delay for stabilization
	GPIO_PORTB_AMSEL_R &= ~0x10; 		// 3) disable analog function on PB5-0
	GPIO_PORTB_DIR_R |= 0x10;    			// 5) outputs on PB5-0
	GPIO_PORTB_AFSEL_R &= ~0x10; 			// 6) regular function on PB5-0
	GPIO_PORTB_DEN_R |= 0x10;    			// 7) enable digital on PB5-0// activate port F
}

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     	// 1) F clock
  delay = SYSCTL_RCGC2_R;           	// delay
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   	// 2) unlock PortF PF0
  GPIO_PORTF_CR_R = 0x1F;           	// allow changes to PF4&&PF0
  GPIO_PORTF_AMSEL_R &= 0x00;        	// 3) disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000;   	// 4) GPIO clear bit PCTL
  GPIO_PORTF_DIR_R |= 0x0E;          	// 5) PF4,PF0 input, PF3,PF2,PF1 output
  GPIO_PORTF_AFSEL_R |= 0x00;        	// 6) no alternate function
  GPIO_PORTF_PUR_R |= 0x11;          	// enable pullup resistors on PF4,PF0
  GPIO_PORTF_DEN_R |= 0x1F;          	// 7) enable digital pins PF4-PF0
}

int BtoD(int Bit_L, int Bit_H, unsigned int Binary[]){ //Function Converts Binary to Decimal
	int temp=0,i;
	for(i = Bit_L;i<=Bit_H;i++)
	temp += Binary[i] * (1<<(Bit_H-i));
	return temp;
}
