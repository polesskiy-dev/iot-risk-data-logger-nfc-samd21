/*******************************************************************************
  FreeRTOS OSAL compatibility layer

  Company:
    Microchip Technology Inc.

  File Name:
    osal_freertos.c

  Summary:
    Provide OSAL mappings for the FreeRTOS(tm) Real-time operating system

  Description:
    This file contains functional implementations of the OSAL for FreeRTOS.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section lists the other files that are included in this file.
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "osal/osal_freertos.h"

// *****************************************************************************
// *****************************************************************************
// Section: OSAL Routines
// *****************************************************************************
// *****************************************************************************
/* These routines implement the OSAL for the chosen RTOS.
*/

// Critical Section group
// *****************************************************************************
/* Function: void OSAL_CRIT_Enter(OSAL_CRIT_TYPE severity)

  Summary:
    Enter a critical section with the specified severity level.

  Description:
     The program is entering a critical section of code. It is assumed that the
     sequence of operations bounded by the enter and leave critical section operations
     is treated as one atomic sequence that will not be disturbed.

  Precondition:
    None

  Parameters:
    severity      - OSAL_CRIT_TYPE_LOW, The RTOS should disable all other running
                    tasks effectively locking the scheduling mechanism.
                  - OSAL_CRIT_TYPE_HIGH, The RTOS should disable all possible
                    interrupts sources including the scheduler ensuring that the
                    sequence of code operates without interruption.

  Returns:
    None

  Example:
    <code>
     OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

     DRV_USART_Reinitialize( objUSART,  &initData);
     OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW);
    </code>

  Remarks:
    The sequence of operations bounded by the OSAL_CRIT_Enter and OSAL_CRIT_Leave
    form a critical section. The severity level defines whether the RTOS should
    perform task locking or completely disable all interrupts.

   NOTE -
    In FreeRTOS only interrupts below configMAX_SYSCALL_INTERRUPT_PRIORITY are
    disabled.  FreeRTOS will handle nesting of this function is scheduler is
    running.
 */
OSAL_CRITSECT_DATA_TYPE OSAL_CRIT_Enter(OSAL_CRIT_TYPE severity)
{
  switch (severity)
  {
    case OSAL_CRIT_TYPE_LOW:
      /* LOW priority critical sections just disable the scheduler */
      vTaskSuspendAll();
      break;

    case OSAL_CRIT_TYPE_HIGH:
      /* HIGH priority critical sections disable interrupts */
      portENTER_CRITICAL();
      break;
    default:
          /* Nothing to do */
      break;
  }

  return(0);
}

// *****************************************************************************
/* Function: void OSAL_CRIT_Leave(OSAL_CRIT_TYPE severity,OSAL_CRITSECT_DATA_TYPE status)

  Summary:
    Leave a critical section with the specified severity level.

  Description:
     The program is leaving a critical section of code. It is assumed that the
     sequence of operations bounded by the enter and leave critical section operations
     is treated as one atomic sequence that will not be disturbed.
    The severity should match the severity level used in the corresponding
    OSAL_CRIT_Enter call to ensure that the RTOS carries out the correct action.

  Precondition:
    None

  Parameters:
    severity      - OSAL_CRIT_TYPE_LOW, The RTOS should disable all other running
                    tasks effectively locking the scheduling mechanism.
                  - OSAL_CRIT_TYPE_HIGH, The RTOS should disable all possible
                    interrupts sources including the scheduler ensuring that the
                    sequence of code operates without interruption.

  Returns:
    None

  Example:
    <code>
     OSAL_CRITSECT_DATA_TYPE intStatus;

     intStatus = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_LOW);

     DRV_USART_Reinitialize( objUSART,  &initData);
     OSAL_CRIT_Leave(OSAL_CRIT_TYPE_LOW, intStatus);
    </code>

  Remarks:
    The sequence of operations bounded by the OSAL_CRIT_Enter and OSAL_CRIT_Leave
    form a critical section. The severity level defines whether the RTOS should
    perform task locking or completely disable all interrupts.

 */
