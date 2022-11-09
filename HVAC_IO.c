 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a través de estados.
 // Authors:         Ivan Urbina, Ernesto Arciniega, Luis Macias
 // Updated:         11/2022

#include "HVAC.h"

//Banderas boleanas par el control de persianas, secuencia de luces y final del propgrama
bool B_1 = false;
bool B_2 = false;
bool persiana1 = false;
bool persiana2 = false;
bool sl = 0;
bool final = false;
bool event = FALSE;            // Evento I/O que fuerza impresión inmediata.

//Arreglos de caracteres par impresion de estado de luces con el UART
char lux1[10];
char lux2[10];
char lux3[10];

uint8_t luxes[3];   //Arreglo para lectura de cada Luz con el ADC

//Enteros usados para ciclos For y Switch(secuencia de luces)
uint8_t j;
uint8_t i = 0;

/* **** SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ******** */
extern void Timer32_INT1 (void);         //Función de interrupción.
extern void Delay_ms (uint32_t time);   //Función de delay.
extern void Delay_ms1 (uint32_t time1); //Función de delay que evalua si se termina el programa.
void funcion_inicial(void);             //Funcion que se ejecuta al iniciar el programa, espera que se presione el Boton conectado en P2.5
void O_C_P1(void);                      //Funcion de apertura, cierre de persiana 1
void O_C_P2(void);                      //Funcion de apertura, cierre de persiana 2
void secuencia(void);                   //Funcion que ejecuta la secuencia de luces con el RGB
void terminar_programa(void);           //Funcion para terminar el programa

/*FUNCTION******************************************************************************
*
* Function Name    : System_InicialiceTIMER
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar TIMER32
*
*END***********************************************************************************/
void System_InicialiceTIMER (void)
{
    T32_Init1();
    Int_registerInterrupt(INT_T32_INT1, Timer32_INT1);
    Int_enableInterrupt(INT_T32_INT1);
}

/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void)
{
    GPIO_clear_interrupt_flag(P1,B1); // Limpia la bandera de la interrupción.
    GPIO_clear_interrupt_flag(P1,B4); // Limpia la bandera de la interrupción.

    if(!GPIO_getInputPinValue(Puerto1,BIT(persiana_1)))         //Cambia de estado la persiana 1
        B_1 = true;
    else if(!GPIO_getInputPinValue(Puerto1,BIT(persiana_2)))    //Cambia de estado la persiana 2
        B_2 = true;

    event = true;
}

