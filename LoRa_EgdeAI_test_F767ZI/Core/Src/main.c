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
#include "fatfs_sd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LoRa.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "File_Handling_RTOS.h"
#include "DS1307.h"
#include "liquidcrystal_i2c.h"
#include "DHT22.h"
#include "NMEA.h"
#include "uartRingBuffer.h"
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

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart3;

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
RTC_time_t mytime;
RTC_date_t mydate;

//--------GPS--------
char GGA[100];
char RMC[100];
char lcdBuffer[50];
int VCCTimeout;
int flagGGA, flagRMC;
float fix_latitude, fix_longitude;
float lat_afterPoint, lon_afterPoint;

struct Message
{
	int data;
};
struct Message *data1;
QueueHandle_t xQueue1;

GPSSTRUCT gpsData;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM8_Init(void);
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
	myLoRa.bandWidth             = BW_125KHz;       // default = BW_125KHz
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
			HAL_UART_Transmit(&huart4, (uint8_t *)lcdBuffer, strlen(lcdBuffer), 0xffff);
			memset(lcdBuffer, '\0', strlen(lcdBuffer));
			// Convert to google map format
			lat_afterPoint = gpsData.ggastruct.lcation.latitude - (int)(gpsData.ggastruct.lcation.latitude);
			fix_latitude = (int)(gpsData.ggastruct.lcation.latitude) + lat_afterPoint/60*100;
			lon_afterPoint = gpsData.ggastruct.lcation.longitude - (int)(gpsData.ggastruct.lcation.longitude);
			fix_longitude = (int)(gpsData.ggastruct.lcation.longitude) + lon_afterPoint/60*100;
			// print the location format
			sprintf(lcdBuffer, "LAT: %.4f%c, LON: %.4f%c\r\n", fix_latitude, \
				  gpsData.ggastruct.lcation.NS, fix_longitude, gpsData.ggastruct.lcation.EW);
			HAL_UART_Transmit(&huart4, (uint8_t *)lcdBuffer, strlen(lcdBuffer), 0xffff);
			memset(lcdBuffer, '\0', strlen(lcdBuffer));
		}
		else if((flagGGA = 1) | (flagRMC = 1)){
			HAL_UART_Transmit(&huart4, "NO FIX YET !!\r\n", 20, 0xffff);
		}

		if (VCCTimeout <= 0){
			VCCTimeout = 5000;  // Reset the timeout

			//reset flags
			flagGGA = 0;
			flagRMC = 0;

			// You are here means the VCC is less, or maybe there is some connection issue
			// Check the VCC, also you can try connecting to the external 5V
			HAL_UART_Transmit(&huart4, "VCC Issue, Check Connection\r\n", 20, 0xffff);
		}
		vTaskDelay(2000);
	}
}

void DHT22_Task(void* pvParameter){
	int index = 1;
	vTaskDelay(800);
	HAL_UART_Transmit(&huart4, (uint8_t*)"DHT22 initialization ..\r\n", 35, 100);
	while(1){
		if(xSemaphoreTake(DHT_Sem, 2500) != pdTRUE){
			HAL_UART_Transmit(&huart4, (uint8_t*)"Unable to acquire semaphore\r\n", 35, 100);
		}
		else{
			if(DHT22_Get_Data(&Temperature, &Humidity) == 1){
				sprintf (message, "%d. Temp = %.2f C\t RH = %.2f%% \r\n",index++, Temperature, Humidity);
				HAL_UART_Transmit(&huart4, (uint8_t *)message, strlen(message), 0xffff);
				memset(message, NULL, strlen(message));
			}
		}
	}
}

