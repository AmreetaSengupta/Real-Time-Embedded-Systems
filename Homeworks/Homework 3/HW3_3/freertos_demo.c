//***********************************************************************************
// File Name: freertos.c
// Description: Creation of two software timers of 10ms and 40ms to synchronize two
// tasks with request time of 30ms and 80ms with execution time of Fibonacci series
// of 10ms and 40ms respectively
// References: Simple FREERTOS (freertos_demo) example code by TI,
// Fibonacci Series Generation from rt_thread_improved by Professor Sam Siewert.
// Author: Amreeta Sengupta
// Date: 3/21/2019
//************************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
TimerHandle_t Timer1,Timer2;
volatile int flag1, flag2;
int count1, count2;
void Task1(void* pvParameters);
void Task2(void* pvParameters);
void TimerCallback1( TimerHandle_t Timer1 );
void TimerCallback2( TimerHandle_t Timer2 );
//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************
   SemaphoreHandle_t g_pUARTSemaphore1,g_pUARTSemaphore2;
   xSemaphoreHandle g_pUARTSemaphore;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************

int
main(void)
{
    //
    // Set the clocking to run at 50 MHz from the PLL.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    //
    // Initialize the UART and configure it for 115,200, 8-N-1 operation.
    //
    ConfigureUART();

    //
    // Print demo introduction.
    //

    UARTprintf("\nWelcome!\n");


    //
    // Create a mutex to guard the UART.
    //
    g_pUARTSemaphore1 = xSemaphoreCreateBinary();
    g_pUARTSemaphore2 = xSemaphoreCreateBinary();
    g_pUARTSemaphore = xSemaphoreCreateMutex();

    Timer1=  xTimerCreate( "Timer1",pdMS_TO_TICKS(10),pdTRUE,( void * ) 0, TimerCallback1 );
    if(Timer1 == NULL)
    {
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nUnable to create Timer!\n");
        xSemaphoreGive(g_pUARTSemaphore);
    }
    else
    {
        if(xTimerStart(Timer1,0) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\nUnable to start Timer1!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
    }
    Timer2=  xTimerCreate( "Timer2",pdMS_TO_TICKS(40),pdTRUE,( void * ) 0, TimerCallback2 );
    if(Timer2 == NULL)
    {
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nUnable to create Timer!\n");
        xSemaphoreGive(g_pUARTSemaphore);
    }
    else
    {
        if(xTimerStart(Timer2,0) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\nUnable to start Timer2!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
    }
    if(xTaskCreate(Task1, (const char *)"Task1", 100, NULL, tskIDLE_PRIORITY + 1, NULL) != pdTRUE)
    {
        while(1)
        {

        }
     }
    if(xTaskCreate(Task2, (const char *)"Task2", 100, NULL, tskIDLE_PRIORITY + 1, NULL) != pdTRUE)
       {
           while(1)
           {

           }
        }

    vTaskStartScheduler();

    //
    // In case the scheduler returns for some reason, print an error and loop
    // forever.
    //

    while(1)
    {
    }
}

void Task1(void* pvParameters)
{
    unsigned int idx = 0, jdx = 1;
    unsigned int fib = 0, fib0 = 0, fib1 = 1;
    TickType_t Tick_count_start, Tick_count_stop, Tick_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\nTask 1!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(1)
    {
        Tick_count_start = xTaskGetTickCount();

        for(idx=0; idx < 0xFFFFF; idx++)
        {
            fib = fib0 + fib1;
            while(jdx < 0xFFFFF)
            {
                fib0 = fib1;
                fib1 = fib;
                fib = fib0 + fib1;
                jdx++;

                if(flag1==1)
                {
                    break;
                }
            }
            if(flag1==1)
            {
                flag1 =0;
                break;
            }
        }

        Tick_count_stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\ntick_count1 = %d\n",Tick_count_stop-Tick_count_start);
        xSemaphoreGive(g_pUARTSemaphore);

        if(xSemaphoreTake(g_pUARTSemaphore1,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake1 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }

        Tick_stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nTime taken for the task1 = %d\n",Tick_stop-Tick_count_start);
        xSemaphoreGive(g_pUARTSemaphore);
    }
}

void Task2(void* pvParameters)
{
    unsigned int idx = 0, jdx = 1;
    unsigned int fib = 0, fib0 = 0, fib1 = 1;
    TickType_t Tick_count_start, Tick_count_stop,Tick_stop;
    UARTprintf("\nTask 2!\n");

    while(1)
    {
        Tick_count_start = xTaskGetTickCount();

        for(idx=0; idx < 0xFFFFF; idx++)
        {
            fib = fib0 + fib1;
            while(jdx < 0xFFFFF)
            {
                fib0 = fib1;
                fib1 = fib;
                fib = fib0 + fib1;
                jdx++;

                if(flag2==1)
                {
                    break;
                }
            }
            if(flag2==1)
            {
                flag2 =0;
                break;
            }
        }
        Tick_count_stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\ntick_count2 = %d\n",Tick_count_stop-Tick_count_start);
        xSemaphoreGive(g_pUARTSemaphore);

        if(xSemaphoreTake(g_pUARTSemaphore2,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake2 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        Tick_stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nTime taken for the task2 = %d\n",Tick_stop-Tick_count_start);
        xSemaphoreGive(g_pUARTSemaphore);
    }
}

void TimerCallback1( TimerHandle_t Timer1 )
{
   count1++;
   if(count1%3==1)
   {
       flag1=1;
   }
   if(count1%3==0)
   {
       xSemaphoreGive(g_pUARTSemaphore1);
   }
}

void TimerCallback2( TimerHandle_t Timer2 )
{
    count2++;
    if(count2%2==1)
    {
        flag2=1;
    }
    if(count2%2==0)
    {
        xSemaphoreGive(g_pUARTSemaphore2);
    }
}
