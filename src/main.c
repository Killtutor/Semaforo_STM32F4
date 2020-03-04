/******************************************************************************/
/********************       STM32F407VG DISCOVERY   ***************************/
/********************               AN              ***************************/
/*****************************************************************************

    El presente codigo es para cumplir con el proyecto propuesto en clase de 
    un semaforo con posibilidad de pedir paso peatonal.

    Reglas a seguir: 
    -comentado del codigo estricto y necesario para este proyecto colaborativo
    -Los objetos y variables a definir se deben escribir con la primera letra en minuscula y las que definan espacios en mayusculas, sin _ ni -
    Ejmplo de variable a definir.. holaSoyUnaVariable o tambien epaSoyUnObjeto


*******************************************************************************/


/*********************************************************************************************/
/**************** Sección Inicialización Variables y definiciones  ***************************/
/***********  Metan todas las variables aqui, indiquen pa q las usan ************************/
/*********************************************************************************************/

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_rcc.h"
#include "UARTConfig.c"


static TIM_HandleTypeDef timerInstance = { 
    .Instance = TIM2
};

int t2Counter = 0;
int contadorRojo = 10;
int contadorVerde = 10;
int contadorAmarillo=4;
int contadorSema = 0;

 

/*******************************************************************************/

/*********************************************************************************************/
/**************** Sección declaracion de funciones a usar  ***************************/
/***********  Metan todas las funciones aqui, indiquen pa q las usan ************************/
/*********************************************************************************************/

void SysTick_Handler(void)    //Esto me mantiene el micro corriendo al reiniciar o desconectar
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

//Función que inicializa el/los timers
void initializeTimers(){
  
  
  HAL_TIM_Base_MspInit(&timerInstance);
  __TIM2_CLK_ENABLE();                                  //Habilito el timer2
  timerInstance.Init.Prescaler=15999;                     //Prescaler de 16000 
  timerInstance.Init.CounterMode = TIM_COUNTERMODE_UP;    //Cuento para arriba
  timerInstance.Init.Period = 9999;                       //Periodo de 1 segundo
  timerInstance.Init.ClockDivision = 0U;
  
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);                  //Alta prioridad para la interrupcion el Tim2
  HAL_NVIC_EnableIRQ(TIM2_IRQn);                           //Hailito la interrupcion del tim2
  __HAL_TIM_CLEAR_FLAG(&timerInstance, TIM_SR_UIF);         //Limpio Bandera de actualizacion de interrupcion 
  if(HAL_TIM_Base_Init(&timerInstance) == HAL_OK)           //Si se inicializo todo bien sigo
  {
    /* Start the TIM time Base generation in interrupt mode */
    // HAL_TIM_Base_Start_IT(&timerInstance);
    __HAL_TIM_ENABLE_IT(&timerInstance, TIM_IT_UPDATE);
      
  /* Enable the Peripheral */
      __HAL_TIM_ENABLE(&timerInstance);
      
  }
  
}
  
//Función que inicializa los Leds
void initializeLeds(){
  BSP_LED_Init(LED3);   //Led Rojo
  BSP_LED_Init(LED5);   //Led Amarillo
  BSP_LED_Init(LED4);   //Segundero
  BSP_LED_Init(LED6);   //Led Verde
}

void reinicioVerde(){
  contadorSema=1;
  printf("Enciendo Verde");
  BSP_LED_Off(LED3);
  BSP_LED_On(LED6);
}

//Funcion que maneja las interrupciones del Tim2
void TIM2_IRQHandler(){
  if (__HAL_TIM_GET_FLAG(&timerInstance, TIM_FLAG_UPDATE) != RESET){            //Porsiacaso entre aqui por error y fue otra interrupcion (Otro Timer por ejemplo)
    if (__HAL_TIM_GET_ITSTATUS(&timerInstance, TIM_IT_UPDATE) != RESET){        //Confirmo interrupcion de este timer
      __HAL_TIM_CLEAR_FLAG(&timerInstance, TIM_FLAG_UPDATE);                    //limpio la bandera para esperar a la otro interrupcion
      //t2Counter= t2Counter + 1;
      //if (t2Counter==200000){
        BSP_LED_Toggle(LED4);                                                   //Segundero
        //t2Counter = 0;    
        contadorSema = contadorSema+1;                                          //Llevara la cuenta de mi timer
        
        if((contadorSema>0) && (contadorSema < contadorVerde)){                 //Control del Verde
          printf("Enciendo Verde");
          BSP_LED_Off(LED3);
          BSP_LED_On(LED6);
        }else if (((contadorSema>contadorVerde) && (contadorSema < (contadorVerde+contadorAmarillo)))){   //Control del amarillo
          printf("Enciendo Amarillo");
          BSP_LED_Off(LED6);
          BSP_LED_On(LED5);
        }else if (((contadorSema>(contadorVerde+contadorAmarillo)) && (contadorSema < (contadorVerde+contadorAmarillo+contadorRojo)))){ // Control del rojo
          printf("Enciendo Rojo");
          BSP_LED_Off(LED5);
          BSP_LED_On(LED3);
        }else if ((contadorSema > (contadorVerde+contadorAmarillo+contadorRojo))){      //REinicio al verde.
          reinicioVerde();
        }
      //}
    }
  }
}

//Inicializa el boton del usuario
void initBoton (){
  BSP_PB_Init(BUTTON_KEY,BUTTON_MODE_EXTI); //Inicializo el boton azul de la tarjeta para ser mi boton del semaforo
}

//Manejador de interrupcion del botón
void BUTTON_IRQHandler(){
  if (__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_0) != RESET) { //Verifico si la interrupcion viene de la linea 0 (A0, B0, C0, D0)
    __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);                //Limpio la bandera de interrupcion de la linea 0 para que no vuelva a entrar aesta interrupción en otoro ciclo maquina (OJO esto porq no tengo mas nada interrumpiendo la linea 0 sino no puedo hacer esto.. dejo que el micro se encargue de esto, limpiando los it pending bits (IT))
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);                  //Limpio los bits de interrupcion de la linea 0
      if (BSP_PB_GetState(BUTTON_KEY)==1){                //chequeo si fue el boton 0 el que se presiono
        for (size_t i = 0; i < 80000; i++)                //Espero medio segundo para verificar de nuevo
        {
          __NOP;
        }
        if (BSP_PB_GetState(BUTTON_KEY)==1){              //Ahora si confirmo que se presiono el boton
          reinicioVerde();                                //Reinicio el Verde porque si.
        }
      }
    }
  }
}

int main (){

  HAL_Init();
  initializeLeds();
  initializeTimers();
  initBoton();
  
  while (1){
  }
}
