//Todo lo que necesito para hacer que esta libreria funcione esta aqui:
#include "stm32f4_discovery.h"
#include <stm32f4xx_hal.h>
#include <stm32_hal_legacy.h>

//Aqui las declaraciones de mis funciones de libreria
static void UARTConfig();
static UART_HandleTypeDef s_UARTHandle;



//Y finalmente las funciones como tal perse.
static void UARTConfig(){
__HAL_RCC_GPIOA_CLK_ENABLE();
__USART2_CLK_ENABLE();

GPIO_InitTypeDef GPIO_UART_Struct;
GPIO_UART_Struct.Pin = GPIO_PIN_2;
GPIO_UART_Struct.Mode = GPIO_MODE_AF_PP;
GPIO_UART_Struct.Pull = GPIO_NOPULL;
GPIO_UART_Struct.Alternate = GPIO_AF7_USART2;
GPIO_UART_Struct.Speed = GPIO_SPEED_FREQ_HIGH;
HAL_GPIO_Init(GPIOA, &GPIO_UART_Struct);

GPIO_UART_Struct.Pin = GPIO_PIN_3;
GPIO_UART_Struct.Mode = GPIO_MODE_AF_OD;
HAL_GPIO_Init(GPIOA, &GPIO_UART_Struct);


s_UARTHandle.Instance = USART2;
s_UARTHandle.Init.BaudRate = 115200;
s_UARTHandle.Init.Mode = USART_MODE_TX_RX;
s_UARTHandle.Init.Parity = USART_PARITY_NONE;
s_UARTHandle.Init.StopBits = USART_STOPBITS_1;
s_UARTHandle.Init.WordLength = USART_WORDLENGTH_8B;
s_UARTHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;

HAL_UART_Init(&s_UARTHandle);
}