//**************************************************************************************
// File Name: freertos_demo.c
// Description: Scheduling of seven tasks using a sequencer for real time applications. This code is used for 
// question 2 by enabling #define seqgen1x for seqgen1x .c and #define seqgen10x for seqgen2x.c
// This code can be used for question 3 by enabling #define seqgen100x.
// Reference: Simple FREERTOS (freertos_demo) example code by TI.
// Author: Amreeta Sengupta and Ayush Dhoot
// Date: 4/09/2019
//***************************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//#define seqgen1x
//#define seqgen10x
#define seqgen100x

#define TRUE (1)
#define FALSE (0)

#ifdef seqgen1x
#define num (30)
#endif

#ifdef seqgen10x
#define num (60)
#endif

#ifdef seqgen100x
#define num (100)
#endif

#define SEQUENCER_PERIOD_DIV (num)

void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);
void Task4(void* pvParameters);
void Task5(void* pvParameters);
void Task6(void* pvParameters);
void Task7(void* pvParameters);

int abortS1=FALSE, abortS2=FALSE, abortS3=FALSE, abortS4=FALSE, abortS5=FALSE, abortS6=FALSE, abortS7=FALSE;
unsigned long long seqCnt=0;
TickType_t main_start, Max1=0, Max2=0, Max3=0, Max4=0, Max5=0, Max6=0, Max7=0;
//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************
xSemaphoreHandle g_pUARTSemaphore;
SemaphoreHandle_t g_pUARTSemaphore1;
SemaphoreHandle_t g_pUARTSemaphore2;
SemaphoreHandle_t g_pUARTSemaphore3;
SemaphoreHandle_t g_pUARTSemaphore4;
SemaphoreHandle_t g_pUARTSemaphore5;
SemaphoreHandle_t g_pUARTSemaphore6;
SemaphoreHandle_t g_pUARTSemaphore7;

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

    main_start = xTaskGetTickCount();

    ConfigureUART(); // Initialize the UART and configure it for 115,200, 8-N-1 operation.

    ROM_IntMasterEnable();  // Enable processor interrupts.

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);   // Enable the peripherals used by this example.
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // Configure the one 32-bit periodic timer.
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A,ROM_SysCtlClockGet()/SEQUENCER_PERIOD_DIV);
    ROM_IntEnable(INT_TIMER0A); // Setup the interrupts for the timer timeouts.
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A); // Enable the timers.


    UARTprintf("\nWelcome\n");
    g_pUARTSemaphore1 = xSemaphoreCreateBinary();
    g_pUARTSemaphore2 = xSemaphoreCreateBinary();
    g_pUARTSemaphore3 = xSemaphoreCreateBinary();
    g_pUARTSemaphore4 = xSemaphoreCreateBinary();
    g_pUARTSemaphore5 = xSemaphoreCreateBinary();
    g_pUARTSemaphore6 = xSemaphoreCreateBinary();
    g_pUARTSemaphore7 = xSemaphoreCreateBinary();

    g_pUARTSemaphore = xSemaphoreCreateMutex(); // Create a mutex to guard the UART.

    if(xTaskCreate(Task1, (const char *)"Task1", 100, NULL, configMAX_PRIORITIES, NULL) != pdTRUE)
    {
        while(1)
        {

        }
    }

    if(xTaskCreate(Task2, (const char *)"Task2", 100, NULL, configMAX_PRIORITIES-1, NULL) != pdTRUE)
    {
        while(1)
        {

        }
    }

    if(xTaskCreate(Task3, (const char *)"Task3", 100, NULL, configMAX_PRIORITIES-2, NULL) != pdTRUE)
    {
        while(1)
        {

        }
    }

    if(xTaskCreate(Task4, (const char *)"Task4", 100, NULL, configMAX_PRIORITIES-1, NULL) != pdTRUE)
    {
        while(1)
        {

        }
    }

    if(xTaskCreate(Task5, (const char *)"Task5", 100, NULL, configMAX_PRIORITIES-2, NULL) != pdTRUE)
    {
        while(1)
        {

        }
    }

    if(xTaskCreate(Task6, (const char *)"Task6", 100, NULL, configMAX_PRIORITIES-1, NULL) != pdTRUE)
    {
        while(1)
        {

        }
    }

    if(xTaskCreate(Task7, (const char *)"Task7", 100, NULL, configMAX_PRIORITIES-3, NULL) != pdTRUE)
    {
        while(1)
        {

        }
    }

    //
    // Start the scheduler.  This should not return.
    //
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
    TickType_t Tick_count_start, Tick_count_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n Welcome to Task 1!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(!abortS1)
    {
        if(xSemaphoreTake(g_pUARTSemaphore1,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake1 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        else
        {
            Tick_count_start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n Task 1 started at:%d ms\n",(Tick_count_start-main_start));
            xSemaphoreGive(g_pUARTSemaphore);
            Tick_count_stop = xTaskGetTickCount();
            if(Max1 < Tick_count_stop-Tick_count_start)
            {
                Max1 = Tick_count_stop-Tick_count_start;
            }
        }
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n WCET for Task 1:%d ms\n",Max1);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

void Task2(void* pvParameters)
{
    TickType_t Tick_count_start, Tick_count_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n Welcome to Task 2!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(!abortS2)
    {
        if(xSemaphoreTake(g_pUARTSemaphore2,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake2 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        else
        {
            Tick_count_start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n Task 2 started at:%d ms\n",(Tick_count_start-main_start));
            xSemaphoreGive(g_pUARTSemaphore);
            Tick_count_stop = xTaskGetTickCount();
            if(Max2 < Tick_count_stop-Tick_count_start)
            {
                Max2 = Tick_count_stop-Tick_count_start;
            }
        }
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n WCET for Task 2:%d ms\n",Max2);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

void Task3(void* pvParameters)
{
    TickType_t Tick_count_start, Tick_count_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n Welcome to Task 3!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(!abortS3)
    {
        if(xSemaphoreTake(g_pUARTSemaphore3,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake3 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        else
        {
            Tick_count_start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n Task 3 started at:%d ms\n",(Tick_count_start-main_start));
            xSemaphoreGive(g_pUARTSemaphore);
            Tick_count_stop = xTaskGetTickCount();
            if(Max3 < Tick_count_stop-Tick_count_start)
            {
                Max3 = Tick_count_stop-Tick_count_start;
            }
        }
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n WCET for Task 3:%d ms\n",Max3);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

void Task4(void* pvParameters)
{
    TickType_t Tick_count_start, Tick_count_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n Welcome to Task 4!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(!abortS4)
    {
        if(xSemaphoreTake(g_pUARTSemaphore4,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake4 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        else
        {
            Tick_count_start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n Task 4 started at:%d ms\n",(Tick_count_start-main_start));
            xSemaphoreGive(g_pUARTSemaphore);
            Tick_count_stop = xTaskGetTickCount();
            if(Max4 < Tick_count_stop-Tick_count_start)
            {
                Max4 = Tick_count_stop-Tick_count_start;
            }
        }
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n WCET for Task 4:%d ms\n",Max4);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

void Task5(void* pvParameters)
{
    TickType_t Tick_count_start, Tick_count_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n Welcome to Task 5!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(!abortS5)
    {
        if(xSemaphoreTake(g_pUARTSemaphore5,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake5 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        else
        {
            Tick_count_start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n Task 5 started at:%d ms\n",(Tick_count_start-main_start));
            xSemaphoreGive(g_pUARTSemaphore);
            Tick_count_stop = xTaskGetTickCount();
            if(Max5 < Tick_count_stop-Tick_count_start)
            {
                Max5 = Tick_count_stop-Tick_count_start;
            }
        }
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n WCET for Task 5:%d ms\n",Max5);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

void Task6(void* pvParameters)
{
    TickType_t Tick_count_start, Tick_count_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n Welcome to Task 6!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(!abortS6)
    {
        if(xSemaphoreTake(g_pUARTSemaphore6,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake6 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        else
        {
            Tick_count_start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n Task 6 started at:%d ms\n",(Tick_count_start-main_start));
            xSemaphoreGive(g_pUARTSemaphore);
            Tick_count_stop = xTaskGetTickCount();
            if(Max6 < Tick_count_stop-Tick_count_start)
            {
                Max6 = Tick_count_stop-Tick_count_start;
            }
        }
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n WCET for Task 6:%d ms\n",Max6);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

void Task7(void* pvParameters)
{
    TickType_t Tick_count_start, Tick_count_stop;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n Welcome to Task 7!\n");
    xSemaphoreGive(g_pUARTSemaphore);
    while(!abortS7)
    {
        if(xSemaphoreTake(g_pUARTSemaphore7,portMAX_DELAY) != pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n SemaphoreTake7 failed!\n");
            xSemaphoreGive(g_pUARTSemaphore);
        }
        else
        {
            Tick_count_start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n Task 7 started at:%d ms\n",(Tick_count_start-main_start));
            xSemaphoreGive(g_pUARTSemaphore);
            Tick_count_stop = xTaskGetTickCount();
            if(Max7 < Tick_count_stop-Tick_count_start)
            {
                Max7 = Tick_count_stop-Tick_count_start;
            }
        }
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n WCET for Task 7:%d ms\n",Max7);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}
void Timer0IntHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  // Clear the timer interrupt.
    ROM_IntMasterDisable();

    seqCnt++;
    #ifdef seqgen1x
    if((seqCnt % 10) == 0) // Servcie_1 = RT_MAX-1   @ 3 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore1, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive1 failed!\n");
        }
     }
    if((seqCnt % 30) == 0) // Service_2 = RT_MAX-2   @ 1 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore2, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive2 failed!\n");
        }
    }
    if((seqCnt % 60) == 0) // Service_3 = RT_MAX-3   @ 0.5 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore3, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive3 failed!\n");
        }
    }
    if((seqCnt % 30) == 0) // Service_4 = RT_M AX-2   @ 1 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore4, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive4 failed!\n");
        }
    }
    if((seqCnt % 60) == 0) // Service_5 = RT_MAX-3   @ 0.5 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore5, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive5 failed!\n");
        }
    }
    if((seqCnt % 30) == 0) // Service_6 = RT_MAX-2   @ 1 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore6, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive6 failed!\n");
        }
    }
    if((seqCnt % 300) == 0) // Service_7 = RT_MIN   @0.1 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore7, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive7 failed!\n");
        }
    }
    #endif

    #ifdef seqgen10x
    if((seqCnt % 2) == 0) // Servcie_1 = RT_MAX-1   @ 30 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore1, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive1 failed!\n");
        }
    }
    if((seqCnt % 6) == 0) // Service_2 = RT_MAX-2   @ 10 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore2, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive2 failed!\n");
        }
    }
    if((seqCnt % 12) == 0)// Service_3 = RT_MAX-3   @ 5 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore3, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive3 failed!\n");
        }
    }
    if((seqCnt % 6) == 0) // Service_4 = RT_MAX-2   @ 10 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore4, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive4 failed!\n");
        }
    }
    if((seqCnt % 12) == 0)// Service_5 = RT_MAX-3   @ 5 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore5, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive5 failed!\n");
        }
    }
    if((seqCnt % 6) == 0) // Service_6 = RT_MAX-2   @ 10 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore6, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive6 failed!\n");
        }
    }
    if((seqCnt % 60) == 0)// Service_7 = RT_MIN   @ 1 Hz
    {
        if(xSemaphoreGiveFromISR(g_pUARTSemaphore7, &xHigherPriorityTaskWoken)!= pdTRUE)
        {
            UARTprintf("\n SemaphoreGive7 failed!\n");
        }
    }
    #endif

