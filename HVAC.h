 // FileName:        HVAC.h
 // Dependencies:    None.
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Incluye librerías, define ciertas macros y significados así como llevar un control de versiones.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#ifndef _hvac_h_
#define _hvac_h_

#pragma once

#define __MSP432P401R__
#define  __SYSTEM_CLOCK    48000000 // Frecuencias funcionales recomendadas: 12, 24 y 48 Mhz.

/* Archivos de cabecera importantes. */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Archivos de cabecera POSIX. */
#include <pthread.h>
#include <semaphore.h>
#include <ti/posix/tirtos/_pthread.h>
#include <ti/sysbios/knl/Task.h>

/* Archivos de cabecera para RTOS. */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>

/* Board Support Package. */
#include "Drivers/BSP.h"

// Definiciones Básicas.
#define ENTRADA 1
#define SALIDA 0

// Re-definición de los bits y puertos de entrada a utilizar.
#define FAN_ON      B3
#define FAN_AUTO    B4
#define SYSTEM_COOL B5
#define SYSTEM_OFF  B6
#define SYSTEM_HEAT B7
#define persiana_1  B1
#define persiana_2  B4

#define Puerto2        2
#define SYSTEM_PORT     2
#define Puerto1      1
#define FAN_PORTT        P2
#define SYSTEM_PORTT     P2
#define SETPOINT_PORTT   P1

#define Lampara1        CH0
#define Lampara2        CH1
#define Lampara3        CH2


#define POT_PIN1         AN1
#define POT_PIN2         AN0
#define POT_PIN3         AN5

// Re-definición de los bits y puertos de salida a utilizar.
#define FAN_LED     BSP_LED1
#define HEAT_LED    BSP_LED2
#define HBeatLED    BSP_LED3
#define COOL_LED    BSP_LED4

#define FAN_LED_PORT    BSP_LED1_PORT
#define HEAT_LED_PORT   BSP_LED2_PORT
#define HB_LED_PORT     BSP_LED3_PORT
#define COOL_LED_PORT   BSP_LED4_PORT

// Definiciones del estado 'normal' de los botones externos a la tarjeta (solo hay dos botones).
#define GND 0
#define VCC 1
#define NORMAL_STATE_EXTRA_BUTTONS GND  // Aqui se coloca GND o VCC.

// Definiciones del sistema.
#define MAX_MSG_SIZE 64
#define MAX_ADC_VALUE 16383             // (2 ^14 bits) es la resolución default.
#define MAIN_UART (uint32_t)(EUSCI_A0)
#define DELAY 20000
#define ITERATIONS_TO_PRINT 49

// Definición para el RTOS.
#define THREADSTACKSIZE1 1500

/* Funciones. */

extern void INT_SWI(void);
extern void INT_SW2(void);

/* Funciones de inicialización. */
extern void HVAC_InicialiceIO   (void);
extern void HVAC_InicialiceADC  (void);
extern void HVAC_InicialiceUART (void);
extern void System_InicialiceTIMER (void); // ESTO LO AÑADI PARA UTILIZAR EL "TIMER32"

/* Funciones principales. */

extern void HVAC_Heartbeat(void);
extern void HVAC_PrintState(void);



#endif
