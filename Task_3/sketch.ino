#include "stm32c0xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Konstante
#define ADC_MAX_VALUE           4095.0f
#define APPS_DEVIATION_LIMIT    10.0f
#define IMPLAUSIBILITY_TIMEOUT  3000
#define TIMER_READING_MS        50
#define TIMER_DIAGNOSTIC_MS     500
#define TIMER_DEBOUNCE_MS       50
#define TIMER_LED_BLINK_MS      200

// Tipovi
typedef struct {
    uint16_t raw1;
    uint16_t raw2;
    float percent1;
    float percent2;
    float average;
    float deviation;
    float output;
} APPS_Data_t;

// Funkcije
void SystemClock_Config(void);
void GPIO_Init(void);
void ADC_Init(void);
void UART_Init(void);
void UART_Print(char* text);
uint16_t Read_APPS_Sensor(uint32_t channel);
void Update_APPS_Data(APPS_Data_t* data, uint8_t map_type);
uint8_t Check_APPS_Plausibility(float apps1, float apps2, uint32_t current_time);
void Print_Diagnostics(APPS_Data_t* data, uint8_t map, uint8_t implaus);
float LinearMap(float pedal_value);
float ProgressiveMap(float pedal_value);

// Globalne varijable
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;

uint8_t current_map = 0;
uint8_t button_last_state = 1;
uint32_t implausibility_start_time = 0;
uint8_t implausibility_active = 0;
uint8_t error_state = 0;

// Glavni
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    ADC_Init();
    HAL_ADCEx_Calibration_Start(&hadc1);
    UART_Init();
    
    APPS_Data_t apps_data;
    uint32_t timer_reading = 0;
    uint32_t timer_diagnostic = 0;
    uint32_t timer_led_blink = 0;
    uint32_t timer_debounce = 0;
    uint8_t led_state = 0;
    uint8_t button_debounced = 0;
    
    UART_Print("=== APPS System Started ===\r\n");
    
    while (1)
    {
        uint32_t now = HAL_GetTick();
        
        // Error (npr posaljes mi 6 7 meme)
        if (error_state)
        {
            if (now - timer_led_blink >= TIMER_LED_BLINK_MS)
            {
                timer_led_blink = now;
                led_state = !led_state;
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, led_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
            
            // Reset radi testiranja
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 0)
            {
                error_state = 0;
                implausibility_active = 0;
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                UART_Print("\r\n[RESET] System restarted\r\n");
            }
            continue;
        }
        
        // Periodicno citanje
        if (now - timer_reading >= TIMER_READING_MS)
        {
            timer_reading = now;
            
            Update_APPS_Data(&apps_data, current_map);
            
            if (Check_APPS_Plausibility(apps_data.percent1, apps_data.percent2, now))
            {
                error_state = 1;
                UART_Print("\r\n*** ERROR: APPS IMPLAUSIBILITY > 3000ms ***\r\n");
                UART_Print("*** SYSTEM HALTED ***\r\n\r\n");
            }
        }
        
        // Debounce i BOTUN
        if (now - timer_debounce >= TIMER_DEBOUNCE_MS)
        {
            timer_debounce = now;
            
            uint8_t button_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
            
            if (button_last_state == 1 && button_state == 0)
            {
                button_debounced = 1;
            }
            
            if (button_debounced && button_state == 1)
            {
                button_debounced = 0;
                current_map = !current_map;
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, current_map ? GPIO_PIN_SET : GPIO_PIN_RESET);
                
                char txt[80];
                sprintf(txt, "[MAP] Changed to: %s\r\n", current_map ? "PROGRESSIVE" : "LINEAR");
                UART_Print(txt);
            }
            
            button_last_state = button_state;
        }
        
        // DajGaNosiTika
        if (now - timer_diagnostic >= TIMER_DIAGNOSTIC_MS)
        {
            timer_diagnostic = now;
            Update_APPS_Data(&apps_data, current_map);
            Print_Diagnostics(&apps_data, current_map, implausibility_active);
        }
    }
}

// Funkcije