void OSAL_CRIT_Leave(OSAL_CRIT_TYPE severity, OSAL_CRITSECT_DATA_TYPE status)
{
  switch (severity)
  {
    case OSAL_CRIT_TYPE_LOW:
      /* LOW priority resumes scheduler */
      (void) xTaskResumeAll();
      break;

    case OSAL_CRIT_TYPE_HIGH:
      /* HIGH priority renables interrupts */
      portEXIT_CRITICAL();
      break;
    default:
          /* Nothing to do */
      break;
  }
}

// Semaphore group
// *****************************************************************************
/* Function: OSAL_RESULT OSAL_SEM_Create(OSAL_SEM_HANDLE_TYPE* semID, OSAL_SEM_TYPE type,
                                uint8_t maxCount, uint8_t initialCount)
  Summary:
    Create an OSAL Semaphore

  Description:
    Create an OSAL binary or counting semaphore. If OSAL_SEM_TYPE_BINARY is specified then
    the initialCount must contain valid value and maxcount value is ignored otherwise this
    must contain valid value.

  Precondition:
    Semaphore must have been declared.

  Parameters:
    semID       - Pointer to the Semaphore ID

    type        - OSAL_SEM_TYPE_BINARY, create a binary semaphore
                - OSAL_SEM_TYPE_COUNTING, create a counting semaphore with the specified
                  count values.

    maxCount    - Maximum value for a counting semaphore. Ignored for a BINARY semaphore.

    initialCount - Starting count value for the semaphore.
                   For a BINARY semaphore if initialCount = 0 then Binary Semaphore is
                   created in a state such semaphore must call 'OSAL_SEM_Post' before it
                   can call 'OSAL_SEM_Pend' whereas if the initialCount = 1 then the first
                   call to 'OSAL_SEM_Pend' would pass.

  Returns:
    OSAL_RESULT_SUCCESS    - Semaphore created
    OSAL_RESULT_FAIL   - Semaphore creation failed

  Example:
    <code>
    OSAL_SEM_Create(&mySemID, OSAL_SEM_TYPE_COUNTING, 10, 5);
    </code>

  Remarks:
 */
/* MISRA C-2012 Rule 16.1, 16.3 deviated below. Deviation record ID -
   H3_MISRAC_2012_R_16_1_DR_1 & H3_MISRAC_2012_R_16_3_DR_1*/

OSAL_RESULT OSAL_SEM_Create(OSAL_SEM_HANDLE_TYPE* semID, OSAL_SEM_TYPE type, uint8_t maxCount, uint8_t initialCount)
{
  switch (type)
  {
    case OSAL_SEM_TYPE_BINARY:
      if ( initialCount <= 1U)
      {
        /* Binary semaphores created using xSemaphoreCreateBinary() are created in
         * a state such that the semaphore must first be 'given' before it can be
         * 'taken'. So call to OSAL_SEM_Create() with initialCount = 1 then
         * Binary semaphore must be created in a state such that the first call to
         * 'take' the semaphore would pass.
         */
        *(SemaphoreHandle_t*)semID = xSemaphoreCreateBinary();

        if ((*(SemaphoreHandle_t*)semID != NULL) && (initialCount == 1U))
        {
          if (xSemaphoreGive(*(SemaphoreHandle_t*)semID) == (int32_t)pdTRUE)
          {
            return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
          }
          else
          {
            return (OSAL_RESULT)OSAL_RESULT_FAIL;
          }
        }
      }
      else // for a Binary Semaphore initialCount must be either "0" or "1"
      {
        return (OSAL_RESULT)OSAL_RESULT_FAIL;
      }
    break;

    case OSAL_SEM_TYPE_COUNTING:
      *(SemaphoreHandle_t*)semID = xSemaphoreCreateCounting((UBaseType_t)maxCount, (UBaseType_t)initialCount);
    break;

    default:
      *(SemaphoreHandle_t*)semID = NULL;

    return (OSAL_RESULT)OSAL_RESULT_NOT_IMPLEMENTED;
  }

  if (*(SemaphoreHandle_t*)semID == NULL)
  {
    return (OSAL_RESULT)OSAL_RESULT_FAIL;
  }

  return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
}
/* MISRAC 2012 deviation block end */
// *****************************************************************************
/* Function: OSAL_RESULT OSAL_SEM_Delete(OSAL_SEM_HANDLE_TYPE* semID)

  Summary:
    Delete an OSAL Semaphore

  Description:
    Delete an OSAL semaphore freeing up any allocated storage associated with it.

  Precondition:
    Semaphore must have been created.

  Parameters:
    semID       - Pointer to the semID

  Returns:
    OSAL_RESULT_SUCCESS    - Semaphore deleted
    OSAL_RESULT_FAIL   - Semaphore deletion failed

  Example:
    <code>
     OSAL_SEM_Delete(&mySemID);
   </code>

  Remarks:
 */
