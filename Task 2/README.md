# FESB Racing Intro Task 2

This repository contains my solution for the first introductory embedded task for FESB Racing.

---

## Task Description

* Platform: NUCLEO-C031C6 development board (simulated in WOKWI)
* Language: C
* Requirements:
  * Transmitter (Node 1) every 500 ms creates a CAN message containing:
    * Device ID
    * Measured ADC value
    * LED status (0 or 1)
  * Receiver (Node 2) retrieves the message from the buffer, parses the data, and:
    * If ADC > 2000, prints a warning message: “High ADC value from Node 1!”
    * If ADC > 2000, turns on the LED (LED_status = 1)
    * Otherwise, prints the message contents to the terminal
  * The Receiver additionally prints the following message every 1 second: "ACK sent"

---

## What I’ve done

The goal of this task was to keep the implementation **simple and clear**, while still demonstrating several important embedded concepts. The program simulates two logical nodes (TX and RX) running on the same microcontroller.

Key points of the implementation:

* ADC value is read every **500 ms** and packed into a message structure
* A custom `Message` struct is used to clearly define transmitted data
* TX prints sent data over UART for debugging
* RX processes the received message and:

  * Turns ON the LED if ADC value is above a defined threshold
  * Turns OFF the LED otherwise
* RX periodically sends a simulated **ACK message** every 1000 ms
* Timing is handled using `HAL_GetTick()` instead of blocking delays

---

## Programming practices used

While this is an introductory task, I tried to follow good embedded programming practices:

* Used **fixed-width data types** (`uint8_t`, `uint16_t`, `uint32_t`) to ensure predictable behavior
* Avoided `HAL_Delay()` and implemented non-blocking timing using system ticks
* Grouped peripheral initialization into dedicated functions (`GPIO_Init`, `ADC_Init`, `UART_Init`)
* Used UART output to clearly visualize TX/RX behavior and system state
* Explicitly enabled peripheral clocks to keep the code portable across STM32 devices

---

## Notes

* This project is intended for learning purposes

---

## Hardware Setup (Simulation)

* **LED:** PA5
* **ADC input:** ADC channel 0
* **UART:** USART2

  * TX: PA2
  * RX: PA3

---

## Hardware Setup (Screenshot)

<img width="668" height="451" alt="Task 2 Wokwi scheme" src="https://github.com/user-attachments/assets/73475f52-c114-4466-a9ff-233ad1ba9383" />
