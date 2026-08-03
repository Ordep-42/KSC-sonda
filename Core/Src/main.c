/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aht.h"
#include "bmp280.h"
#include "qmc.h"
#include "ebyte.h"
#include "gnss.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    TX_IDLE,
    TX_WAKE,
    TX_SEND,
    TX_SLEEP
} TxSequence_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_SENSOR_TIMEOUT 100

// AHT10
#define AHT10_ADDRESS (0x38 << 1) // 0b1110000; Address[7-bit]Write/Read[1-bit]
#define AHT10_DRDY 1

// BMP280
#define BMP280_ADDRESS (0x76 << 1) // 0b1110110; Address[7-bit]Write/Read[1-bit]
#define BMP280_DRDY (1<<1)

// QMC5883L
#define QMC5883L_ADDRESS (0x0D << 1) // 0b0001101; Address[7-bit]Write/Read[1-bit]

// EBYTE Radio
#define GS_CHAN 0x46
#define GS_ADDR 0x1A2B
#define RADIO_TX_MS 1000
#define STATUS_BLINK_MS 50

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
AHT_Handle_t haht;
AHT_Data_t aht_data;

BMP280_Handle_t hbmp;
BMP280_Data_t bmp_data;

QMC5883L_Handle_t hqmc;
QMC5883L_Data_t mag_data;

EBYTE_Handle_t hradio;
TxSequence_t radio_seq = TX_IDLE;
char radio_msg[96];

