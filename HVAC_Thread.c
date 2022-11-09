 // FileName:        HVAC_Thread.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Definición de las funciones del thread de HVAC_Thread().
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "hvac.h"                           // Incluye definición del sistema.

/*****  SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ******** */
bool retraso = false,delay1 = false,boton = true;
uint32_t j;
void Timer32_INT1 (void); // Función de interrupción.
void Delay_ms (uint32_t time); // Función de delay.
void Delay_ms1(uint32_t time1);
extern void funcion_inicial(void);
/***********************************************************************************************/

/*********************************THREAD******************************************
 * Function: HVAC_Thread
 * Preconditions: None.
 * Overview: Realiza la lectura de la temperatura y controla salidas actualizando
 *           a su vez entradas. Imprime estados. También contiene el heartbeat.
 * Input:  Apuntador vacío que puede apuntar cualquier tipo de dato.
 * Output: None.
 *
 ********************************************************************************/

void *HVAC_Thread(void *arg0)
{
    SystemInit();
    System_InicialiceTIMER();            // SE AÑADIO LA FUNCION PARA INICIALIZAR EL TIMER
    HVAC_InicialiceIO();
    HVAC_InicialiceADC();
    HVAC_InicialiceUART();
    funcion_inicial();

    while(boton)
    {
        HVAC_PrintState();
        HVAC_Heartbeat();
    }
    return 0;
}



/* *********  FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ********* */
void Delay_ms(uint32_t time)
{
    T32_EnableTimer1(); // Habilita timer.
    T32_EnableInterrupt1(); // Habilita interrupción.
    // Carga de valor en milisegundos.
    T32_SetLoadValue1(time*(__SYSTEM_CLOCK/1000));
    retraso = true;
    while(retraso); // While enclavado.
}
void Timer32_INT1(void)
{
    T32_ClearInterruptFlag1(); // Al llegar a la interrupción
    retraso = false; // desenclava el while.
    delay1 = false;
}


void Delay_ms1(uint32_t time1)
{
    UART_putsf(MAIN_UART, "Vuelva a presionar si desea terminar tiene 5 seg\r\n");  //Mensaje para terminar la aplicacion
    T32_EnableTimer1();                                                 // Habilita timer.
    T32_EnableInterrupt1();                                             // Habilita interrupción.
    T32_SetLoadValue1(time1*(__SYSTEM_CLOCK/1000));                     // Carga de valor en milisegundos.
    delay1 = true;

    while(delay1)
    {
        for(j = 0;j< 300000;j++);   //Retardo para el rebote mecanico

        if(!GPIO_getInputPinValue(Puerto2,BIT(B5)))
        {
            UART_putsf(MAIN_UART, "Aplicacion terminada\r\n");      //Mensaje final
            GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  0);
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  0);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  0);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  0);
            delay1 = false;
            boton = false;
        }
    }
}