OSAL_RESULT OSAL_SEM_Delete(OSAL_SEM_HANDLE_TYPE* semID)
{
  vSemaphoreDelete(*(SemaphoreHandle_t*)semID);
  *(SemaphoreHandle_t*)semID = NULL;

  return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
}

// *****************************************************************************
/* Function: OSAL_RESULT OSAL_SEM_Pend(OSAL_SEM_HANDLE_TYPE* semID, uint16_t waitMS)

  Summary:
     Pend on a semaphore. Returns true if semaphore obtained within time limit.

  Description:
     Blocking function call that pends (waits) on a semaphore. The function will
     return true is the semaphore has been obtained or false if it was not available
     or the time limit was exceeded.

  Precondition:
     Semaphore must have been created.

  Parameters:
     semID       - The semID

    waitMS       - Time limit to wait in milliseconds.
                   0 - do not wait
                   OSAL_WAIT_FOREVER - return only when semaphore is obtained
                   Other values - timeout delay

  Returns:
    OSAL_RESULT_SUCCESS    - Semaphore obtained
    OSAL_RESULT_FAIL   - Semaphore not obtained or timeout occurred

  Example:
    <code>
    if (OSAL_SEM_Pend(semUARTRX, 50) == OSAL_RESULT_SUCCESS)
    {

        c = DRV_USART_ReadByte(drvID);
        ...
    }
    else
    {

        ...
    }
   </code>

  Remarks:
 */
OSAL_RESULT OSAL_SEM_Pend(OSAL_SEM_HANDLE_TYPE* semID, uint16_t waitMS)
{
  TickType_t timeout = 0;

  if(waitMS == OSAL_WAIT_FOREVER)
  {
    timeout = portMAX_DELAY;
  }
  else
  {
    timeout = ((TickType_t)waitMS / portTICK_PERIOD_MS);
  }

  if (xSemaphoreTake(*(SemaphoreHandle_t*)semID, timeout) == (int32_t)pdTRUE)
  {
    return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
  }
  else
  {
    return (OSAL_RESULT)OSAL_RESULT_FAIL;
  }
}

// *****************************************************************************
/* Function: OSAL_RESULT OSAL_SEM_Post(OSAL_SEM_HANDLE_TYPE* semID)

  Summary:
     Post a semaphore or increment a counting semaphore.

  Description:
     Post a binary semaphore or increment a counting semaphore. The highest
     priority task currently blocked on the semaphore will be released and
     made ready to run.

  Precondition:
     Semaphore must have been created.

  Parameters:
     semID       - The semID

  Returns:
    OSAL_RESULT_SUCCESS    - Semaphore posted
    OSAL_RESULT_FAIL   - Semaphore not posted

  Example:
    <code>
    OSAL_SEM_Post(semSignal);
    </code>

  Remarks:
 */
OSAL_RESULT OSAL_SEM_Post(OSAL_SEM_HANDLE_TYPE* semID)
{
  if (xSemaphoreGive(*(SemaphoreHandle_t*)semID) == (int32_t)pdTRUE)
  {
    return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
  }

  return (OSAL_RESULT)OSAL_RESULT_FAIL;
}

