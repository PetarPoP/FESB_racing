#include "stm32c0xx_hal.h"

void SystemClock_Config(void);
void GPIO_Init(void);
void ADC_Init(void);
void UART_Init(void);
void UART_Print(char* text);

ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;

typedef struct {
    uint8_t id;
    uint16_t adc;
    uint8_t led;
} Message;

Message message;
uint8_t new_message = 0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    ADC_Init();
    UART_Init();
    
    uint32_t timer_tx = 0;
    uint32_t timer_ack = 0;
    uint8_t led = 0;
    
    while (1)
    {
        uint32_t now = HAL_GetTick();
        
        if (now - timer_tx >= 500)
        {
            timer_tx = now;
            
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, 100);
            uint16_t value = HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);
            
            message.id = 1;
            message.adc = value;
            message.led = led;
            new_message = 1;
            
            char txt[80];
            sprintf(txt, "[TX] ID=%d ADC=%d LED=%d\r\n", message.id, message.adc, message.led);
            UART_Print(txt);
        }
        
        if (new_message == 1)
        {
            new_message = 0;
            
            if (message.adc > 2000)
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                led = 1;
                UART_Print("[RX] WARNING: High ADC!\r\n");
            }
            else
            {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                led = 0;
                
                char txt[80];
                sprintf(txt, "[RX] ID=%d ADC=%d LED=%d\r\n", message.id, message.adc, message.led);
                UART_Print(txt);
            }
        }
        
        if (now - timer_ack >= 1000)
        {
            timer_ack = now;
            UART_Print("[RX] ACK sent\r\n");
        }
    }
}

void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef gpio;
    gpio.Pin = GPIO_PIN_5;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

void ADC_Init(void)
{
    __HAL_RCC_ADC_CLK_ENABLE();
    
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    HAL_ADC_Init(&hadc1);
    
    ADC_ChannelConfTypeDef channel;
    channel.Channel = ADC_CHANNEL_0;
    channel.Rank = ADC_REGULAR_RANK_1;
    channel.SamplingTime = ADC_SAMPLETIME_39CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &channel);
}

void UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
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