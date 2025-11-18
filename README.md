# FESB Racing Intro Task 1 – NUCLEO-C031C6

This repository contains my solution for the first introductory embedded task for FESB Racing.

## Task Description

- Platform: **NUCLEO-C031C6** development board (simulated in [WOKWI](https://wokwi.com/projects/new/st-nucleo-c031c6))  
- Language: **C**  
- Requirements:  
  - Display all **4-bit code words** (0000 to 1111) on **4 LEDs**.  
  - LEDs represent bits: ON = 1, OFF = 0.  
  - Code words must change **every 0.5 seconds** from lowest to highest.

## What I've done

My main goal was to keep this task simple, as it is an introductory exercise. At the same time, I wanted to follow some good programming practices, for example:  

- Minimized memory usage by using appropriate data types (`uint8_t` for the counter, `uint16_t` for pin masks).  
- Kept the code easy to read and maintain by using helper functions.  
- Used `__HAL_RCC_GPIOA_CLK_ENABLE()` even though the simulated board already enables the clock, this ensures the code works reliably across different STM32 boards.  
- Set pins to `GPIO_PIN_RESET` after initialization with `HAL_GPIO_WritePin(GPIOA, pins[i], GPIO_PIN_RESET)` not strictly necessary, but good practice to start with known states.  

## Notes

- This project is for learning purposes.

## Hardware Setup Screenshot

- Used a breadboard with all LED cathodes connected to GND and anodes connected to resistors leading to PA0, PA1, PA4, and PA11.  
  This setup ensures the LEDs turn on when the GPIO pin outputs HIGH, while the resistors limit the current to protect the LEDs and the microcontroller.

<img width="609" height="742" alt="{9729CE4E-E81D-43DA-973D-C576236A6FE7}" src="https://github.com/user-attachments/assets/eab14b59-7838-46c3-b95f-18a1e43de1cf" />
