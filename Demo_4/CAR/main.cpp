
#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "stdio.h"

//test the communication between two bluetooth module

//Pin Usage
// Function ** Pin Name ** Board Pin Out
// TIM3 CH1 PWM ** PA6 ** LD2
// TIM3 CH2 PWM ** PB6   A8

// TIM3 CH1 PWM ** PB12   on board LED
#define TIM3_CH1_PWM_RCC_GPIO   RCC_APB2Periph_GPIOA
#define TIM3_CH1_PWM_GPIO GPIOA
#define TIM3_CH1_PWM_PIN  GPIO_Pin_6
/*
// TIM3 CH2 PWM ** PB12   A8
#define TIM3_CH2_PWM_RCC_GPIO RCC_APB2Periph_GPIOB
#define TIM3_CH2_PWM_GPIO GPIOB
#define TIM3_CH2_PWM_PIN GPIO_Pin_12
*/

//Function prototypes
void PWM_init(void);
void USART1_init(void);

//configure serial port
void USART1_init(void) {
	//USART1 TX A9 RX A10
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure );
	
	//USART1 STLINK USB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1,ENABLE);
}

void PWM_init(void) {
	RCC_APB2PeriphClockCmd(TIM3_CH1_PWM_RCC_GPIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	// Configure I/O for Tim3 Ch1 PWM pin
	GPIO_InitStructure.GPIO_Pin = TIM3_CH1_PWM_PIN | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(TIM3_CH1_PWM_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//Tim3 set up
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef timerInitStructure ;
	timerInitStructure.TIM_Prescaler = 720-1; //1/(72Mhz/720)=0.01ms
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 5000-1;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3 , &timerInitStructure);
	TIM_Cmd(TIM3 , ENABLE);	
	
	//Enable Tim3 Ch1 PWM
	TIM_OCInitTypeDef outputChannelInit;
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 1-1;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &outputChannelInit);
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
	//TIM3 CH2
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 1-1;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM3, &outputChannelInit);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
		//TIM3 CH3
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 1-1;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC3Init(TIM3, &outputChannelInit);
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
		//TIM3 CH2
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 1-1;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM3, &outputChannelInit);
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);
}


//variable for input
char DataRX;

int main(void) {

	USART1_init();	
	PWM_init();

	while(1){
		
		//get input char
		while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
		DataRX = USART_ReceiveData(USART1) & 0xFF;
		//flag clears automatically
		//USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		
		if(DataRX == '1')
		{
		TIM_SetCompare1(TIM3,0);
		TIM_SetCompare2(TIM3,1400);
		TIM_SetCompare3(TIM3,1500);
		TIM_SetCompare4(TIM3,0);
		}
		else if(DataRX == '3')
		{
		TIM_SetCompare1(TIM3,1300);
		TIM_SetCompare2(TIM3,0);
		TIM_SetCompare3(TIM3,0);
		TIM_SetCompare4(TIM3,1500);
		}
		else if(DataRX == '2')
		{
		TIM_SetCompare1(TIM3,0);
		TIM_SetCompare2(TIM3,1500);
		TIM_SetCompare3(TIM3,0);
		TIM_SetCompare4(TIM3,1500);
		}
		else if(DataRX == '4')
		{
		TIM_SetCompare1(TIM3,1500);
		TIM_SetCompare2(TIM3,0);
		TIM_SetCompare3(TIM3,1500);
		TIM_SetCompare4(TIM3,0);
		}
		else if(DataRX == 'a')
			{
		TIM_SetCompare1(TIM3,0);
		TIM_SetCompare2(TIM3,0);
		TIM_SetCompare3(TIM3,0);
		TIM_SetCompare4(TIM3,0);
		}
	}//end of while(1)
}//end of main function