void LCD_Task(void* pvParameter){
	vTaskDelay(2000);
	HD44780_Clear();
	char* LCD_message;
	LCD_message = pvPortMalloc(50*sizeof(char));
	sprintf(LCD_message, "LCD1602 initialization .. \r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*)LCD_message, strlen(LCD_message), 0xffff);
	vPortFree(LCD_message);
	while(1){
		HD44780_SetCursor(0, 0);
		LCD_message = pvPortMalloc(50*sizeof(char));
		sprintf(LCD_message, "%02d:%02d:%02d", mytime.hours, mytime.minutes, mytime.seconds);
		HD44780_PrintStr(LCD_message);
		vPortFree(LCD_message);
		HD44780_SetCursor(0, 1);
		LCD_message = pvPortMalloc(50*sizeof(char));
		sprintf(LCD_message, "%02d/%02d/%04d", mydate.month, mydate.date, mydate.year+2000);
		HD44780_PrintStr(LCD_message);
		vPortFree(LCD_message);
		vTaskDelay(800);
	}
}

void SDCARD_Task(void* pvParameter){
	char *buffer;
	Mount_SD("/");
	Format_SD();
	Create_File("GPSR.TXT");
	Create_File("TEMP.TXT");
	Check_SD_Space();
	Unmount_SD("/");

	int index = 1;
	while (1){
		Mount_SD("/");
		buffer = pvPortMalloc(50*sizeof(char));
		sprintf (buffer, "Hi, SD card ! %d\n", index);
		Update_File("GPSR.TXT", buffer);
		vPortFree(buffer);
		Unmount_SD("/");

		index++;

		vTaskDelay(10000);
	}
}

void RTC_Task(void* pvParameter){
	char* RTC_message = pvPortMalloc(50*sizeof(char));
	if(ds1307_init(&hi2c1)){
		sprintf(RTC_message, "DS1307 initialization .. fail !\r\n");
		HAL_UART_Transmit(&huart4, (uint8_t*)RTC_message, strlen(RTC_message), 0xffff);
		vPortFree(RTC_message);
		while(1);
	}
	RTC_message = pvPortMalloc(50*sizeof(char));
	sprintf(RTC_message, "DS1307 initialization .. success !\r\n");
	HAL_UART_Transmit(&huart4, (uint8_t*)RTC_message, strlen(RTC_message), 0xffff);
	vPortFree(RTC_message);
	mytime.seconds = 0;
	mytime.minutes = 0;
	mytime.hours = 12;
	mytime.time_format = TIME_FORMAT_24HRS;
	mydate.date = 30;
	mydate.day = SATURDAY;
	mydate.month = 9;
	mydate.year = 23;
	ds1307_set_current_time(&mytime);
	ds1307_set_current_date(&mydate);

	while(1){
		ds1307_get_current_time(&mytime);
		RTC_message = pvPortMalloc(50*sizeof(char));
		sprintf(RTC_message, "Time: %02d:%02d:%02d\r\n", mytime.hours, mytime.minutes, mytime.seconds);
		HAL_UART_Transmit(&huart4, (uint8_t*)RTC_message, strlen(RTC_message), 0xffff);
		vPortFree(RTC_message);

		ds1307_get_current_date(&mydate);
		RTC_message = pvPortMalloc(50*sizeof(char));
		sprintf(RTC_message, "Date: %02d/%02d/%04d\r\n", mydate.month, mydate.date, (mydate.year +2000));
		HAL_UART_Transmit(&huart4, (uint8_t*)RTC_message, strlen(RTC_message), 0xffff);
		vPortFree(RTC_message);
		vTaskDelay(950);
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
  MX_FATFS_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  // UART4 HSE must be set to 8 MHz, which is twice the HSI 16 MHz.
  srand(time(0));
  Ringbuf_init();
  HD44780_Init(2);
  DHT_Sem = xSemaphoreCreateBinary();
  sprintf(message, "System initialization ... \r\n");
  HAL_UART_Transmit(&huart4, (uint8_t*)message, strlen(message), 0xffff);

//	xTaskCreate(LED1_Task, "LED1", 128, NULL, 1, NULL);
//	xTaskCreate(LED3_Task, "LED3", 128, NULL, 1, NULL);

	xTaskCreate(LoRa_Task, "LoRa", 512, NULL, 3, NULL);
	xTaskCreate(RTC_Task, "RTC", 256, NULL, 2, NULL);
	xTaskCreate(SDCARD_Task, "SDCARD", 512, NULL, 2, NULL);
	xTaskCreate(GPSR_Task, "GPSR", 512, NULL, 1, NULL);
	xTaskCreate(LCD_Task, "LCD", 256, NULL, 1, NULL);
	xTaskCreate(DHT22_Task, "DHT22", 256, NULL, 4, NULL);
	/*must be put after the HD44780_Init and heap size = 256.
	Otherwise, it doesn't work, nor do the other task with
	priority lower than it. */
	HAL_TIM_Base_Start(&htim8); // us delay timer
	HAL_TIM_Base_Start_IT(&htim1); // periodic delay timer

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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 54000-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 8000-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM8 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM8_Init(void)
{

  /* USER CODE BEGIN TIM8_Init 0 */

  /* USER CODE END TIM8_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM8_Init 1 */

  /* USER CODE END TIM8_Init 1 */
  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 216-1;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 65535;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM8_Init 2 */

  /* USER CODE END TIM8_Init 2 */

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
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, NSS_Pin|RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SDCARD_CS_GPIO_Port, SDCARD_CS_Pin, GPIO_PIN_SET);

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

  /*Configure GPIO pin : DHT_Pin */
  GPIO_InitStruct.Pin = DHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_GPIO_Port, &GPIO_InitStruct);

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

  /*Configure GPIO pin : SDCARD_CS_Pin */
  GPIO_InitStruct.Pin = SDCARD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SDCARD_CS_GPIO_Port, &GPIO_InitStruct);

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
	if (htim->Instance == TIM1){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		xSemaphoreGiveFromISR(DHT_Sem, &xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
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
