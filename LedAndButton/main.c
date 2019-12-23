/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include <stdio.h>
#include <string.h>

#include "stm32f30x.h"
#include "stm32f3_discovery.h"
#include "FreeRTOS.h"
#include "task.h"

#define TRUE 1
#define FALSE 0
#define PRESSED TRUE
#define NOT_PRESSED FALSE

//function prototypes
static void privateSetupHardware(void);
static void privateSetupGPIO(void);
static void privateSetupUART(void);
void printMsg(char *msg);

//task prototypes
void led_task_handler(void *params);
void button_handler(void *params);

//global space for variables
uint8_t button_status_flag = NOT_PRESSED;

int main(void)
{

	//1. Reset the RCC clock configuration to the default reset state
	// HSI on, PLL off, HSE off system clock = 8MHz, cpu clock = 8Mhz
	RCC_DeInit();

	//2. Update the SystemCoreClock variable
	SystemCoreClockUpdate();

	privateSetupHardware();

	//led task
	xTaskCreate(led_task_handler, "LED-TASK", configMINIMAL_STACK_SIZE, NULL, 1, NULL);


	//lets start the scheduler
	vTaskStartScheduler();

	for(;;);
}

void led_task_handler(void *params)
{
	while(1)
	{
		if(button_status_flag == PRESSED)
		{
			//turn on the led
			GPIO_WriteBit(GPIOE, GPIO_Pin_14, Bit_SET);
		}
		else
		{
			//turn off the led
			GPIO_WriteBit(GPIOE, GPIO_Pin_14, Bit_RESET);
		}
	}
}

void button_handler(void *params)
{
	button_status_flag ^= 1;
}

static void privateSetupHardware(void)
{

	//Setup LED and BUTTON
	privateSetupGPIO();
	// UART setup
	privateSetupUART();

}

static void privateSetupGPIO(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);

	//this function is board specific
	GPIO_InitTypeDef led_init , button_init;
	led_init.GPIO_Mode = GPIO_Mode_OUT;
	led_init.GPIO_OType = GPIO_OType_PP;
	led_init.GPIO_Pin = GPIO_Pin_14;
	led_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
	led_init.GPIO_Speed = GPIO_Speed_2MHz;



	GPIO_Init(GPIOE,&led_init);

	button_init.GPIO_Mode = GPIO_Mode_IN;
	button_init.GPIO_OType = GPIO_OType_PP;
	button_init.GPIO_Pin = GPIO_Pin_0;
	button_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
	button_init.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_Init(GPIOA,&button_init);

	//interrupt configuration for the button
	//1. System configuration for the exti line (SYSCFG settings)
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	//2. EXTI0 configuration 0, falling edge, interrupt mode
	EXTI_InitTypeDef exti_init;
	exti_init.EXTI_Line = EXTI_Line0;
	exti_init.EXTI_LineCmd = ENABLE;
	exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init.EXTI_Trigger = EXTI_Trigger_Falling;

	EXTI_Init(&exti_init);

	//3. Enable NVIC interrupts
	NVIC_SetPriority(EXTI0_IRQn,5);
	NVIC_EnableIRQ(EXTI0_IRQn);

}

void EXTI0_IRQHandler(void)
{

	traceISR_ENTER();
	//1. clear the interrupt pending bit
	EXTI_ClearITPendingBit(EXTI_Line0);
	button_handler(NULL);
	traceISR_EXIT();
}

static void privateSetupUART(void)
{
	GPIO_InitTypeDef gpio_uart_pins;
	USART_InitTypeDef uart1_init;

	//1. Enable the UART1 and GPIOC Peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE);

	//PC5 is UART1_Rx and PC4 is UART1_Tx
	//2. Alternate function config of MCU to act as UART1 TX and RX

	//reset the structure to zero
	memset(&gpio_uart_pins,0,sizeof(gpio_uart_pins));

	gpio_uart_pins.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	gpio_uart_pins.GPIO_Mode = GPIO_Mode_AF;
	gpio_uart_pins.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &gpio_uart_pins);

	//3. AF mode settings for the pin
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_7); //PC4 Tx
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_7); //PC5 Rx


	//4 reset the structure to zero
	memset(&uart1_init,0,sizeof(uart1_init));

	uart1_init.USART_BaudRate = 115200;
	uart1_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart1_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	uart1_init.USART_Parity = USART_Parity_No;
	uart1_init.USART_StopBits = USART_StopBits_1;
	uart1_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&uart1_init);

	//5. Enable the UART1 peripheral
	USART_Cmd(USART1,ENABLE);
}

void printMsg(char *msg)
{
	for(uint32_t i = 0; i < strlen(msg); i++)
	{
		//wait till the transmit buffer is ready
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);
		//send the data
		USART_SendData(USART1,msg[i]);
	}
}