// *****************************************************************************
/* MISRA C-2012 Rule 15.6, 14.4,20.7 deviated below. Deviation record ID -
   H3_MISRAC_2012_R_14_4_DR_1, H3_MISRAC_2012_R_15_6_DR_1 & H3_MISRAC_2012_R_20_7_DR_1*/
/* Function: OSAL_RESULT OSAL_SEM_PostISR(OSAL_SEM_HANDLE_TYPE* semID)

  Summary:
     Post a semaphore or increment a counting semaphore from within an ISR

  Description:
     Post a binary semaphore or increment a counting semaphore. The highest
     priority task currently blocked on the semaphore will be released and
     made ready to run. This form of the post function should be used inside
     an ISR.

  Precondition:
     Semaphore must have been created.

  Parameters:
     semID       - The semID

  Returns:
    OSAL_RESULT_SUCCESS    - Semaphore posted
    OSAL_RESULT_FAIL   - Semaphore not posted

  Example:
    <code>
     void __ISR(UART_2_VECTOR) _UART2RXHandler()
     {
        char c;

        c = U2RXREG;

        IFS1bits.U2IF = 0;

        OSAL_SEM_PostISR(semSignal);
     }
    </code>

  Remarks:
     This version of the OSAL_SEM_Post function should be used if the program
     is, or may be, operating inside and ISR. The OSAL will take the necessary
     steps to ensure correct operation possibly disabling interrupts or entering
     a critical section. The exact requirements will depend upon the particular
     RTOS being used.
 */
OSAL_RESULT OSAL_SEM_PostISR(OSAL_SEM_HANDLE_TYPE* semID)
{
  BaseType_t taskWoken = (int32_t)pdFALSE;

  if ((xSemaphoreGiveFromISR(*(SemaphoreHandle_t*)semID, &taskWoken)) != 0)
  {
    portEND_SWITCHING_ISR(taskWoken);
    return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
  }

  return (OSAL_RESULT)OSAL_RESULT_FAIL;
}

/* MISRAC 2012 deviation block end */
// *****************************************************************************
/* Function: uint8_t OSAL_SEM_GetCount(OSAL_SEM_HANDLE_TYPE* semID)

  Summary:
    Return the current value of a counting semaphore.

  Description:
    Return the current value of a counting semaphore. The value returned is
    assumed to be a single value ranging from 0-255.

  Precondition:
     Semaphore must have been created.

  Parameters:
     semID       - The semID

  Returns:
    0           - Semaphore is unavailable
    1-255       - Current value of the counting semaphore

  Example:
    <code>
     uint8_t semCount;

     semCount = OSAL_SEM_GetCount(semUART);

     if (semCount > 0)
     {

         if (OSAL_SEM_Pend(semUART) == OSAL_RESULT_SUCCESS)
         {

            ...
         }
     }
     else
     {

        ...
     }
    </code>

  Remarks:
     This version of the OSAL_SEM_Post function should be used if the program
     is, or may be, operating inside and ISR. The OSAL will take the necessary
     steps to ensure correct operation possibly disabling interrupts or entering
     a critical section. The exact requirements will depend upon the particular
     RTOS being used.
 */
uint8_t OSAL_SEM_GetCount(OSAL_SEM_HANDLE_TYPE* semID)
{
  UBaseType_t SemCount;
  SemCount = uxQueueMessagesWaiting(*(SemaphoreHandle_t*)semID);

  if(SemCount > 255U)
  {
    SemCount = 255;
  }

  return (uint8_t)SemCount;
}

