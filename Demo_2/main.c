#include "stm32f10x.h"  
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
//Name: Chang Jiahan
//ID: 17083442D
void RCC_init(void);
unsigned short GPIO_define(int x);
unsigned short GPIO_SOURCE_define(int x);
unsigned short EXTI_LINE_define(int x);
void GPIOB_OUT_init(int x);
void GPIOB_AF_init(int x);
void GPIOA_OUT_init(int x);
void GPIOA_AF_init(int x);
void TIM1_init(void);
void TIM3_init(void);
void TIM4_init(void);
void NVIC_TIM1_init(void);
void NVIC_TIM4_init(void);
void TIM3_PWM_CH2_init(void);
void TIM3_PWM_CH3_init(void);
void SPI_init(void);

void readFloor(void);
void alter_speed(void);
void delay(int t);
void TURN_right(float root_speed, float left_rate, float right_rate);
void TURN_left(float root_speed, float left_rate, float right_rate);

char buffer[50] = {'\0'};
int count = 0;
int sensor = 0;
int previous = 0;

float SPEED_root =850; //850
float SPEED_L1 = 700; //700
float SPEED_L2 = 550; //550

int step = 250;
int LEFT_adjust = -100;

float rminus_rate = 0.1;
float rplus_rate = 0;

float lminus_rate_L1 = 2;
float lplus_rate_L1 = 3;

float lminus_rate_L2 = 4;
float lplus_rate_L2 = 2;

static bool stop = false;

int ss = 0;


int main(){
	RCC_init();//
	
	//TIM1, count the black line
	TIM1_init();//
	NVIC_TIM1_init();//
	
	//TIM4, read the floor, alter the speed 
	TIM4_init();//
	NVIC_TIM4_init();//
	
	
	//SPI
	GPIOB_AF_init(13);//
	GPIOB_OUT_init(15);//
	SPI_init();//
	
	//on board LED
	GPIOA_OUT_init(5);//
	
	//PWM
	TIM3_init();//
	GPIOA_AF_init(7);//
	GPIOB_AF_init(0);//
  TIM3_PWM_CH2_init();//
	TIM3_PWM_CH3_init();//

	while(1){}
}

void TIM1_UP_IRQHandler(void){
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
		if(sensor >240){
			if (ss ==0){
				GPIO_SetBits(GPIOA,GPIO_Pin_5);
				ss = 1;
			}
			else{ 
				if(ss==1){
					GPIO_ResetBits(GPIOA,GPIO_Pin_5);
					ss = 0;
				}
			}
			count++;
			if (count ==1){
				delay(250);
			}
			if (count ==3){
				delay(200);
			}
			if (count == 5){
				TIM_SetCompare2(TIM3,(0));  
				TIM_SetCompare3(TIM3,(1500));
				delay(600);
				SPEED_root = 630; //650
				SPEED_L1 = 480; //500
				SPEED_L2 = 430; //450
			}
			if (count == 6){
				TIM_SetCompare2(TIM3,(0));  
				TIM_SetCompare3(TIM3,(0));
				delay(200);
				TIM_SetCompare2(TIM3,(1500));  
				TIM_SetCompare3(TIM3,(0));
				delay(800);
				TIM_SetCompare2(TIM3,(0));  
				TIM_SetCompare3(TIM3,(1500));
				delay(2000);	
				
				SPEED_root = 750; 
				SPEED_L1 = 600; 
				SPEED_L2 = 550; 			
			}
		}
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);	
	}
}

void TIM4_IRQHandler(void){
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
		readFloor();
		if(stop == true){
			TIM_SetCompare2(TIM3,0);  
			TIM_SetCompare3(TIM3,0);
		}else{
			alter_speed();
			
		}
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	
	}
}


void alter_speed(){
	//turn left
	if(sensor >= 8 && sensor <=15){
		TURN_right(SPEED_L1,rplus_rate,rminus_rate);
	}
	else if(sensor <=7){
		TURN_right(SPEED_L2,rplus_rate,rminus_rate);
	}
	
	//turn right 
	else if(sensor == 240 || sensor == 208 || sensor == 176 || sensor == 144 
		      || sensor == 112 || sensor == 80 || sensor == 48 || sensor == 16){
						//240 = 1111; 208 = 1101; 176 = 1011; 144 = 1001; 112 = 0111; 80 = 0101; 48 = 0011; 16 = 0001;
		TURN_left(SPEED_L1,lminus_rate_L1,lplus_rate_L1);
	}
	else if(sensor == 224 || sensor == 160 || sensor == 96 || sensor == 32
		||sensor == 192 || sensor == 64||sensor == 128||sensor == 124 || sensor == 125 || sensor == 127){
			//224 = 1110; 160 = 1010; 96 = 0110; 32 = 0010; 192 = 1100; 64 =0100; 128 = 1000; 124 = 0111 1100
		TURN_left(SPEED_L2,lminus_rate_L2,lplus_rate_L2); //450 4 2
	}
	
	//straight line
	else{
		TIM_SetCompare2(TIM3,(SPEED_root-LEFT_adjust));  //1200
		TIM_SetCompare3(TIM3,(SPEED_root)); //1100
	}
}


void readFloor() {
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
	SPI_I2S_SendData(SPI2, 0);
	NVIC_EnableIRQ(SPI2_IRQn);
}

void SPI2_IRQHandler() {	
	char c = (char) SPI_I2S_ReceiveData(SPI2) & 0xff;
	previous = sensor;
	sensor = c;
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15) == 1) 
	{
	  GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	  SPI_I2S_SendData(SPI2, 0);
	} 
	else 
	{
	  NVIC_DisableIRQ(SPI2_IRQn);
	}
}

