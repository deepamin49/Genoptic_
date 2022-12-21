/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

#include "mlx90614.h"
#include <stdio.h>
#include <string.h>
#include "BH1750.h"
#include "sht3x.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define BH1750_COUNTER 5
#define MLX90614_COUNTER 5
#define SHT31_COUNTER 5

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* unit16_t is unassigned 16 bit integer variable to hold timer value ; bool datatype used to represent boolian functions like true, false*/
uint16_t SHT31_counter = SHT31_COUNTER;
bool SHT31_counter_timeout_flag = false;
uint16_t BH1750_counter = BH1750_COUNTER;
bool BH1750_counter_timeout_flag = false;
uint16_t MLX90614_counter = MLX90614_COUNTER;
bool  MLX90614_counter_timeout_flag = false;

/* we have assigned handle value to sht31d */
/* sht3x_handle_t is a user defined datatype*/
sht3x_handle_t sht31d;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

static void scan_I2C_bus(uint8_t bus);
static void printUART(char string[]);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

	     /*A hardware abstraction layer (HAL) is a programming or code layer that
	      allows for more broad communication between software and hardware in a system.
	      HAL function to call back timer; gets called whenever timer overflow occurs *//*
		 * Calls when Timer period is reached T = 0.1s
		 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
		if(--BH1750_counter == 0) {
			//BH1750 Handler
			BH1750_counter_timeout_flag = true;
			BH1750_counter = BH1750_COUNTER;
		}
		if(--SHT31_counter == 0) {
			//SHT31 Handler
			SHT31_counter_timeout_flag = true;
			SHT31_counter = SHT31_COUNTER;
		}
		if(--MLX90614_counter == 0) {
			//MLX90614 Handler
			MLX90614_counter_timeout_flag = true;
			MLX90614_counter = MLX90614_COUNTER;
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

	/* defining uart buffer of 128 buffer length and stored in 32 bit uart buffer */

	 char uart_buf[128] = {'\0'};
	 uint32_t uart_buf_len;

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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  float temp_obj1, temp_amb, light, sht31_temp, sht31_humidity;

  /* HAL timer interuppt start */

  HAL_TIM_Base_Start_IT(&htim3);

   scan_I2C_bus(1);

   MLX90614_Init(&hi2c1);
   BH1750_Init(&hi2c1);
   BH1750_SetMode(CONTINUOUS_HIGH_RES_MODE_2);
   sht31d.i2c_handle = &hi2c1;
   sht31d.device_address = (uint16_t)SHT3X_I2C_DEVICE_ADDRESS_ADDR_PIN_LOW;
   sht3x_init(&sht31d);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //	  timer_val = __HAL_TIM_GET_COUNTER(&htim3);
	  //	  HAL_Delay(2000);
	  //	  timer_val = __HAL_TIM_GET_COUNTER(&htim3) - timer_val;
	  //	  uart_buf_len = sprintf(uart_buf, "%u us\r\n", timer_val);
	  //	  HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, uart_buf_len, 100);

	  	  if(BH1750_counter_timeout_flag) {
	  		  //trigger BH1750 read

	  		  BH1750_counter_timeout_flag = 0;

	  		  BH1750_ReadLight(&light);

	  		  uart_buf_len = sprintf(uart_buf, "Light: %.2f\r\n", light);
	  		  HAL_UART_Transmit(&huart2, uart_buf, uart_buf_len, 100);
	  		  memset(uart_buf, 0, sizeof(uart_buf));
	  	  }

	  	  if(SHT31_counter_timeout_flag) {
	  		  //trigger BH1750 read

	  		  SHT31_counter_timeout_flag = 0;

	  		  sht3x_read_temperature_and_humidity(&sht31d, &(sht31_temp), &(sht31_humidity) );

	  		  uart_buf_len = sprintf(uart_buf, "Temperature: %0.2f and Humidity: %.2f\r\n", sht31_temp, sht31_humidity);
	  		  HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, uart_buf_len, 100);
	  		  memset(uart_buf, 0, sizeof(uart_buf));
	  		  // buffer memory reset
	  	  }

	  	  if(MLX90614_counter_timeout_flag) {
	  		  //trigger MLX90614 read

	  		  MLX90614_counter_timeout_flag = 0;

	  		  temp_obj1 = MLX90614_ReadTObj1();
	  		  HAL_Delay(5);
	  		  temp_amb = MLX90614_ReadTAmb();

	  		  uart_buf_len = sprintf(uart_buf, "IR Sensor: T obj1: %.2f, T amb: %.2f\r\n", temp_obj1, temp_amb);
	  		  HAL_UART_Transmit(&huart2, uart_buf, uart_buf_len, 100);
	  		  memset(uart_buf, 0, sizeof(uart_buf));
	  	  }

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 16800;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 5000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  huart2.Init.BaudRate = 115200;
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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

static void scan_I2C_bus(uint8_t bus) {
	HAL_StatusTypeDef result;
	char buf[20];

	for (int i = 0; i<128; i++)
	  {
		  result = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t) (i<<1), 3, 5);
		  if (result != HAL_OK)
		  {
			  sprintf(buf, "0x%-\r\n");
			  HAL_UART_Transmit(&huart2, buf, sizeof(buf), 100);
			  memset(buf, 0, sizeof(buf));
		  }
		  if (result == HAL_OK)
		  {
			  sprintf(buf, "0x%X\r\n", i);
			  HAL_UART_Transmit(&huart2, buf, sizeof(buf), 100);
			  memset(buf, 0, sizeof(buf));
		  }
	  }
}


/* USER CODE END 4 */

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