// *****************************************************************************
/* Function: OSAL_RESULT OSAL_MUTEX_Create(OSAL_MUTEX_HANDLE_TYPE* mutexID)

  Summary:
    Create a mutex.

  Description:
    This function creates a mutex, allocating storage if required and placing
    the mutex handle into the passed parameter.

  Precondition:
    None.

  Parameters:
    mutexID      - Pointer to the mutex handle

  Returns:
    OSAL_RESULT_SUCCESS    - Mutex successfully created

    OSAL_RESULT_FAIL   - Mutex failed to be created.

  Example:
    <code>
    OSAL_MUTEX_HANDLE_TYPE mutexData;

    OSAL_MUTEX_Create(&mutexData);
    ...
     if (OSAL_MUTEX_Lock(mutexData, 1000) == OSAL_RESULT_SUCCESS)
     {

        ...
     }
    </code>

  Remarks:

 */
OSAL_RESULT OSAL_MUTEX_Create(OSAL_MUTEX_HANDLE_TYPE* mutexID)
{
  /* mutex may already have been created so test before creating it */
  if (*(SemaphoreHandle_t*)mutexID != NULL)
  {
    return (OSAL_RESULT)OSAL_RESULT_FAIL;
  }

  *(SemaphoreHandle_t*)mutexID = xSemaphoreCreateMutex();

  return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
}

// *****************************************************************************
/* Function: OSAL_RESULT OSAL_MUTEX_Delete(OSAL_MUTEX_HANDLE_TYPE* mutexID)

  Summary:
    Delete a mutex.

  Description:
    This function deletes a mutex and frees associated storage if required.

  Precondition:
    None.

  Parameters:
    mutexID      - Pointer to the mutex handle

  Returns:
    OSAL_RESULT_SUCCESS    - Mutex successfully deleted.

    OSAL_RESULT_FAIL   - Mutex failed to be deleted.

  Example:
    <code>
    OSAL_MUTEX_Delete(mutexData);
    </code>

  Remarks:

 */
OSAL_RESULT OSAL_MUTEX_Delete(OSAL_MUTEX_HANDLE_TYPE* mutexID)
{
  if(*(SemaphoreHandle_t*)mutexID == NULL)
  {
    return (OSAL_RESULT)OSAL_RESULT_FAIL;
  }

  vSemaphoreDelete(*(SemaphoreHandle_t*)mutexID);
  *(SemaphoreHandle_t*)mutexID = NULL;

  return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
}

// *****************************************************************************
/* Function: OSAL_RESULT OSAL_MUTEX_Lock(OSAL_MUTEX_HANDLE_TYPE* mutexID, uint16_t waitMS)

  Summary:
    Lock a mutex.

  Description:
    This function locks a mutex, waiting for the specified timeout. If it cannot
    be obtained or the timeout period elapses then false is returned;

  Precondition:
    None.

  Parameters:
    mutexID      - The mutex handle

    waitMS       - Timeout value in milliseconds,
                   0 - do not wait, return immediately
                   OSAL_WAIT_FOREVER - wait until mutex is obtained before returning
                   Other values - Timeout delay

  Returns:
    OSAL_RESULT_SUCCESS    - Mutex successfully obtained.

    OSAL_RESULT_FAIL   - Mutex failed to be obtained or timeout occurred.

  Example:
    <code>
    OSAL_MUTEX_HANDLE_TYPE* mutexData;

    OSAL_MUTEX_Create(&mutexData);
    ...
     if (OSAL_MUTEX_Lock(mutexData, 1000) == OSAL_RESULT_SUCCESS)
     {

        ...


        OSAL_MUTEX_Unlock(mutexData);
     }
    </code>

  Remarks:

 */
OSAL_RESULT OSAL_MUTEX_Lock(OSAL_MUTEX_HANDLE_TYPE* mutexID, uint16_t waitMS)
{
  TickType_t timeout = 0;

  if(waitMS == OSAL_WAIT_FOREVER)
  {
    timeout = portMAX_DELAY;
  }
  else
  {
    timeout = ((TickType_t)waitMS / portTICK_PERIOD_MS);
  }

  if (xSemaphoreTake(*(SemaphoreHandle_t*)mutexID, timeout) == (int32_t)pdTRUE)
  {
    return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
  }
  else
  {
    return (OSAL_RESULT)OSAL_RESULT_FAIL;
  }
}

