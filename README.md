# MediTime — Automatic Medicine Dispenser

An **ATmega328P-based automatic medicine dispensing system** designed to help children take their prescribed medicine on time when a parent or guardian is unavailable.

MediTime provides scheduled medicine reminders, detects the child's presence using an ultrasonic sensor, and automatically dispenses medicine using a **two-stage servo mechanism**. The system also provides an LCD interface and buzzer alerts, with GSM notification planned for missed medication events.

## Features

- ⏰ **Scheduled medicine reminders** using the DS1302 Real-Time Clock
- 🔔 **Buzzer alarm** when medicine is due
- 🖥️ **16×2 LCD** for displaying time, alarms, and system status
- 📏 **Ultrasonic hand detection** to determine whether the child has approached the dispenser
- ⚙️ **Dual-servo dispensing mechanism**
- 💊 Automatic transfer of medicine between the upper and lower dispensing cups
- 🔄 Separate dispensing operations for two scheduled alarms
- 🔘 Four-button interface for setting and adjusting alarms
- 📱 **SIM800L GSM module** planned for notifying the parent when medication is missed
- 🔌 Built around the **ATmega328P microcontroller**

## System Overview

MediTime consists of two vertically positioned servo-controlled medicine cups.

The **upper servo** stores the medicine and transfers a tablet into the lower cup when required. The **lower servo** subsequently rotates to dispense the tablet to the child.

### General Operation

```text
              ┌─────────────────┐
              │   Upper Cup      │
              │   Medicine       │
              └────────┬────────┘
                       │
                 Upper Servo
                    (PB2)
                       │
                       ▼
              ┌─────────────────┐
              │   Lower Cup      │
              │   Medicine       │
              └────────┬────────┘
                       │
                 Lower Servo
                    (PB3)
                       │
                       ▼
                Child's Hand

## Working Principle

### Alarm 1

1. The DS1302 RTC continuously keeps track of the current time.
2. When **Alarm 1** time is reached, the buzzer alerts the child.
3. The ultrasonic sensor monitors the area around the dispensing cup.
4. If the child approaches and takes the medicine, the system recognizes the interaction.
5. If the medicine is not taken within the configured waiting period:
   - The lower servo rotates to dispense the stored tablet.
   - The upper servo rotates to transfer another tablet into the lower cup.
   - Both servos return to their initial positions.
6. The system can subsequently notify the parent through the GSM module.

### Alarm 2

1. When **Alarm 2** time is reached, the buzzer alerts the child.
2. The lower servo rotates to dispense the previously prepared medicine.
3. The servo returns to its original position.

## Hardware Components

| Component | Purpose |
|---|---|
| ATmega328P | Main microcontroller |
| DS1302 RTC | Real-time clock and alarm timing |
| 16×2 LCD | User interface and status display |
| MG90S Servo ×2 | Medicine dispensing mechanism |
| HC-SR04 Ultrasonic Sensor | Hand/presence detection |
| Buzzer | Medicine reminder |
| Push Buttons ×4 | Alarm configuration |
| SIM800L GSM Module | Parent notification |
| 5V Power Supply | System power |
| Medicine Cups | Medicine storage and dispensing |

## Pin Configuration

### ATmega328P

| Component | ATmega328P Pin |
|---|---|
| DS1302 CLK | PD2 |
| DS1302 IO | PD3 |
| DS1302 CE | PD6 |
| LCD RS | PD4 |
| LCD EN | PD5 |
| LCD Data D4–D7 | PC0–PC3 |
| Buzzer | PB4 |
| Set Button | PB5 |
| Next Button | PC4 |
| Up Button | PC5 |
| Down Button | PD7 |
| Ultrasonic TRIG | PB0 |
| Ultrasonic ECHO | PB1 |
| Upper Servo | PB2 / OC1B |
| Lower Servo | PB3 |

> **Note:** The GSM module is not included in the current integrated firmware version and will be added as a future module.

## Software

The firmware is written in **Embedded C** and compiled for the ATmega328P using **AVR-GCC**.

### Development Tools

- AVR-GCC
- AVRDUDE
- USBasp programmer
- ATmega328P
- 8 MHz clock
- Embedded C
- Makefile / AVR development environment

## Firmware Structure

A recommended repository structure is:

```text
MediTime/
│
├── README.md
│
├── firmware/
│   ├── main.c
│   ├── lcd.c
│   ├── lcd.h
│   ├── ds1302.c
│   ├── ds1302.h
│   ├── servo.c
│   ├── servo.h
│   ├── ultrasonic.c
│   ├── ultrasonic.h
│   └── Makefile
│
├── gsm/
│   ├── sim800l.c
│   └── sim800l.h
│
├── hardware/
│   ├── circuit-diagram/
│   ├── pcb/
│   └── mechanical-design/
│
├── documentation/
│   ├── project-report/
│   └── presentation/
│
└── images/
    ├── prototype.jpg
    ├── circuit.jpg
    └── dispenser.jpg

