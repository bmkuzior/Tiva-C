//#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "ADCSWTrigger.h"
#include "PLL.h"

// DEFINE


//bit specefic addressing
// swtiches 1 and 2 on PF4 and PF0 respectively
#define Switch_1         				(*((volatile unsigned long *)0x40025040))
#define Switch_2         				(*((volatile unsigned long *)0x40025004))	

// LEDs RED, BLUE and GREEN on PF1, PF2 and PF3 respectively
#define RED_LED         				(*((volatile unsigned long *)0x40025008))
#define BLUE_LED         				(*((volatile unsigned long *)0x40025010))
#define GREEN_LED         			(*((volatile unsigned long *)0x40025020))
	

//2. Declaration Section
//GLOBAL VARIABLES
unsigned long SW1,SW2;  // input from PF4,PF0
unsigned long In; 
unsigned long Out;      // outputs to PF3,PF2,PF1 (multicolor LED)
unsigned long Led;      // outputs to PF3,PF2,PF1 (multicolor LED)

volatile unsigned long ADCvalue;
volatile float result;

//*******
// initialization function
///****
void PortF_Init(void); 
void ADC0_InitSWTriggerSeq3_Ch1(void);
unsigned long ADC0_InSeq3(void);
///**



int main(void)
{
	PLL_Init();   
  ADC0_InitSWTriggerSeq3_Ch1();         // ADC initialization PE2/AIN1
	PortF_Init();  // PortF initialization
	
	while(1)
	{
  
    ADCvalue = ADC0_InSeq3();
		
		if (ADCvalue < 1760){
			RED_LED = 0x02;
			BLUE_LED = 0x00;
			GREEN_LED = 0x00;
		}
		
		else if (ADCvalue > 1760 && ADCvalue < 3112){
			RED_LED = 0x00;
			BLUE_LED = 0x04;
			GREEN_LED = 0x00;
		}
		else{
			RED_LED = 0x00;
			BLUE_LED = 0x00;
			GREEN_LED = 0x08;
		}
	}
}


void PortF_Init(void)
{
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0

}