uint16_t Read_APPS_Sensor(uint32_t channel)
{
    ADC_ChannelConfTypeDef adc_channel;
    adc_channel.Channel = channel;
    adc_channel.Rank = ADC_REGULAR_RANK_1;
    adc_channel.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    HAL_ADC_ConfigChannel(&hadc1, &adc_channel);
    
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint16_t value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    return value;
}

void Update_APPS_Data(APPS_Data_t* data, uint8_t map_type)
{
    data->raw1 = Read_APPS_Sensor(ADC_CHANNEL_0);
    data->raw2 = Read_APPS_Sensor(ADC_CHANNEL_1);
    
    data->percent1 = (data->raw1 / ADC_MAX_VALUE) * 100.0f;
    data->percent2 = (data->raw2 / ADC_MAX_VALUE) * 100.0f;
    
    data->average = (data->percent1 + data->percent2) / 2.0f;
    data->deviation = fabs(data->percent1 - data->percent2);
    
    data->output = (map_type == 0) ? LinearMap(data->average) : ProgressiveMap(data->average);
}

uint8_t Check_APPS_Plausibility(float apps1, float apps2, uint32_t current_time)
{
    float deviation = fabs(apps1 - apps2);
    
    if (deviation > APPS_DEVIATION_LIMIT)
    {
        if (!implausibility_active)
        {
            implausibility_active = 1;
            implausibility_start_time = current_time;
            UART_Print("[WARN] APPS Implausibility detected!\r\n");
        }
        else if (current_time - implausibility_start_time >= IMPLAUSIBILITY_TIMEOUT)
        {
            return 1;  // Kad je ERROR
        }
    }
    else
    {
        if (implausibility_active)
        {
            UART_Print("[INFO] APPS back to plausible state\r\n");
        }
        implausibility_active = 0;
    }
    
    return 0;  // Kad je OK
}

void Print_Diagnostics(APPS_Data_t* data, uint8_t map, uint8_t implaus)
{
    int apps1_int = (int)data->percent1;
    int apps1_dec = (int)((data->percent1 - apps1_int) * 10);
    int apps2_int = (int)data->percent2;
    int apps2_dec = (int)((data->percent2 - apps2_int) * 10);
    int avg_int = (int)data->average;
    int avg_dec = (int)((data->average - avg_int) * 10);
    int dev_int = (int)data->deviation;
    int dev_dec = (int)((data->deviation - dev_int) * 10);
    int out_int = (int)data->output;
    int out_dec = (int)((data->output - out_int) * 10);
    
    char txt[250];
    sprintf(txt, "[DIAG] RAW1=%d RAW2=%d | APPS1=%d.%d%% APPS2=%d.%d%% AVG=%d.%d%% DEV=%d.%d%% OUT=%d.%d%% MAP=%s %s\r\n",
            data->raw1, data->raw2, apps1_int, apps1_dec, apps2_int, apps2_dec,
            avg_int, avg_dec, dev_int, dev_dec, out_int, out_dec,
            map ? "PROG" : "LIN", implaus ? "IMPLAUS!" : "OK");
    UART_Print(txt);
}

float LinearMap(float pedal_value)
{
    return pedal_value;
}

float ProgressiveMap(float pedal_value)
{
    float normalized = pedal_value / 100.0f;
    return normalized * normalized * 100.0f;
}

// Inicijalizacija
void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef gpio;
    
    // (PA5)
    gpio.Pin = GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    
    // (PC13)
    gpio.Pin = GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &gpio);
    
    // (PA0, PA1)
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);
}


void ADC_Init(void)
{
    __HAL_RCC_ADC_CLK_ENABLE();
    
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
    hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
    hadc1.Init.OversamplingMode = DISABLE;
    hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    
    HAL_ADC_Init(&hadc1);
}

void UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // (PA2=TX, PA3=RX)
    GPIO_InitTypeDef gpio;
    gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    HAL_UART_Init(&huart2);
}

void UART_Print(char* text)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)text, strlen(text), 100);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSIDiv = RCC_HSI_DIV1;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    HAL_RCC_OscConfig(&osc);

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1);
}
