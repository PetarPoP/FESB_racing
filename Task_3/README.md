# FESB Racing Intro Task - APPS & Pedal Mapping

[cite_start]This repository contains my solution for the combined introductory embedded task (APPS + Pedal Mapping) for the FESB Racing Software team[cite: 1, 3].

---

## Task Description

* [cite_start]**Platform:** NUCLEO-C031C6 development board (simulated in WOKWI)[cite: 8, 11].
* [cite_start]**Language:** C using the STM32 HAL library[cite: 9, 10].
* [cite_start]**Microcontroller:** STM32C031C6[cite: 7].
* **Core Requirements:**
    * [cite_start]Implement APPS sensor safety checks according to **Formula Student Germany (FSG)** rules[cite: 5, 31].
    * [cite_start]Simulate pedal mapping using two different mapping functions (Linear and Progressive)[cite: 26, 41].
    * [cite_start]Ensure completely non-blocking program execution using `HAL_GetTick()`[cite: 29, 46, 47].

---

## What I’ve done

[cite_start]The goal of this task was to integrate safety-critical sensor monitoring with customizable driver input mapping[cite: 4, 5].

Key points of the implementation:
* [cite_start]**APPS Redundancy:** The system periodically reads two potentiometers (APPS1 and APPS2) and normalizes their values to a 0–100% range[cite: 14, 15, 23, 24].
* [cite_start]**FSG Safety Logic:** * If the deviation between APPS1 and APPS2 exceeds **10%**, the state is considered implausible[cite: 32]. 
    * [cite_start]If this implausibility persists for more than **3000 ms**, a safety reaction is triggered[cite: 33, 34].
    * [cite_start]The safety reaction enters an error handler that blinks the LED every 200ms and halts normal operation[cite: 36, 37].
* [cite_start]**Pedal Mapping:** When the sensors are plausible, the pedal value is calculated as the average of both sensors[cite: 40].
    * [cite_start]**Linear Map:** Output is directly proportional to the average input[cite: 42].
    * [cite_start]**Progressive Map:** A quadratic function used for more granular control[cite: 43].
* [cite_start]**Diagnostics:** System state, including raw values, percentages, and deviation, is printed periodically to the UART terminal[cite: 21, 28, 38].



---

## Programming practices used

While following the task requirements, I applied several standard embedded programming practices:
* [cite_start]**Non-blocking Architecture:** I strictly avoided `HAL_Delay()` and used `HAL_GetTick()` to manage multiple independent timers for sensor reading, diagnostics, and LED blinking[cite: 45, 46, 47].
* **Structured Data:** Created an `APPS_Data_t` struct to encapsulate all sensor-related data, improving code organization and readability.
* [cite_start]**Input Debouncing:** Implemented a non-blocking debounce logic for the map selection button (PC13)[cite: 27].
* [cite_start]**Safety-First Design:** The code is designed to "fail-safe" by entering a permanent error loop if a critical sensor mismatch is detected[cite: 36].
* [cite_start]**Peripheral Management:** Grouped initialization into specific functions (`GPIO_Init`, `ADC_Init`, `UART_Init`) to maintain a clean `main` loop[cite: 12].

---

## Notes
* The system allows a manual "Reset" from the error state via a button press (PC13) specifically to facilitate faster testing cycles in the WOKWI simulator.

---

## Hardware Setup (Simulation)

* [cite_start]**APPS1 (Potentiometer):** ADC Channel 0 (PA0) [cite: 14]
* [cite_start]**APPS2 (Potentiometer):** ADC Channel 1 (PA1) [cite: 15]
* [cite_start]**Map Switch (Button):** PC13 (Internal Pull-up) [cite: 17]
* [cite_start]**Status/Error LED:** PA5 [cite: 19]
* [cite_start]**UART (Diagnostics):** USART2 (TX: PA2, RX: PA3) [cite: 20]

---

## Hardware Setup (Screenshot)

<img width="835" height="437" alt="image" src="https://github.com/user-attachments/assets/4f193208-7dc7-45a9-b73e-15678c85c354" />
