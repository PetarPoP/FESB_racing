#include "stm32c0xx_hal.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

uint16_t pins[] = {GPIO_PIN_11, GPIO_PIN_4, GPIO_PIN_1, GPIO_PIN_0};

GPIO_PinState bitToState(uint8_t val, uint8_t bit) {
    return (val & (1 << bit)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    uint8_t value = 0;

    while (1)
    {
        for(int i = 0; i < 4; i++)
        {
            HAL_GPIO_WritePin(GPIOA, pins[i], bitToState(value, i));
        }

        HAL_Delay(500);

        value++;
        if (value > 15) value = 0;
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    for(int i = 0; i < 4; i++) {
        GPIO_InitStruct.Pin = pins[i];
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        HAL_GPIO_WritePin(GPIOA, pins[i], GPIO_PIN_RESET);
    }
}