## Current Firmware

The current firmware integrates:

- DS1302 RTC
- 16×2 LCD
- Four-button alarm configuration
- Buzzer
- HC-SR04 ultrasonic sensor
- Upper MG90S servo
- Lower MG90S servo
- Two medicine alarms

The system currently supports **two scheduled medicine times**.

## Alarm Configuration

The four buttons are used as follows:

| Button | Function |
|---|---|
| SET | Select alarm configuration mode |
| NEXT | Select hour/minute |
| UP | Increase selected value |
| DOWN | Decrease selected value |

The LCD displays the current time and configured alarm times.

Example:

```text
Time 12:30:45
A 12:35 B 18:30

## Medicine Dispensing Sequence

The basic dispensing sequence is:

```text
             Alarm Time
                  │
                  ▼
           Buzzer Activated
                  │
                  ▼
       Detect Child's Hand
             /        \
           Yes         No
            │           │
            ▼           ▼
      Medicine Taken   Wait
                        │
                    5 Minutes
                        │
                        ▼
                  Lower Servo
                     180°
                        │
                        ▼
                Medicine Released
                        │
                        ▼
                  Upper Servo
                     180°
                        │
                        ▼
             Refill Lower Cup
                        │
                        ▼
              Return Servos

## Future Improvements

The project can be extended with:

- 📱 **SIM800L GSM notifications** to the parent
- 📩 SMS notification when medicine is missed
- 📊 Medication history and logging
- 🔋 Battery backup
- 🔐 Secure medicine compartment
- 🔔 Different alarm patterns for different medicines
- 🧠 Improved medicine detection
- 📱 Mobile application for remote configuration
- 🗓️ Multiple medication schedules
- ⚠️ Low-medicine-level detection
- 🔄 Automatic daily schedule management

## Project Objective

The main objective of MediTime is to develop a low-cost embedded system that can **automate medication reminders and dispensing**, reducing the dependency on a parent or caregiver being physically present at the exact medication time.

The project combines **embedded systems, real-time scheduling, sensor interfacing, motor control, human interaction, and IoT/GSM communication** into a practical assistive technology solution.

## Project Status

**Current Status:** 🚧 Prototype Development

### Completed

- ATmega328P firmware
- DS1302 RTC integration
- 16×2 LCD interface
- Alarm configuration using buttons
- Buzzer reminder
- Ultrasonic hand detection
- Upper servo control
- Lower servo control
- Two-stage medicine dispensing mechanism

### In Progress

- SIM800L GSM integration
- Missed-medication SMS notification
- Final mechanical enclosure
- Full system testing
- Medication history

## Disclaimer

MediTime is an **academic undergraduate embedded-systems project and prototype**. It is intended for educational and demonstration purposes and should not be used as a replacement for professional medical supervision or a certified medication dispensing device.