#ifdef seqgen100x
  if((seqCnt % 1) == 0) // Servcie_1 = RT_MAX-1   @ 30 Hz
  {
      if(xSemaphoreGiveFromISR(g_pUARTSemaphore1, &xHigherPriorityTaskWoken)!= pdTRUE)
      {
          UARTprintf("\n SemaphoreGive1 failed!\n");
      }
  }
  if((seqCnt % 3) == 0) // Service_2 = RT_MAX-2   @ 10 Hz
  {
      if(xSemaphoreGiveFromISR(g_pUARTSemaphore2, &xHigherPriorityTaskWoken)!= pdTRUE)
      {
          UARTprintf("\n SemaphoreGive2 failed!\n");
      }
  }
  if((seqCnt % 6) == 0)// Service_3 = RT_MAX-3   @ 5 Hz
  {
      if(xSemaphoreGiveFromISR(g_pUARTSemaphore3, &xHigherPriorityTaskWoken)!= pdTRUE)
      {
          UARTprintf("\n SemaphoreGive3 failed!\n");
      }
  }
  if((seqCnt % 3) == 0) // Service_4 = RT_MAX-2   @ 10 Hz
  {
      if(xSemaphoreGiveFromISR(g_pUARTSemaphore4, &xHigherPriorityTaskWoken)!= pdTRUE)
      {
          UARTprintf("\n SemaphoreGive4 failed!\n");
      }
  }
  if((seqCnt % 6) == 0)// Service_5 = RT_MAX-3   @ 5 Hz
  {
      if(xSemaphoreGiveFromISR(g_pUARTSemaphore5, &xHigherPriorityTaskWoken)!= pdTRUE)
      {
          UARTprintf("\n SemaphoreGive5 failed!\n");
      }
  }
  if((seqCnt % 3) == 0) // Service_6 = RT_MAX-2   @ 10 Hz
  {
      if(xSemaphoreGiveFromISR(g_pUARTSemaphore6, &xHigherPriorityTaskWoken)!= pdTRUE)
      {
          UARTprintf("\n SemaphoreGive6 failed!\n");
      }
  }
  if((seqCnt % 30) == 0)// Service_7 = RT_MIN   @ 1 Hz
  {
      if(xSemaphoreGiveFromISR(g_pUARTSemaphore7, &xHigherPriorityTaskWoken)!= pdTRUE)
      {
          UARTprintf("\n SemaphoreGive7 failed!\n");
      }
  }
  #endif
        if(seqCnt >= 900)
        {
            xSemaphoreGiveFromISR(g_pUARTSemaphore1, &xHigherPriorityTaskWoken);
            xSemaphoreGiveFromISR(g_pUARTSemaphore2, &xHigherPriorityTaskWoken);
            xSemaphoreGiveFromISR(g_pUARTSemaphore3, &xHigherPriorityTaskWoken);
            xSemaphoreGiveFromISR(g_pUARTSemaphore4, &xHigherPriorityTaskWoken);
            xSemaphoreGiveFromISR(g_pUARTSemaphore5, &xHigherPriorityTaskWoken);
            xSemaphoreGiveFromISR(g_pUARTSemaphore6, &xHigherPriorityTaskWoken);
            xSemaphoreGiveFromISR(g_pUARTSemaphore7, &xHigherPriorityTaskWoken);

            abortS1 = TRUE;
            abortS2 = TRUE;
            abortS3 = TRUE;
            abortS4 = TRUE;
            abortS5 = TRUE;
            abortS6 = TRUE;
            abortS7 = TRUE;
            ROM_IntDisable(INT_TIMER0A);
            ROM_TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
            ROM_TimerDisable(TIMER0_BASE, TIMER_A);
    }
    ROM_IntMasterEnable();

}
