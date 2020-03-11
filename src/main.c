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
//#include "UARTConfig.c"


static TIM_HandleTypeDef timerInstance = { 
    .Instance = TIM2
};

#define HCSR04_NUMBER			((float)0.0171821) // Para convertir de ms a cm
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

//Funcion que inicializa los pines del medidor ultrasonico
void initUltraPins(){
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef  gpio_init_structure;
  gpio_init_structure.Pin = GPIO_PIN_0;           //Pin 0 como entrada del echo
  gpio_init_structure.Mode = GPIO_MODE_INPUT;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOB,&gpio_init_structure);
  gpio_init_structure.Pin = GPIO_PIN_1;           //Pin 1 como salida para el hc-sr04
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOB,&gpio_init_structure);
}

//Función que inicializa el/los timers
void initializeTimers(){
  
  
  HAL_TIM_Base_MspInit(&timerInstance);
  __TIM2_CLK_ENABLE();                                  //Habilito el timer2
  timerInstance.Init.Prescaler=15999;                     //Prescaler de 16000 
  timerInstance.Init.CounterMode = TIM_COUNTERMODE_UP;    //Cuento para arriba
  timerInstance.Init.Period = 999;                       //Periodo de 1 segundo o 9999 Probar
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

void reinicioRojo(){
  contadorSema=15;
  printf("Enciendo Rojo");
  BSP_LED_Off(LED6);
  BSP_LED_Off(LED5);
  BSP_LED_On(LED3);
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
          reinicioRojo();                                //Reinicio el Verde porque si.
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
  initUltraPins();
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
  int8_t chequeando = 0;
  int16_t contador = 0;
  int tiempo = 0;
  int distancia = 0;
  while (1){

    switch (chequeando)                     //Para ir sabiendo en que paso estoy 
    {
    case 0:                                 //comenzandito voy a  mandar el pulso de 10us
      chequeando = 1;
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);//enciendo el pin del pulso
      break;
    case 1:
      contador = contador+1;
      if (contador==450){                               //Espero que haya pasado 10us
        contador = 0;
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET); //Apago el Pin del pulso
        chequeando = 2;
      }
      break;
    case 2:
      if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == 1){       //Espero a leer el echo cuando leo el echo acumulo los ciclos de maquina que pasa ON
        contador = contador+1;
      }else if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == 0 && contador>0){ //Cuando se va a OFF y si tengo algo contado
        tiempo = contador*0.000000022;                                  //Calculo el tiempo que estuvo en ON
        chequeando = 3;
        contador =0;
      }
      if (contador >454454){                                         //No me quedare esperando para siempre si ya pasaron 10ms empiezo de nuevo
        contador =0;
        chequeando = 0;
      }
      break;
    case 3:
      distancia = tiempo*0.034/2;                                     //Calculo la distancia
      if (6>distancia){                                              //si hay un peaton a 6cm del sensor reinicio rojo
        reinicioRojo();
      }
      contador = contador+1;                                  
      if (contador==454454){                                        //espero 10ms para iniciar de nuevo
        contador =0;
        chequeando = 0;
      }
      break;
    
    default:
      chequeando = 1;
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
      break;
    }
    

  }
}