uint8_t sensors_drdy = 0;
uint8_t status_led = 0;
uint32_t status_led_timestamp = 0;
GNSS_Handle_t hgnss;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Send printf to uart1
#ifdef DEBUG_UART
int _write(int fd, char* ptr, int len) {
  HAL_StatusTypeDef hstatus;

  if (fd == 1 || fd == 2) {
    hstatus = HAL_UART_Transmit(&DEBUG_UART, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    if (hstatus == HAL_OK)
      return len;
    else
      return -1;
  }
  return -1;
}
#endif

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t offset) {
    if (huart == hgnss.huart)
        GNSS_RxCallback(&hgnss, offset);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
 if(htim == &I2C_SENSOR_TIMER)
 {
	 sensors_drdy |= AHT10_DRDY;
	 sensors_drdy |= BMP280_DRDY;
	 HAL_TIM_Base_Stop_IT(&I2C_SENSOR_TIMER);
 }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == QMC_DRDY_Pin) {
        QMC5883L_OnDataReadyIRQ(&hqmc);
    }
    if (GPIO_Pin == LORA_AUX_Pin) {
    	EBYTE_AuxCallback(&hradio);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	EBYTE_TxCpltCallback(&hradio, huart);
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  GNSS_Init(&hgnss, &GNSS_UART);

  hradio = (EBYTE_Handle_t){
		  .huart = &RADIO_UART,
		  .radio_timeout = 500,

		  .m0_gpio_port = LORA_MODE_GPIO_Port,
		  .m0_pin = LORA_MODE_Pin,
		  .m1_gpio_port = LORA_MODE_GPIO_Port,
		  .m1_pin = LORA_MODE_Pin,

		  .aux_gpio_port = LORA_AUX_GPIO_Port,
		  .aux_pin = LORA_AUX_Pin
  };

  AHT_Init(&haht, &SENSOR_I2C, AHT10_ADDRESS, I2C_SENSOR_TIMEOUT);
  AHT_TriggerMeasurement(&haht);
  
  BMP280_Init(&hbmp, &hi2c1, BMP280_ADDRESS, I2C_SENSOR_TIMEOUT);
  BMP280_Ctrl_t bmp_ctrl_cfg = {
      .osrs_t = BMP280_OSRS_X4,
      .osrs_p = BMP280_OSRS_X1,
      .mode   = BMP280_MODE_NORMAL
  };
  BMP280_Config_t bmp_config_cfg = {
      .standby = BMP280_STANDBY_62_5MS,
      .filter  = BMP280_FILTER_4,
      .spi3w_enable = 0
  };

  BMP280_SetMode(&hbmp, BMP280_CtrlEncode(&bmp_ctrl_cfg));
  BMP280_SetConfig(&hbmp, BMP280_ConfigEncode(&bmp_config_cfg));
  
	HAL_TIM_Base_Start_IT(&I2C_SENSOR_TIMER);

	QMC5883L_Init(&hqmc, &hi2c1, QMC5883L_ADDRESS, I2C_SENSOR_TIMEOUT);
	QMC5883L_Ctrl1_t qmc_ctrl1_cfg = {
	    .osr = QMC5883L_OSR_128,
	    .range = QMC5883L_RANGE_2G,
	    .odr = QMC5883L_ODR_10HZ,
	    .mode = QMC5883L_CONTINUOUS
	};
	QMC5883L_Ctrl2_t qmc_ctrl2_cfg = {
		.interrupt = QMC5883L_INT_ENABLE,
		.roll_pointer = QMC5883L_ROL_ENABLE
	};

	QMC5883L_SetCtrl1(&hqmc, QMC5883L_Ctrl1Encode(&qmc_ctrl1_cfg));
	QMC5883L_SetCtrl2(&hqmc, QMC5883L_Ctrl2Encode(&qmc_ctrl2_cfg));
	hqmc.events |= QMC_EVT_DATA_READY;

	EBYTE_Init(&hradio);
	EBYTE_Sleep(&hradio);
	uint32_t last_radio_tx = 0;
	static uint32_t seq = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (hgnss.events & GNSS_EVT_LINE_READY) {
		  hgnss.events &= ~GNSS_EVT_LINE_READY;
		  GNSS_Process(&hgnss);
	  }

	  if (hgnss.events & GNSS_EVT_FIX_VALID) {
		  hgnss.events &= ~GNSS_EVT_FIX_VALID;

		  // GPS FIX VALID

	  }
	  if (sensors_drdy & AHT10_DRDY) {
		  if (AHT_ReadData(&haht, &aht_data) == HAL_OK) {
			  sensors_drdy &= ~AHT10_DRDY;
			  AHT_TriggerMeasurement(&haht);
			  HAL_TIM_Base_Start_IT(&htim4);
		  }
	  }
	  if (sensors_drdy & BMP280_DRDY) {
		  if (BMP280_ReadData(&hbmp, &bmp_data) == HAL_OK)
			sensors_drdy &= ~BMP280_DRDY;
	  }

	  if (hqmc.events & QMC_EVT_DATA_READY){
		  hqmc.events &= ~QMC_EVT_DATA_READY;
		  HAL_StatusTypeDef status = QMC5883L_ReadData(&hqmc, &mag_data);

		  if (status == HAL_OK) {
			  int16_t temp;
			  (void)QMC5883L_ReadTemp(&hqmc, &temp);

			  if (hqmc.events & QMC_EVT_OVERFLOW) {
				  //overflow!
				  hqmc.events &= ~QMC_EVT_OVERFLOW;
			  }
		  }
	  }

	  if ((HAL_GetTick() - last_radio_tx >= RADIO_TX_MS) && radio_seq == TX_IDLE) {
		  last_radio_tx += RADIO_TX_MS;
	      snprintf(radio_msg, sizeof(radio_msg), "SEQ:%lu,UPTIME:%lu,TA:%d.%02d,H:%u.%02u,TB:%ld.%02ld,P:%lu.%02lu;\r\n",
									 seq++, HAL_GetTick(),
									 aht_data.temp / 100, abs(aht_data.temp % 100),
									 aht_data.humi / 100, aht_data.humi % 100,
									 bmp_data.temp / 100, labs(bmp_data.temp % 100),
									 bmp_data.pres / 100, labs(bmp_data.pres % 100));

	      radio_seq = TX_WAKE;
	  }

	  switch (radio_seq) {
	  case TX_IDLE:
		  break;

	  case TX_WAKE:
		  if (hradio.state == EBYTE_SLEEP) {
			  EBYTE_WakeUp(&hradio);
		  } else if (hradio.state == EBYTE_IDLE) {
			  radio_seq = TX_SEND;
		  }
		  break;

	  case TX_SEND:
		  if (hradio.state == EBYTE_IDLE) {
			  if (EBYTE_Transmit(&hradio, GS_ADDR, GS_CHAN, (uint8_t *)radio_msg, strlen(radio_msg)) == EBYTE_OK) {
				  radio_seq = TX_SLEEP;
				  status_led = 1;
				  status_led_timestamp = HAL_GetTick();
				  HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET);
			  }
		  }
		  break;

	  case TX_SLEEP:
		  if (hradio.state == EBYTE_IDLE) EBYTE_Sleep(&hradio);
		  else if (hradio.state == EBYTE_SLEEP) radio_seq = TX_IDLE;
	  }

	  if(status_led && HAL_GetTick() - status_led_timestamp >= STATUS_BLINK_MS) {
	      status_led = 0;
	      HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
	  }
	  HAL_Delay(1);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
