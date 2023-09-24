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
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "FreeRTOS.h"
#include "task.h"
#include "LoRa.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "liquidcrystal_i2c.h"
#include "DHT22.h"
#include "NMEA.h"
#include "uartRingBuffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define DHT22_PORT GPIOB
#define DHT22_PIN GPIO_PIN_1
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
//--------Handle--------
xSemaphoreHandle DHT_Sem;
xTaskHandle SDCARD_Task_Handler;
xTaskHandle LORA_Task_Handler;
xTaskHandle DHT_Task_Handler;
xTaskHandle LCD1602Task_Handler;
xTaskHandle GPSR_Task_Handler;

//--------LoRa Service--------
LoRa myLoRa;
uint16_t LoRa_status;
char message[100];
uint8_t TxBuffer[16];
uint8_t RxBuffer[16];
int rssi;
float snr;
char pckt_str[50];
char pckt_buffer[2];

//--------Environment--------
float Temperature = 0;
float Humidity = 0;
uint8_t Presence = 0;

//--------RTC--------
RTC_DateTypeDef gDate;
RTC_TimeTypeDef gTime;
char myTime[10];
char myDate[12];

//--------GPS--------
char GGA[100];
char RMC[100];

//--------BLE--------
xSemaphoreHandle BLE_Sem;
char message2[50];
uint8_t rxData = 0;
uint16_t pulse = 1000;//3999
struct Message
{
	int data;
};
struct Message *data1;
QueueHandle_t xQueue1;

GPSSTRUCT gpsData;

char lcdBuffer[50];
int VCCTimeout;
int flagGGA, flagRMC;
float fix_latitude, fix_longitude;
float lat_afterPoint, lon_afterPoint;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_UART5_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LED1_Task(void* pvParameter);
void LED3_Task(void* pvParameter);
void SDCARD_Task(void* pvParameter);
void LoRa_Task(void* pvParameter);
void RTC_Task(void* pvParameter);
void GPSR_Task(void* pvParameter);
void DHT22_Task(void* pvParameter);
void LCD1602_Task(void* pvParameter);

void float2Bytes(float val,uint8_t* bytes_array){
  // Create union of shared memory space
  union {
    float float_variable;
    uint8_t temp_array[4];
  } u;
  // Overwrite bytes of union with float variable
  u.float_variable = val;
  // Assign bytes to input array
  memcpy(bytes_array, u.temp_array, 4);
}

void LED1_Task(void* pvParameter)
{
	while(1)
	{
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);	//Green LED High
		vTaskDelay(500);
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);	//Green LED Low
		vTaskDelay(500);
	}
}

void LED3_Task(void* pvParameter)
{
	while(1)
	{
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);	//Red LED High
		vTaskDelay(1000);
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);	//Red LED Low
		vTaskDelay(1000);
	}
}

void LoRa_Task(void* pvParameter)
{
	myLoRa = newLoRa();

	myLoRa.CS_port         = NSS_GPIO_Port;
	myLoRa.CS_pin          = NSS_Pin;
	myLoRa.reset_port      = RST_GPIO_Port;
	myLoRa.reset_pin       = RST_Pin;
	myLoRa.DIO0_port       = DIO0_GPIO_Port;
	myLoRa.DIO0_pin        = DIO0_Pin;
	myLoRa.hSPIx           = &hspi1;

	myLoRa.frequency             = 920;             // default = 433 MHz
	myLoRa.spredingFactor        = SF_12;           // default = SF_7
	myLoRa.bandWidth             = BW_250KHz;       // default = BW_125KHz
	myLoRa.crcRate               = CR_4_8;          // default = CR_4_5
	myLoRa.power                 = POWER_20db;      // default = 20db
	myLoRa.overCurrentProtection = 150;             // default = 100 mA

	uint16_t LoRa_status = LoRa_init(&myLoRa);
	sprintf(message, "LoRa initialization ..");
	HAL_UART_Transmit(&huart4, (uint8_t *)message, strlen(message), 0xffff);
	memset(message, NULL, strlen(message));
	if(LoRa_status != LORA_OK){
		sprintf(message, " fail ! \r\n");
		HAL_UART_Transmit(&huart4, (uint8_t *)message, strlen(message), 0xffff);
		memset(message, NULL, strlen(message));
		while(1);
	}
	else if(LoRa_status == LORA_OK)
	{
		sprintf(message, " success !\r\n");
		HAL_UART_Transmit(&huart4, (uint8_t *)message, strlen(message), 0xffff);
		memset(message, NULL, strlen(message));
	}

	LoRa_startReceiving(&myLoRa);

	while(1)
	{
		uint8_t sum = 0;
		memset(pckt_str, NULL, strlen(pckt_str));
		for(int i = 0;i<15; i++)
		{
			uint8_t rand_num = rand();
			TxBuffer[i] = rand_num;
			if (TxBuffer[i] < 16) strcat(pckt_str, "0");
			strcat(pckt_str, itoa(TxBuffer[i], pckt_buffer, 16));
			strcat(pckt_str, " ");
			sum += TxBuffer[i];
			if (i == 14)
			{
				TxBuffer[i + 1] = sum;
				if (TxBuffer[i + 1] < 16) strcat(pckt_str, "0");
				strcat(pckt_str, itoa(TxBuffer[i + 1], pckt_buffer, 16));
			}
		}

		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);	//PC13 High
		LoRa_transmit(&myLoRa, TxBuffer, 16, 0xffff);
		sprintf(message, "Packet size: %d ,Send packet: %s \r\n", sizeof(TxBuffer)/sizeof(TxBuffer[0]), pckt_str);
		HAL_UART_Transmit(&huart4, (uint8_t *)message, strlen(message), 0xffff);
		memset(message, NULL, strlen(message));
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);	//PC13 Low
		vTaskDelay(5000);// Delay
	}
}

