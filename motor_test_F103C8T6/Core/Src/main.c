/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char message[50];
uint8_t rxData = 0;
int dc_pulse = 0;
int servo_pulse = 0;
int pulse = 0;
int stepDelay = 1000;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM4_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LED_Task(void* pvParameter);
void Motion_Task(void* pvParameter);
void Platform_Rotation_Task(void* pvParameter);
void Trigger_Task(void* pvParameter);

void microDelay(uint16_t delay){
	__HAL_TIM_SET_COUNTER(&htim4, 0);
	while(__HAL_TIM_GET_COUNTER(&htim4) < delay);
}

void step(int steps, uint8_t direction, uint16_t delay){
	int x;
	if(direction == 0){
		HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_SET);
	}else
		HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);
	for(int i = 0;i < steps; i++){
		HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_SET);
		microDelay(delay);
		HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);
		microDelay(delay);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART2){
		rxData = huart->Instance->DR;

		HAL_UART_Receive_IT(huart, &rxData, 1);
		sprintf((uint8_t*)message, "UART2 Interrupt\r\n", rxData);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xffff);
		memset(message, 0, sizeof(message));
	}
}

void LED_Task(void* pvParameter){
	while(1){
		HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
		vTaskDelay(1000);
		HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
		vTaskDelay(1000);
	}
}

void Motion_Task(void* pvParameter){
	char* motion_message = pvPortMalloc(50*sizeof(char));
	sprintf((char*)motion_message,"Motion_Task initialize ..\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
	vPortFree(motion_message);
	while(1){
		if(rxData == 'U'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): Up !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);

			HAL_GPIO_WritePin(ENA1_GPIO_Port, ENA1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(ENA2_GPIO_Port, ENA2_Pin, GPIO_PIN_SET);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);

			rxData = 0;
		}
		else if(rxData == 'D'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): Down !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);

			HAL_GPIO_WritePin(ENA1_GPIO_Port, ENA1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(ENA2_GPIO_Port, ENA2_Pin, GPIO_PIN_SET);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pulse);

			rxData = 0;
		}
		else if(rxData == 'L'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): Left !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);

			HAL_GPIO_WritePin(ENA1_GPIO_Port, ENA1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(ENA2_GPIO_Port, ENA2_Pin, GPIO_PIN_SET);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pulse);

			rxData = 0;
		}
		else if(rxData == 'R'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): Right !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);

			HAL_GPIO_WritePin(ENA1_GPIO_Port, ENA1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(ENA2_GPIO_Port, ENA2_Pin, GPIO_PIN_SET);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pulse);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);

			rxData = 0;
		}
		else if(rxData == 'A'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): High speed !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);

			pulse = 3200;

			rxData = 0;
		}
		else if(rxData == 'Z'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): Low speed !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);

			pulse = 1500;

			rxData = 0;
		}
		else if(rxData == '0'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): 0 !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);

			HAL_GPIO_WritePin(ENA1_GPIO_Port, ENA1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(ENA2_GPIO_Port, ENA2_Pin, GPIO_PIN_RESET);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);

			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);

			rxData = 0;
		}
		else if(rxData == 'o'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			motion_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)motion_message, "Word(%c): o !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)motion_message, strlen(motion_message), 0xffff);
			vPortFree(motion_message);
			rxData = 0;
		}
		HAL_UART_Receive_IT(&huart2, &rxData, 1);
		vTaskDelay(100);
		HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
	}

}

void Platform_Rotation_Task(void* pvParameter){
	char* stepper_message = pvPortMalloc(50*sizeof(char));
	sprintf((char*)stepper_message, "Platform_Rotation_Task initialize ..\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)stepper_message, strlen(stepper_message), 0xffff);
	vPortFree(stepper_message);
	while(1){
		if(rxData == 'T'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			stepper_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)stepper_message, "Word(%c): Stepper Clockwise !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)stepper_message, strlen(stepper_message), 0xffff);
			vPortFree(stepper_message);
			step(200, 0, 500);

			rxData = 0;
		}
		else if(rxData == 'X'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			stepper_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)stepper_message, "Word(%c): Stepper Counterclockwise !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)stepper_message, strlen(stepper_message), 0xffff);
			vPortFree(stepper_message);
			step(200, 1, 500);

			rxData = 0;
		}
		HAL_UART_Receive_IT(&huart2, &rxData, 1);
		vTaskDelay(100);
	}
}

void Trigger_Task(void* pvParameter){
	char* servo_message = pvPortMalloc(50*sizeof(char));
	sprintf((char*)servo_message, "Trigger_Task initialize ..\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)servo_message, strlen(servo_message), 0xffff);
	vPortFree(servo_message);
	int PWM = 0;
	while(1){
		if(rxData == 'Q'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			servo_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)servo_message, "Word(%c): Servo Clockwise !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)servo_message, strlen(servo_message), 0xffff);
			vPortFree(servo_message);
//			PWM ++;
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 200);
			rxData = 0;
		}
		else if(rxData == 'W'){
			HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
			servo_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)servo_message, "Word(%c): Servo Counterclockwise !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)servo_message, strlen(servo_message), 0xffff);
			vPortFree(servo_message);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 400);
			rxData = 0;
		}
		else if(rxData == 'M'){
			servo_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)servo_message, "Word(%c): M !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)servo_message, strlen(servo_message), 0xffff);
			vPortFree(servo_message);

//			PWM = 0;
			rxData = 0;
		}
		else if(rxData == 'N'){
			servo_message = pvPortMalloc(50*sizeof(char));
			sprintf((char*)servo_message, "Word(%c): N !\r\n", rxData);
			HAL_UART_Transmit(&huart1, (uint8_t*)servo_message, strlen(servo_message), 0xffff);
			vPortFree(servo_message);
			rxData = 0;
		}
		HAL_UART_Receive_IT(&huart2, &rxData, 1);
		vTaskDelay(100);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	sprintf(message,(uint8_t*)"System initialize ..\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xffff);
	memset((uint8_t*)message, 0, sizeof(message));

	HAL_TIM_Base_Start(&htim4);

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, dc_pulse);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, dc_pulse);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, dc_pulse);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, dc_pulse);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, servo_pulse);

	HAL_GPIO_WritePin(ENA1_GPIO_Port, ENA1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ENA2_GPIO_Port, ENA2_Pin, GPIO_PIN_RESET);

	sprintf(message,"Motor control pins initialize ..\r\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xffff);
	memset(message, 0, sizeof(message));

	xTaskCreate(LED_Task, "LED", 50, NULL, 1, NULL);
	xTaskCreate(Motion_Task, "MOTION", 128, NULL, 1, NULL);
	xTaskCreate(Platform_Rotation_Task, "PLATFORM", 128, NULL, 1, NULL);
	xTaskCreate(Trigger_Task, "TRIGGER", 128, NULL, 1, NULL);
	vTaskStartScheduler();
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 3999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 143;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ENA1_Pin|ENA2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DIR_Pin|STEP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : GREEN_LED_Pin */
  GPIO_InitStruct.Pin = GREEN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GREEN_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ENA1_Pin ENA2_Pin */
  GPIO_InitStruct.Pin = ENA1_Pin|ENA2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DIR_Pin STEP_Pin */
  GPIO_InitStruct.Pin = DIR_Pin|STEP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