void delay(int t){ //1000 for 1s
	unsigned i,j,k;
	for(i = 0; i< 6500; i++){
		for(j = 0; j< t; j++){
			k++;
	}
 }
}


void TURN_right(float root_speed, float left_rate, float right_rate){
		TIM_SetCompare2(TIM3,(root_speed + left_rate*step-LEFT_adjust+900));  
		TIM_SetCompare3(TIM3,(root_speed - right_rate*step-500));
}
void TURN_left(float root_speed, float left_rate, float right_rate){
		TIM_SetCompare2(TIM3,(root_speed - left_rate*step - LEFT_adjust-700));  
		TIM_SetCompare3(TIM3,(root_speed + right_rate*step));
}




void RCC_init(){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //USART
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); //AFIO 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // GPIOA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //GPIOC
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_SPI2EN, ENABLE);//SPI
}

unsigned short GPIO_define(int x){
	switch(x){
		case 0:
			return GPIO_Pin_0;
		case 1:
			return GPIO_Pin_1;
		case 2:
			return GPIO_Pin_2;
		case 3:
			return GPIO_Pin_3;
		case 4:
			return GPIO_Pin_4;
		case 5:
			return GPIO_Pin_5;
		case 6:
			return GPIO_Pin_6;
		case 7:
			return GPIO_Pin_7;
		case 8:
			return GPIO_Pin_8;
		case 9:
			return GPIO_Pin_9;
		case 10:
			return GPIO_Pin_10;
		case 11:
			return GPIO_Pin_11;
		case 12:
			return GPIO_Pin_12;
		case 13:
			return GPIO_Pin_13;
		case 14:
			return GPIO_Pin_14;
		case 15:
			return GPIO_Pin_15;		
		default: 
			return 0;
	}	
}

unsigned short GPIO_SOURCE_define(int x){
	switch(x){
		case 0:
			return GPIO_PinSource0;
		case 1:
			return GPIO_PinSource1;
		case 2:
			return GPIO_PinSource2;
		case 3:
			return GPIO_PinSource3;
		case 4:
			return GPIO_PinSource4;
		case 5:
			return GPIO_PinSource5;
		case 6:
			return GPIO_PinSource6;
		case 7:
			return GPIO_PinSource7;
		case 8:
			return GPIO_PinSource8;
		case 9:
			return GPIO_PinSource9;
		case 10:
			return GPIO_PinSource10;
		case 11:
			return GPIO_PinSource11;
		case 12:
			return GPIO_PinSource12;
		case 13:
			return GPIO_PinSource13;
		case 14:
			return GPIO_PinSource14;
		case 15:
			return GPIO_PinSource15;		
		default: 
			return 0;
	}	
}

unsigned short EXTI_LINE_define(int x){
	switch(x){
		case 0:
			return EXTI_Line0;
		case 1:
			return EXTI_Line1;
		case 2:
			return EXTI_Line2;
		case 3:
			return EXTI_Line3;
		case 4:
			return EXTI_Line4;
		case 5:
			return EXTI_Line5;
		case 6:
			return EXTI_Line6;
		case 7:
			return EXTI_Line7;
		case 8:
			return EXTI_Line8;
		case 9:
			return EXTI_Line9;
		case 10:
			return EXTI_Line10;
		case 11:
			return EXTI_Line11;
		case 12:
			return EXTI_Line12;
		case 13:
			return EXTI_Line13;
		case 14:
			return EXTI_Line14;
		case 15:
			return EXTI_Line15;		
		default: 
			return 0;
	}	
}


//GPIO Init
void GPIOB_OUT_init(int x){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_define(x);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);		
}

void GPIOB_AF_init(int x){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_define(x);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void GPIOA_OUT_init(int x){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_define(x);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);		
}

void GPIOA_AF_init(int x){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_define(x);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}


//TIM Init
void TIM1_init(){
	TIM_TimeBaseInitTypeDef timerInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	timerInitStructure.TIM_Prescaler = 24000-1; //24/72000 = 1/300
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period =300-1; //.1s
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &timerInitStructure);
	TIM_Cmd(TIM1, ENABLE);
	
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	//NVIC_EnableIRQ(TIM1_UP_IRQn);
}

void TIM3_init(){
	TIM_TimeBaseInitTypeDef timerInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	timerInitStructure.TIM_Prescaler =   720-1; //1/(72Mhz/720)=0.01ms
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period =5000-1; //50ms
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &timerInitStructure);
	TIM_Cmd(TIM3, ENABLE);
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	//NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM4_init(){
	TIM_TimeBaseInitTypeDef timerInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	timerInitStructure.TIM_Prescaler = 2400 -1;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 15-1; // .1s
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &timerInitStructure);
	TIM_Cmd(TIM4, ENABLE);
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
}


//NVIC Init
void NVIC_TIM1_init(){
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_InitStruct.NVIC_IRQChannelCmd= ENABLE;
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_Init(&NVIC_InitStruct);
}

void NVIC_TIM4_init(){
	NVIC_InitTypeDef NVIC_InitStruct;
	
	NVIC_InitStruct.NVIC_IRQChannelCmd= ENABLE;
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_Init(&NVIC_InitStruct);
}


//PWM Init
void TIM3_PWM_CH2_init(){
	TIM_OCInitTypeDef outputChannelInit;
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 1-1;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM3, &outputChannelInit);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
}

void TIM3_PWM_CH3_init(){
	TIM_OCInitTypeDef outputChannelInit;
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 1-1;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC3Init(TIM3, &outputChannelInit);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
}

//SPI Init
void SPI_init(){
    SPI_InitTypeDef   SPI_InitStructure;
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    // 36 MHz / 256 = 140.625 kHz
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI2, &SPI_InitStructure);
		// Enable the receive interrupt
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
    // Enable SPI2
    SPI_Cmd(SPI2, ENABLE);
}