void GPSR_Task(void* pvParameter){
	while(1){

		if (Wait_for("GGA") == 1){

			VCCTimeout = 5000; // Reset the VCC Timeout indicating the GGA is being received

			Copy_upto("*", GGA);
			if(decodeGGA(GGA, &gpsData.ggastruct) == 0) flagGGA = 2; // 2 indicates the data is valid
			else flagGGA = 1;
		}


		if (Wait_for("RMC") == 1){

			VCCTimeout = 5000; // Reset the VCC Timeout indicating the RMC is being received

			Copy_upto("*", RMC);
			if(decodeRMC(RMC, &gpsData.rmcstruct) == 0) flagRMC = 2; // 2 indicates the data is valid
			else flagRMC = 1;
		}


		if ((flagGGA = 2) | (flagRMC = 2)){
			// print the time format
			sprintf(lcdBuffer, "%02d:%02d:%02d, %02d-%02d-%04d\r\n", gpsData.ggastruct.tim.hour, \
				  gpsData.ggastruct.tim.min, gpsData.ggastruct.tim.sec, gpsData.rmcstruct.date.Mon, \
				  gpsData.rmcstruct.date.Day, 2000 + gpsData.rmcstruct.date.Yr);
			HAL_UART_Transmit(&huart2, (uint8_t *)lcdBuffer, strlen(lcdBuffer), 0xffff);
			memset(lcdBuffer, '\0', strlen(lcdBuffer));
			// Convert to google map format
			lat_afterPoint = gpsData.ggastruct.lcation.latitude - (int)(gpsData.ggastruct.lcation.latitude);
			fix_latitude = (int)(gpsData.ggastruct.lcation.latitude) + lat_afterPoint/60*100;
			lon_afterPoint = gpsData.ggastruct.lcation.longitude - (int)(gpsData.ggastruct.lcation.longitude);
			fix_longitude = (int)(gpsData.ggastruct.lcation.longitude) + lon_afterPoint/60*100;
			// print the location format
			sprintf(lcdBuffer, "LAT: %.4f%c, LON: %.4f%c\r\n", fix_latitude, \
				  gpsData.ggastruct.lcation.NS, fix_longitude, gpsData.ggastruct.lcation.EW);
			HAL_UART_Transmit(&huart2, (uint8_t *)lcdBuffer, strlen(lcdBuffer), 0xffff);
			memset(lcdBuffer, '\0', strlen(lcdBuffer));
		}
		else if((flagGGA = 1) | (flagRMC = 1)){
			HAL_UART_Transmit(&huart2, "NO FIX YET !!\r\n", 20, 0xffff);
		}

		if (VCCTimeout <= 0){
			VCCTimeout = 5000;  // Reset the timeout

			//reset flags
			flagGGA =flagRMC =0;

			// You are here means the VCC is less, or maybe there is some connection issue
			// Check the VCC, also you can try connecting to the external 5V
			HAL_UART_Transmit(&huart2, "VCC Issue, Check Connection\r\n", 20, 0xffff);
		}
		vTaskDelay(2000);
	}
}

