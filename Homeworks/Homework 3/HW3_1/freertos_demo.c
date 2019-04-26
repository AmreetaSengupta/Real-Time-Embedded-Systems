//*****************************************************************************
// File Name: freertos.c
// Description: Software Timer Handler signal a task using semaphore and updates the time of
// the timerhandlercall which is then printed in the task.
// Reference: Simple FREERTOS (freertos_demo) example code by TI.
// Author: Amreeta Sengupta
// Date: 3/21/2019
//*****************************************************************************

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
TimerHandle_t Timer1;
TickType_t tick_count;
void Task1(void* pvParameters);
void TimerCallback( TimerHandle_t Timer1 );

//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************
SemaphoreHandle_t g_pUARTSemaphore;

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
    g_pUARTSemaphore = xSemaphoreCreateBinary();

    Timer1=  xTimerCreate( "Timer1",pdMS_TO_TICKS(500),pdTRUE,( void * ) 0, TimerCallback );
    if(Timer1 == NULL)
    {
        UARTprintf("\nUnable to create Timer!\n");
    }
    else if (xTimerStart(Timer1,0) != pdPASS)
    {
        UARTprintf("\nUnable to start Timer!\n");
    }
    if(xTaskCreate(Task1, (const char *)"Task1", 100, NULL, tskIDLE_PRIORITY + 1, NULL) != pdTRUE)
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
    UARTprintf("\nTask 1!\n");

    while(1)
    {
        if(xSemaphoreTake(g_pUARTSemaphore, pdMS_TO_TICKS(600)) != pdPASS)
        {
            UARTprintf("\n SemaphoreTake failed!\n");
        }
        else
        {
            UARTprintf("\n Interrupt Tick Value = %d\n",tick_count);
        }
    }
}

void TimerCallback( TimerHandle_t Timer1 )
{
   // UARTprintf("\n\nTimer 1!\n");
   if(xSemaphoreGive(g_pUARTSemaphore)!= pdPASS)
   {
       UARTprintf("\n SemaphoreGive failed!\n");
   }
   tick_count = xTaskGetTickCount();
}