/**********************************************************************************
 * Function: INT_SW2
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón de P2.5 o P2.6.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SW2(void)
{
    GPIO_clear_interrupt_flag(P2,B6);   // Limpia la bandera de la interrupción.
    GPIO_clear_interrupt_flag(P2,B5);   // Limpia la bandera de la interrupción.

    if(!GPIO_getInputPinValue(Puerto2,BIT(B6))) //Boton para la secuencia de luces
    {
        sl ^= 1;        //Se habilita la secuencia de luces
        event = true;   //Forza la impresion debido a el evento
    }

    if(!GPIO_getInputPinValue(Puerto2,BIT(B5))) //Boton que termina el programa
       final = true;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar las entradas y salidas GPIO.
*
*END***********************************************************************************/
void HVAC_InicialiceIO(void)
{
    // Para entradas y salidas ya definidas en la tarjeta, asi como los botones externos.
    GPIO_init_board();

    // Modo de interrupción de los botones principales y los externos (P2.5 y P2.6).
    GPIO_interruptEdgeSelect(Puerto1,BIT(persiana_1),   GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(Puerto1,BIT(persiana_2), GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(Puerto2,BIT(B5),   GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(Puerto2,BIT(B6), GPIO_HIGH_TO_LOW_TRANSITION);

    // Preparativos de interrupción.
    GPIO_clear_interrupt_flag(P1,B1);
    GPIO_clear_interrupt_flag(P1,B4);
    GPIO_enable_bit_interrupt(P1,B1);
    GPIO_enable_bit_interrupt(P1,B4);
    GPIO_clear_interrupt_flag(P2,B6);
    GPIO_enable_bit_interrupt(P2,B6);
    GPIO_clear_interrupt_flag(P2,B5);

    /* Uso del módulo Interrupt para generar la interrupción general y registro de esta en una función
    *  que se llame cuando la interrupción se active.                                                   */
    Int_registerInterrupt(INT_PORT1, INT_SWI);
    Int_enableInterrupt(INT_PORT1);
    Int_registerInterrupt(INT_PORT2, INT_SW2);
    Int_enableInterrupt(INT_PORT2);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    el módulo general ADC, 3 canales, uno para cada luz
*
*END***********************************************************************************/
void HVAC_InicialiceADC(void)
{
    // Iniciando ADC y canales.
    ADC_Initialize(ADC_14bitResolution, ADC_CLKDiv8);
    ADC_SetConvertionMode(ADC_SequenceOfChannelsRepeat);    //Lee 3 canales (3 potenciometros)

    ADC_ConfigurePinChannel(Lampara1, POT_PIN1, ADC_VCC_VSS);   // Pin AN1 5.4 para potenciómetro.
    ADC_ConfigurePinChannel(Lampara2, POT_PIN2, ADC_VCC_VSS);   // Pin AN0 5.5 para potenciómetro.
    ADC_ConfigurePinChannel(Lampara3, POT_PIN3, ADC_VCC_VSS);   // Pin AN5 5.0 para potenciómetro.

    ADC_SetStartOfSequenceChannel(Lampara1);
    ADC_SetEndOfSequenceChannel(Lampara3);                     // Termina en el AN5, canal 2.
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicación asíncrona).
*
*END***********************************************************************************/
void HVAC_InicialiceUART (void)
{
    UART_init();
}

/*FUNCTION******************************************************************************
*
* Function Name    : funcion_inicial
* Returned Value   : None.
* Comments         :
*    Realiza el estado inicial de la aplicacion
*END***********************************************************************************/
void funcion_inicial(void)
{
    bool condicion = true;

    //Inician todos los LED apagados
    GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  0);
    GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  0);
    GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  0);
    GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  0);
    UART_putsf(MAIN_UART, "Presione el switch del P2.5\r\n");   //Mensaje inicial
    GPIO_disable_bit_interrupt(P2,B5);

    while(condicion)                                            //Mientras no se presione un switch, no inicia la aplicacion
    {
        if(!GPIO_getInputPinValue(Puerto2,BIT(B5)))
        {
            UART_putsf(MAIN_UART, "Aplicacion iniciada\r\n");   //Mensaje de inicio de app
            GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  1);
            Delay_ms(200);
            GPIO_enable_bit_interrupt(P2,B5);
            condicion = false;
        }
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heartbeat
* Returned Value   : None.
* Comments         :
* Funcion que se ejecuta repetidamente, lee el ADC de cada Luz, evalua las persianas,
* la secuencia de luces y el final del programa
*END***********************************************************************************/
void HVAC_Heartbeat(void)
{
    ADC_trigger();                                  //Dispara el ADC
    while(ADC_is_busy());                           //Mientras el ADC convierte

    //Nos da el valor en Luxes(escala de 1-10 para cada luz medida)
    luxes[0] = (ADC_result(Lampara1)*11)/16384;
    luxes[1] = (ADC_result(Lampara2)*11)/16384;
    luxes[2] = (ADC_result(Lampara3)*11)/16384;

    if(B_1)     //Si se interrumpió por el SW1, llama a la funcion que controla el estado de la persiana 1
    {
        O_C_P1();
        B_1 = false;
    }

    if(B_2)    //Si se interrumpió por el SW2, llama a la funcion que controla el estado de la persiana 2
    {
        O_C_P2();
        B_2 = false;
    }

    if(sl)              //Si se interrumpe por el boton en P2.6 activa o desactiva la secuencia de luces
        secuencia();
    else                //Apaga el RGB
    {
        GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  0);
        GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  0);
        GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  0);
    }

    if(final)   //Si se desea entrar a la ventana de 5 segundos para terminar el programa
        terminar_programa();
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART el estado de las persianas, valor de las luces, y otros mensajes de
*    control cada 50 iteraciones o cada evento
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    static char iterations = 0;

    iterations++;
    if(iterations >= ITERATIONS_TO_PRINT || event == TRUE)
    {
       UART_putsf(MAIN_UART,"\r\n");

       //Convierte los valores enteros del arreglo luxes en string para ser impresos
       sprintf(lux1,"LUZ 1 = %d\r\n",luxes[0]);
       UART_putsf(MAIN_UART,lux1);

       sprintf(lux2,"LUZ 2 = %d\r\n",luxes[1]);
       UART_putsf(MAIN_UART,lux2);

       sprintf(lux3,"LUZ 3 = %d\r\n",luxes[2]);
       UART_putsf(MAIN_UART,lux3);

       //Dependiendo del estado del sistema imprime
       if(persiana1)
           UART_putsf(MAIN_UART, "Persiana 1 abierta\r\n");

       if(!persiana1)
           UART_putsf(MAIN_UART, "Persiana 1 cerrada\r\n");

       if(persiana2)
           UART_putsf(MAIN_UART, "Persiana 2 abierta\r\n");

       if(!persiana2)
           UART_putsf(MAIN_UART, "Persiana 2 cerrada\r\n");

       if(sl)
           UART_putsf(MAIN_UART, "Secuencia encendida\r\n");

       if(!sl)
           UART_putsf(MAIN_UART, "Secuencia apagada\r\n");

        Delay_ms(500);
        iterations = 0;
        event = FALSE;
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    :  O_C_P1
* Returned Value   : None.
* Comments         :
*    Abre o cierra la persiana 1 segun el estado acutual de la misma
*END***********************************************************************************/
void O_C_P1(void)
{
    if(!persiana1)
    {
        persiana1 = true;
        UART_putsf(MAIN_UART, "P1 UP...\r\n");
    }
    else if(persiana1)
    {
        persiana1 = false;
        UART_putsf(MAIN_UART, "P1 DOWN...\r\n");
    }

    Delay_ms(5000);
}

/*FUNCTION******************************************************************************
*
* Function Name    :  O_C_P2
* Returned Value   : None.
* Comments         :
*    Abre o cierra la persiana 2 segun el estado acutual de la misma
*END***********************************************************************************/
void O_C_P2(void)
{
    if(!persiana2)
    {
        persiana2 =  true;
        UART_putsf(MAIN_UART, "P2 UP...\r\n");
    }
    else if(persiana2)
    {
        persiana2 = false;
        UART_putsf(MAIN_UART, "P2 DOWN...\r\n");
    }

    Delay_ms(5000);
}

/*FUNCTION******************************************************************************
*
* Function Name    : secuencia
* Returned Value   : None.
* Comments         :
*   Realiza la secuencia de colores con el RGB
*END***********************************************************************************/
void secuencia(void)
{
    switch(i)   //Genera distintos colores con el RGB
    {
        case 0:
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  1);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  0);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  0);
            Delay_ms(500);
            i++;
        break;

        case 1:
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  0);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  1);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  0);
            Delay_ms(500);
            i++;
        break;

        case 2:
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  0);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  0);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  1);
            Delay_ms(500);
            i++;
        break;

        case 3:
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  1);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  1);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  0);
            Delay_ms(500);
            i++;
        break;

        case 4:
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  0);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  1);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  1);
            Delay_ms(500);
            i++;
        break;

        case 5:
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  1);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  0);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  1);
            Delay_ms(500);
            i++;
        break;

        case 6:
            GPIO_setOutput(BSP_LED2_PORT,  BSP_LED2,  1);
            GPIO_setOutput(BSP_LED3_PORT,  BSP_LED3,  1);
            GPIO_setOutput(BSP_LED4_PORT,  BSP_LED4,  1);
            Delay_ms(500);
            i = 0;
        break;
    }
    event = true;   //Forza la impresion
}

/*FUNCTION******************************************************************************
*
* Function Name    : terminar_programa
* Returned Value   : None.
* Comments         :
*   Si se presiona el boton del P2.5 con la aplicacion en ejecucion, entra en la
*   ventana de 5 segundos para terminar la aplicacion
*END***********************************************************************************/
void terminar_programa(void)
{
    Delay_ms1(5000);    //Ventana de 5 segundos
    final = false;      //Si no termina, continua el programa
}