void DHT22_Task(void* pvParameter){
	int index = 1;
	while(1){
		if(xSemaphoreTake(DHT_Sem, 4500) != pdTRUE){
			HAL_UART_Transmit(&huart2, (uint8_t*)"Unable to acquire semaphore\r\n", 35, 100);
		}
		else{
			if(DHT22_Get_Data(&Temperature, &Humidity) == 1){
				sprintf (message, "%d. Temp = %.2f C\t RH = %.2f%% \r\n",index++, Temperature, Humidity);
				HAL_UART_Transmit(&huart2, (uint8_t *)message, strlen(message), 0xffff);
				memset(message, NULL, strlen(message));
			}
		}
	}
}

void LCD1602_Task(void* pvParameter){
	HD44780_Init(2);
	HD44780_Clear();
	while(1){
		HD44780_SetCursor(0,0);
		sprintf(message, "TEMP: %.2f C", Temperature);
		HD44780_PrintStr(message);
		memset(message, NULL, strlen(message));
		HD44780_SetCursor(0,1);
		sprintf(message, "RH: %.2f%%", Humidity);
		HD44780_PrintStr(message);
		memset(message, NULL, strlen(message));
//		for(int x=0; x<8; x=x++)
//		{
//		HD44780_ScrollDisplayLeft();  //HD44780_ScrollDisplayRight();
//		HAL_Delay(300);
//		}
		HAL_UART_Transmit(&huart2, "LCD is very healthy !!!\r\n", 20, 0xffff);
		vTaskDelay(2000);
		HD44780_Clear();
		for(int i = 0; i < 10;i++){
			get_time();
			HD44780_SetCursor(0,0);
			HD44780_PrintStr(myTime);
			HD44780_SetCursor(0,1);
			HD44780_PrintStr(myDate);
			vTaskDelay(500);
		}
	}
}

void SDCARD_Task(void* pvParameter){
	Mount_SD("/");
	Format_SD();
	Create_File("GPSR.TXT");
	Create_File("TEMP.TXT");
	Unmount_SD("/");

	int index = 1;
	while (1){
		char *buffer = pvPortMalloc(80*sizeof(char));
		sprintf (buffer, "LAT: %.6f, LON: %.6f\tTime: %02d:%02d:%02d\tDate: %02d-%02d-%02d\n",
							fix_latitude, fix_longitude,
							gpsData.ggastruct.tim.hour, gpsData.ggastruct.tim.min, gpsData.ggastruct.tim.sec,
							gpsData.rmcstruct.date.Mon, gpsData.rmcstruct.date.Day, gpsData.rmcstruct.date.Yr);
		Mount_SD("/");
		Update_File("GPSR.TXT", buffer);
		memset(buffer, NULL, strlen(buffer));
		sprintf (buffer, "%d. Temp = %.2f C\t RH = %.2f%% \n"
				"   Time: %02d:%02d:%02d \n"
				"   Date: %02d-%02d-%02d \n",
				index, Temperature, Humidity,
				gTime.Hours, gTime.Minutes, gTime.Seconds,
				gDate.Month, gDate.Date, 2000 + gDate.Year);
		Update_File("TEMP.TXT", buffer);
		vPortFree(buffer);
		Unmount_SD("/");

		index++;

		vTaskDelay(2000);
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
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_FATFS_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // UART4 HSE must be set to 8 MHz, which is twice the HSI 16 MHz.
  srand(time(0));
	xTaskCreate(LED1_Task, "LED1", 128, NULL, 1, NULL);
	xTaskCreate(LED3_Task, "LED3", 128, NULL, 1, NULL);
	xTaskCreate(LoRa_Task, "LoRa", 512, NULL, 2, NULL);

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
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x20404768;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SDCARD_CS_GPIO_Port, SDCARD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, NSS_Pin|RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DIO0_Pin */
  GPIO_InitStruct.Pin = DIO0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DIO0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDCARD_CS_Pin */
  GPIO_InitStruct.Pin = SDCARD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SDCARD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : NSS_Pin RST_Pin */
  GPIO_InitStruct.Pin = NSS_Pin|RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

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
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
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
