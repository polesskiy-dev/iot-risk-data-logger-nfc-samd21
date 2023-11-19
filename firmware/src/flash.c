/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    flash.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "flash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "time.h"
#include "definitions.h"
#include "config/default/peripheral/rtc/plib_rtc.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the FLASH_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

FLASH_DATA flashData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void FLASH_Initialize ( void )

  Remarks:
    See prototype in flash.h.
 */
/* Binary Semaphore: Create a global binary semaphore. 
 * This semaphore will be taken (waited on) by the LED task and given (signaled) by the RTC alarm callback. */
SemaphoreHandle_t xRTCSemaphore = NULL;

struct tm alarmTime; 
struct tm currentTime; // TODO check can we make it local

void vDummySendLEDEventCb (RTC_CLOCK_INT_MASK event, uintptr_t context )
{
    if((event & RTC_CLOCK_INT_MASK_YEAR_OVERFLOW) ==
    RTC_CLOCK_INT_MASK_YEAR_OVERFLOW)
    {
        // This means a year overflow has occurred.
    }
    else if ((event & RTC_CLOCK_INT_MASK_ALARM) == RTC_CLOCK_INT_MASK_ALARM)
    {
        xSemaphoreGiveFromISR(xRTCSemaphore, NULL); // Signal the LED task
    }
}

void FLASH_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    flashData.state = FLASH_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    
    /* Create the semaphore before starting the scheduler */
    xRTCSemaphore = xSemaphoreCreateBinary();

    
    // TODO move RTC init to a separate module
    // init RTC Alarm
    alarmTime.tm_sec = 1;
    RTC_ALARM_MASK mask = RTC_ALARM_MASK_SS; // alarm every minute
    RTC_RTCCTimeSet(&currentTime); // time should be parsed from __DATE__, __TIME__
    
    // The mask is specified to match all time field and ignore all date fields.
    if(RTC_RTCCAlarmSet(&alarmTime, mask) == false)
    {
        //incorrect format
    }
    
    RTC_RTCCCallbackRegister(vDummySendLEDEventCb, (uintptr_t)NULL); // TODO pass appropriate context instead of NULL
}


/******************************************************************************
  Function:
    void FLASH_Tasks ( void )

  Remarks:
    See prototype in flash.h.
 */

void FLASH_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( flashData.state )
    {
        /* Application's initial state. */
        case FLASH_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {

                flashData.state = FLASH_STATE_SERVICE_TASKS;
            }
            break;
        }

        case FLASH_STATE_SERVICE_TASKS:
        {
            if(xSemaphoreTake(xRTCSemaphore, portMAX_DELAY))
            {
                // Toggle LED
                _LED_Toggle();
                /*
                 * To Check Heap: 
                 * For now it's free 4912 bytes with only USB and LED
                 */
                volatile size_t heapConsumption = xPortGetFreeHeapSize();
                (void)heapConsumption;
                __NOP();
                
            }
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