// *****************************************************************************
/* Function: OSAL_RESULT OSAL_MUTEX_Unlock(OSAL_MUTEX_HANDLE_TYPE* mutexID)

  Summary:
    Unlock a mutex.

  Description:
    This function unlocks a previously obtained mutex.

  Precondition:
    None.

  Parameters:
    mutexID      - The mutex handle

  Returns:
    OSAL_RESULT_SUCCESS    - Mutex released.

    OSAL_RESULT_FAIL   - Mutex failed to be released or error occurred.

  Example:
    <code>
    OSAL_MUTEX_HANDLE_TYPE* mutexData;

    OSAL_MUTEX_Create(&mutexData);
    ...
     if (OSAL_MUTEX_Lock(mutexData, 1000) == OSAL_RESULT_SUCCESS)
     {

        ...


        OSAL_MUTEX_Unlock(mutexData);
     }
    </code>

  Remarks:

 */
OSAL_RESULT OSAL_MUTEX_Unlock(OSAL_MUTEX_HANDLE_TYPE* mutexID)
{
  if (xSemaphoreGive(*(SemaphoreHandle_t*)mutexID) == (int32_t)pdTRUE)
  {
    return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
  }

  return (OSAL_RESULT)OSAL_RESULT_FAIL;
}

// *****************************************************************************
/* Function:
    void* OSAL_Malloc(size_t size)

  Summary:
    Allocates memory using the OSAL default allocator.

  Description:
     This function allocates a block of memory from the default allocator from
     the underlying RTOS. If no RTOS is present, it defaults to malloc.
     Many operating systems incorporate their own memory allocation scheme, using
     pools, blocks or by wrapping the standard C library functions in a critical
     section. Since an Harmony application may not know what target OS is being used
     (if any), this function ensures that the correct thread safe memory
     allocator will be used.

  Precondition:
    None.

  Parameters:
    size      - Size of the requested memory block in bytes

  Returns:
     Pointer to the block of allocated memory. NULL is returned if memory could
     not be allocated.

  Example:
    <code>

    uint8_t* pData;

     pData = OSAL_Malloc(32);
     if (pData != NULL)
     {
        ...
     }
    </code>

  Remarks:
    None.

 */
void* OSAL_Malloc(size_t size)
{
    return pvPortMalloc(size);
}

// *****************************************************************************
/* Function:
    void OSAL_Free(void* pData)

  Summary:
    Deallocates a block of memory and return to the default pool.

  Description:
     This function deallocates memory and returns it to the default pool.
     In an RTOS-based application, the memory may have been allocated from
     multiple pools or simply from the heap.
     In non-RTOS applications, this function calls the C standard function
     free.

  Precondition:
    None.

  Parameters:
    pData      - Pointer to the memory block to be set free

  Returns:
    None.

  Example:
    <code>

    uint8_t* pData;

     pData = OSAL_Malloc(32);
     if (pData != NULL)
     {
        ...


        OSAL_Free(pData);

        pData = NULL;
     }
    </code>

  Remarks:
    None.

 */
void OSAL_Free(void* pData)
{
    vPortFree(pData);
}

// *****************************************************************************
/* Function: OSAL_RESULT OSAL_Initialize(void)

  Summary:
    Perform OSAL initialization.

  Description:
     This function should be called near the start of main in an application
     that will use an underlying RTOS. This permits the RTOS to perform
     any one time initialization before the application attempts to create
     drivers or other items that may use the RTOS. Typical actions performed by
     OSAL_Initialize would be to allocate and prepare any memory pools for
     later use.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    OSAL_RESULT_SUCCESS  - Initialization completed successfully.

  Example:
    <code>
     int main()
     {
         OSAL_Initialize();

         App_Init();
         OSAL_Start();
     }
    </code>

  Remarks:
 */
OSAL_RESULT OSAL_Initialize(void)
{
  // nothing required
  return (OSAL_RESULT)OSAL_RESULT_SUCCESS;
}

/*******************************************************************************
 End of File
*/

