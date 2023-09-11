/*******************************************************************************
  SYS USB CDC CONSOLE Support Layer

  File Name:
    sys_console_usb_cdc.c

  Summary:
    SYS USB CDC CONSOLE Support Layer

  Description:
    This file contains the SYS USB CDC CONSOLE support layer logic.
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

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    console_usb.c

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

#include "system/console/sys_console.h"
#include "driver/usb/drv_usb.h"
#include "sys_console_usb_cdc.h"
#include "configuration.h"

static CONS_USB_CDC_DEVICE gConsoleUSBCdcData;

/* Return the pointer to the USB CDC instance specific data */
#define CONSOLE_USB_CDC_GET_INSTANCE(index)    ((index) >= (SYS_CONSOLE_USB_CDC_MAX_INSTANCES))? (NULL) : (&gConsoleUSBCdcData.cdcInstance[index])

/* Expose the USB CDC console layer APIs to the Console System Service */
const SYS_CONSOLE_DEV_DESC sysConsoleUSBCdcDevDesc =
{
    .consoleDevice              = SYS_CONSOLE_DEV_USB_CDC,
    .intent                     = DRV_IO_INTENT_READWRITE,
    .init                       = Console_USB_CDC_Initialize,
    .read_t                       = Console_USB_CDC_Read,
    .readFreeBufferCountGet     = Console_USB_CDC_ReadFreeBufferCountGet,
    .readCountGet               = Console_USB_CDC_ReadCountGet,
    .write_t                      = Console_USB_CDC_Write,
    .writeFreeBufferCountGet    = Console_USB_CDC_WriteFreeBufferCountGet,
    .writeCountGet              = Console_USB_CDC_WriteCountGet,
    .task                       = Console_USB_CDC_Tasks,
    .status                     = Console_USB_CDC_Status,
    .flush                      = Console_USB_CDC_Flush,
};

// *****************************************************************************
// *****************************************************************************
// USB CDC Function Event Handler.
// *****************************************************************************
// *****************************************************************************

USB_DEVICE_CDC_EVENT_RESPONSE USBDeviceCDCEventHandler
(
    USB_DEVICE_CDC_INDEX index,
    USB_DEVICE_CDC_EVENT event,
    void* pData,
    uintptr_t userData
)
{
    CONS_USB_CDC_INSTANCE* cdcInstance;
    USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE* eventDataRead;
    USB_CDC_CONTROL_LINE_STATE* controlLineStateData;

    cdcInstance = (CONS_USB_CDC_INSTANCE *)userData;

    switch(event)
    {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */

            (void) USB_DEVICE_ControlSend(gConsoleUSBCdcData.deviceHandle,
                    &cdcInstance->getLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */

            (void) USB_DEVICE_ControlReceive(gConsoleUSBCdcData.deviceHandle,
                    &cdcInstance->setLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *)pData;

            if (controlLineStateData->dtr == 1U)
            {
                cdcInstance->isPortOpened = true;
            }
            else
            {
                cdcInstance->isPortOpened = false;
            }

            (void) USB_DEVICE_ControlStatus(gConsoleUSBCdcData.deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */

            /* Complete the control transfer by sending a ZLP  */
            (void) USB_DEVICE_ControlStatus(gConsoleUSBCdcData.deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data*/
            eventDataRead = (USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *)pData;
            cdcInstance->isReadComplete = true;
            cdcInstance->numBytesRead = eventDataRead->length;

            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data */

            (void) USB_DEVICE_ControlStatus(gConsoleUSBCdcData.deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We don't
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            /* This means that the data write got completed. We can schedule
             * the next read. */

            cdcInstance->isWriteComplete = true;
            break;

        default:
                /* Nothing to do */
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

// *****************************************************************************
// *****************************************************************************
// USB Device Layer Event Handler.
// *****************************************************************************
// *****************************************************************************

void USBDeviceEventHandler
(
    USB_DEVICE_EVENT event,
    void* eventData,
    uintptr_t context
)
{
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;
    CONS_USB_CDC_INSTANCE* cdcInstance;
    uint32_t i;

    switch(event)
    {
        case USB_DEVICE_EVENT_SOF:
            break;

        case USB_DEVICE_EVENT_RESET:

            for (i = 0; i < SYS_CONSOLE_USB_CDC_MAX_INSTANCES; i++)
            {
                cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(i);
                cdcInstance->isPortOpened = false;
            }

            gConsoleUSBCdcData.isConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*)eventData;

            if ( configuredEventData->configurationValue == 1U)
            {
                /* Register the CDC Device application event handler here.
                 * Note how the cdcInstance object pointer is passed as the
                 * user data */

                /* Mark that the device is now configured */
                gConsoleUSBCdcData.isConfigured = true;

                for (i = 0; i < SYS_CONSOLE_USB_CDC_MAX_INSTANCES; i++)
                {
                    cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(i);
                    (void) USB_DEVICE_CDC_EventHandlerSet(cdcInstance->cdcInstanceIndex, USBDeviceCDCEventHandler, (uintptr_t)cdcInstance);
                }
            }

            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(gConsoleUSBCdcData.deviceHandle);

            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(gConsoleUSBCdcData.deviceHandle);

            for (i = 0; i < SYS_CONSOLE_USB_CDC_MAX_INSTANCES; i++)
            {
                cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(i);
                cdcInstance->isPortOpened = false;
            }

            gConsoleUSBCdcData.isConfigured = false;

            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            break;

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
                  /* Nothing to do */
            break;
    }
}

static bool Console_USB_CDC_ResourceLock(CONS_USB_CDC_INSTANCE* cdcInstance)
{
    bool CheckLock = true;
    if(OSAL_MUTEX_Lock(&(cdcInstance->mutexTransferObjects), OSAL_WAIT_FOREVER) == OSAL_RESULT_FAIL)
    {
        CheckLock = false;
    }
    return CheckLock;
}

static void Console_USB_CDC_ResourceUnlock(CONS_USB_CDC_INSTANCE* cdcInstance)
{
    /* Release mutex */
    (void) OSAL_MUTEX_Unlock(&(cdcInstance->mutexTransferObjects));
}

static bool Console_USB_CDC_Reset(CONS_USB_CDC_INSTANCE* cdcInstance)
{
    /* This function returns true if the device was reset  */

    bool retVal;

    if(gConsoleUSBCdcData.isConfigured == false)
    {
        (void) Console_USB_CDC_ResourceLock(cdcInstance);

        cdcInstance->state = CONSOLE_USB_CDC_STATE_WAIT_FOR_CONFIGURATION;
        cdcInstance->readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        cdcInstance->writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        cdcInstance->isReadComplete = true;
        cdcInstance->isWriteComplete = true;

        Console_USB_CDC_ResourceUnlock(cdcInstance);

        retVal = true;
    }
    else
    {
        retVal = false;
    }

    return(retVal);
}

/* This function is only called by the USB Console Task */
static bool Console_USB_CDC_RxPushByte(uint32_t index, uint8_t rdByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    tempInIndex = cdcInstance->rdInIndex + 1U;

    if (tempInIndex >= cdcInstance->consoleReadBufferSize)
    {
        tempInIndex = 0;
    }
    if (tempInIndex != cdcInstance->rdOutIndex)
    {
        /* Copy the received character to the ring buffer */
        cdcInstance->consoleReadBuffer[cdcInstance->rdInIndex] = rdByte;
        cdcInstance->rdInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }

    return isSuccess;
}

/* This function is only called by the USB Console Task */
static bool Console_USB_CDC_ReadCompleteEventHandler(uint32_t index, uint32_t nBytesRead)
{
    uint32_t i;
    bool isSuccess = true;
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);
    USB_DEVICE_CDC_RESULT result;

    if (Console_USB_CDC_ResourceLock(cdcInstance) == true)
    {
        for (i = 0; i < nBytesRead; i++)
        {
            /* Copy data from the USB CDC buffer to the console ring buffer */
            if (Console_USB_CDC_RxPushByte(index, cdcInstance->cdcReadBuffer[i]) == false)
            {
                /* Ring buffer is full */
                break;
            }
        }

        cdcInstance->isReadComplete = false;

        cdcInstance->readTransferHandle =  USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

        result = USB_DEVICE_CDC_Read (cdcInstance->cdcInstanceIndex,
                &cdcInstance->readTransferHandle, cdcInstance->cdcReadBuffer,
                SYS_CONSOLE_USB_CDC_READ_WRITE_BUFFER_SIZE);

        if ((result != USB_DEVICE_CDC_RESULT_OK) || (cdcInstance->readTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID))
        {
            isSuccess = false;
        }

        Console_USB_CDC_ResourceUnlock(cdcInstance);
    }

    return isSuccess;
}

/* Read out the data from the RX Ring Buffer */
ssize_t Console_USB_CDC_Read(uint32_t index, void* pRdBuffer, size_t count)
{
    ssize_t nBytesRead = 0;
    uint8_t* pRdBuff = (uint8_t*)pRdBuffer;
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    if ((cdcInstance == NULL) || (pRdBuff == NULL))
    {
        return -1;
    }

    if (Console_USB_CDC_ResourceLock(cdcInstance) == false)
    {
        return -1;
    }

    while (nBytesRead < (ssize_t)count)
    {
        if (cdcInstance->rdOutIndex != cdcInstance->rdInIndex)
        {
            /* Copy data from the ring buffer to the application buffer */
            pRdBuff[nBytesRead] = cdcInstance->consoleReadBuffer[cdcInstance->rdOutIndex];
            nBytesRead++;
            cdcInstance->rdOutIndex++;

            if (cdcInstance->rdOutIndex >= cdcInstance->consoleReadBufferSize)
            {
                cdcInstance->rdOutIndex = 0;
            }
        }
        else
        {
            break;
        }
    }

    Console_USB_CDC_ResourceUnlock(cdcInstance);

    return nBytesRead;
}

/* Return number of unread bytes available in the RX Ring Buffer */
ssize_t Console_USB_CDC_ReadCountGet(uint32_t index)
{
    ssize_t nUnreadBytesAvailable = 0;
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);
    uint32_t temp;

    if (cdcInstance == NULL)
    {
        return -1;
    }

    if (Console_USB_CDC_ResourceLock(cdcInstance) == false)
    {
        return -1;
    }

    if ( cdcInstance->rdInIndex >=  cdcInstance->rdOutIndex)
    {
        temp = cdcInstance->rdInIndex -  cdcInstance->rdOutIndex;
        nUnreadBytesAvailable =  (ssize_t)temp;
    }
    else
    {
        temp = (cdcInstance->consoleReadBufferSize -  cdcInstance->rdOutIndex) + cdcInstance->rdInIndex;
        nUnreadBytesAvailable = (ssize_t)temp;
    }

    Console_USB_CDC_ResourceUnlock(cdcInstance);

    return nUnreadBytesAvailable;
}

/* Return free space available in the RX Ring Buffer */
ssize_t Console_USB_CDC_ReadFreeBufferCountGet(uint32_t index)
{
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);
    ssize_t nUnreadBytesAvailable = 0;

    if (cdcInstance == NULL)
    {
        return -1;
    }

    nUnreadBytesAvailable = Console_USB_CDC_ReadCountGet(index);

    if (nUnreadBytesAvailable >= 0)
    {
        nUnreadBytesAvailable = ((ssize_t)cdcInstance->consoleReadBufferSize - 1) - nUnreadBytesAvailable;
    }

    return nUnreadBytesAvailable;
}

/* This function is only called by the USB Console Task */
static bool Console_USB_CDC_TxPullByte(uint32_t index, uint8_t* pWrByte)
{
    bool isSuccess = false;
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    if (cdcInstance->wrOutIndex != cdcInstance->wrInIndex)
    {
        *pWrByte = cdcInstance->consoleWriteBuffer[cdcInstance->wrOutIndex];
        cdcInstance->wrOutIndex++;

        if (cdcInstance->wrOutIndex >= cdcInstance->consoleWriteBufferSize)
        {
            cdcInstance->wrOutIndex = 0;
        }
        isSuccess = true;
    }

    return isSuccess;
}

/* This function is called by the Console_USB_CDC_Write routine */
static bool Console_USB_CDC_TxPushByte(uint32_t index, uint8_t wrByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    tempInIndex = cdcInstance->wrInIndex + 1U;

    if (tempInIndex >= cdcInstance->consoleWriteBufferSize)
    {
        tempInIndex = 0;
    }
    if (tempInIndex != cdcInstance->wrOutIndex)
    {
        cdcInstance->consoleWriteBuffer[cdcInstance->wrInIndex] = wrByte;
        cdcInstance->wrInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }

    return isSuccess;
}

static ssize_t lConsole_USB_CDC_WriteCountGet(CONS_USB_CDC_INSTANCE* cdcInstance)
{
    ssize_t nPendingTxBytes = 0;
    uint32_t temp;

    if ( cdcInstance->wrInIndex >=  cdcInstance->wrOutIndex)
    {
        temp = cdcInstance->wrInIndex -  cdcInstance->wrOutIndex;
        nPendingTxBytes = (ssize_t)temp;
    }
    else
    {
        temp = (cdcInstance->consoleWriteBufferSize -  cdcInstance->wrOutIndex) + cdcInstance->wrInIndex;
        nPendingTxBytes =  (ssize_t)temp;
    }

    return nPendingTxBytes;
}

static bool Console_USB_CDC_WriteSubmit(uint32_t index)
{
    ssize_t nWriteBytesPending = 0;
    int32_t i;
    uint8_t wrByte;
    bool isSuccess = true;
    USB_DEVICE_CDC_RESULT result;

    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    nWriteBytesPending = lConsole_USB_CDC_WriteCountGet(cdcInstance);

    if (nWriteBytesPending > 0)
    {
        if (nWriteBytesPending > (SYS_CONSOLE_USB_CDC_READ_WRITE_BUFFER_SIZE))
        {
            nWriteBytesPending = (SYS_CONSOLE_USB_CDC_READ_WRITE_BUFFER_SIZE);
        }

        /* Get data from the ring buffer to the USB CDC buffer */
        for (i = 0; i < nWriteBytesPending; i++)
        {
            if (Console_USB_CDC_TxPullByte(index, &wrByte) == true)
            {
                cdcInstance->cdcWriteBuffer[i] = wrByte;
            }
        }

        cdcInstance->isWriteScheduled = true;

        result = USB_DEVICE_CDC_Write(cdcInstance->cdcInstanceIndex,
                &cdcInstance->writeTransferHandle, cdcInstance->cdcWriteBuffer, (size_t)nWriteBytesPending,
                USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);

        if ((result != USB_DEVICE_CDC_RESULT_OK) || (cdcInstance->writeTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID))
        {
            cdcInstance->isWriteScheduled = false;
            isSuccess = false;
        }
    }

    return isSuccess;
}

static bool Console_USB_CDC_WriteCompleteEventHandler(uint32_t index)
{
    bool isSuccess = true;

    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    if (Console_USB_CDC_ResourceLock(cdcInstance) == true)
    {
        /* isWriteScheduled = false; when no write is pending with (or submitted to) the the USB CDC
         * isWriteComplete = true; when a write submitted to the USB CDC is complete */
        if((cdcInstance->isWriteScheduled == false) || (cdcInstance->isWriteComplete == true))
        {
            if (cdcInstance->isWriteComplete == true)
            {
                cdcInstance->isWriteComplete = false;

                cdcInstance->isWriteScheduled = false;
            }

            isSuccess = Console_USB_CDC_WriteSubmit(index);
        }

        Console_USB_CDC_ResourceUnlock(cdcInstance);
    }

    return isSuccess;
}

ssize_t Console_USB_CDC_WriteCountGet(uint32_t index)
{
    ssize_t nPendingTxBytes = 0;

    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    if (cdcInstance == NULL)
    {
        return -1;
    }

    if (Console_USB_CDC_ResourceLock(cdcInstance) == false)
    {
        return -1;
    }

    nPendingTxBytes = lConsole_USB_CDC_WriteCountGet(cdcInstance);

    Console_USB_CDC_ResourceUnlock(cdcInstance);

    return nPendingTxBytes;
}

/* MISRA C-2012 Rule 11.8 deviated:1 Deviation record ID -  H3_MISRAC_2012_R_11_8_DR_1 */
ssize_t Console_USB_CDC_Write(uint32_t index, const void* pWrBuffer, size_t size )
{
    ssize_t nBytesWritten  = 0;
    uint8_t* pWrBuff = (uint8_t*)pWrBuffer;
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);

    if ((cdcInstance == NULL) || (pWrBuff == NULL) || (gConsoleUSBCdcData.isConfigured == false))
    {
        return -1;
    }

    if (Console_USB_CDC_ResourceLock(cdcInstance) == false)
    {
        return -1;
    }

    while (nBytesWritten < (ssize_t)size)
    {
        if (Console_USB_CDC_TxPushByte(index, pWrBuff[nBytesWritten]) == true)
        {
            nBytesWritten++;
        }
        else
        {
            /* Queue is full, exit the loop */
            break;
        }
    }

    /* isWriteScheduled = false; when no write is pending with (or submitted to) the the USB CDC
     * isWriteComplete = true; when a write submitted to the USB CDC is complete */

    if((cdcInstance->isWriteScheduled == false) || (cdcInstance->isWriteComplete == true))
    {
        if (cdcInstance->isWriteComplete == true)
        {
            cdcInstance->isWriteComplete = false;

            cdcInstance->isWriteScheduled = false;
        }

        (void) Console_USB_CDC_WriteSubmit(index);
    }

    Console_USB_CDC_ResourceUnlock(cdcInstance);

    return nBytesWritten;
}
/* MISRAC 2012 deviation block end */

ssize_t Console_USB_CDC_WriteFreeBufferCountGet(uint32_t index)
{
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);
    ssize_t nPendingTxBytes = 0;

    if (cdcInstance == NULL)
    {
        return -1;
    }

    nPendingTxBytes = Console_USB_CDC_WriteCountGet(index);

    if (nPendingTxBytes >= 0)
    {
        nPendingTxBytes = ((ssize_t)cdcInstance->consoleWriteBufferSize - 1) - nPendingTxBytes;
    }

    return nPendingTxBytes;
}

SYS_CONSOLE_STATUS Console_USB_CDC_Status(uint32_t index)
{
    SYS_CONSOLE_STATUS status = SYS_CONSOLE_STATUS_NOT_CONFIGURED;

    if (gConsoleUSBCdcData.isConfigured == true)
    {
        status = SYS_CONSOLE_STATUS_CONFIGURED;
    }

    return status;
}

bool Console_USB_CDC_Flush(uint32_t index)
{
    /* Data is not buffered, nothing to flush */
    return true;
}

void Console_USB_CDC_Initialize (uint32_t index, const void* initData)
{
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);
    const SYS_CONSOLE_USB_CDC_INIT_DATA* consoleUSBCdcInitData = (const SYS_CONSOLE_USB_CDC_INIT_DATA*)initData;

    if ((cdcInstance == NULL) || (consoleUSBCdcInitData == NULL))
    {
        return;
    }

    if(OSAL_MUTEX_Create(&(cdcInstance->mutexTransferObjects)) != OSAL_RESULT_SUCCESS)
    {
        return;
    }

    /* Initialize the common data variables */
    gConsoleUSBCdcData.deviceHandle     = USB_DEVICE_HANDLE_INVALID;
    gConsoleUSBCdcData.isConfigured     = false;

    /* Initialize the USB CDC instance specific data variables */
    cdcInstance->cdcInstanceIndex       = consoleUSBCdcInitData->cdcInstanceIndex;
    cdcInstance->state                  = CONSOLE_USB_CDC_STATE_INIT;
    cdcInstance->getLineCodingData.dwDTERate              = 9600;
    cdcInstance->getLineCodingData.bCharFormat            = 0;
    cdcInstance->getLineCodingData.bParityType            = 0;
    cdcInstance->getLineCodingData.bDataBits              = 8;
    cdcInstance->readTransferHandle     = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
    cdcInstance->writeTransferHandle    = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
    cdcInstance->isReadComplete         = true;
    cdcInstance->isWriteComplete        = true;
    cdcInstance->isPortOpened           = false;
    cdcInstance->cdcReadBuffer          = consoleUSBCdcInitData->cdcReadBuffer;
    cdcInstance->cdcWriteBuffer         = consoleUSBCdcInitData->cdcWriteBuffer;
    cdcInstance->consoleReadBufferSize  = consoleUSBCdcInitData->consoleReadBufferSize;
    cdcInstance->consoleWriteBufferSize = consoleUSBCdcInitData->consoleWriteBufferSize;
    cdcInstance->consoleReadBuffer      = consoleUSBCdcInitData->consoleReadBuffer;
    cdcInstance->consoleWriteBuffer     = consoleUSBCdcInitData->consoleWriteBuffer;
    cdcInstance->numBytesRead           = 0;
    cdcInstance->wrInIndex              = 0;
    cdcInstance->wrOutIndex             = 0;
    cdcInstance->isWriteScheduled       = 0;
    cdcInstance->rdInIndex              = 0;
    cdcInstance->rdOutIndex             = 0;
}
void Console_USB_CDC_Tasks(uint32_t index, SYS_MODULE_OBJ object)
{
    CONS_USB_CDC_INSTANCE* cdcInstance = CONSOLE_USB_CDC_GET_INSTANCE(index);
    USB_DEVICE_CDC_RESULT result;

    if (cdcInstance == NULL)
    {
        return;
    }

    switch(cdcInstance->state)
    {
        case CONSOLE_USB_CDC_STATE_INIT:

            /* Open the device layer */
            if (gConsoleUSBCdcData.deviceHandle == USB_DEVICE_HANDLE_INVALID)
            {
                if (USB_DEVICE_Status(USB_DEVICE_INDEX_0) == SYS_STATUS_READY)
                {
                    gConsoleUSBCdcData.deviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE );

                    if(gConsoleUSBCdcData.deviceHandle != USB_DEVICE_HANDLE_INVALID)
                    {
                        /* Register a callback with device layer to get event notification (for end point 0) */
                        USB_DEVICE_EventHandlerSet(gConsoleUSBCdcData.deviceHandle, USBDeviceEventHandler, 0);

                        cdcInstance->state = CONSOLE_USB_CDC_STATE_WAIT_FOR_CONFIGURATION;
                    }
                    else
                    {
                        /* The Device Layer is not ready to be opened. We should try
                         * again later. */
                    }
                }
            }
            else
            {
                cdcInstance->state = CONSOLE_USB_CDC_STATE_WAIT_FOR_CONFIGURATION;
            }

            break;

        case CONSOLE_USB_CDC_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if(gConsoleUSBCdcData.isConfigured)
            {
                /* Device is configured. Start reading. */
                cdcInstance->isReadComplete = false;

                result = USB_DEVICE_CDC_Read (cdcInstance->cdcInstanceIndex,
                    &cdcInstance->readTransferHandle, cdcInstance->cdcReadBuffer,
                    SYS_CONSOLE_USB_CDC_READ_WRITE_BUFFER_SIZE);

                if ((result == USB_DEVICE_CDC_RESULT_OK) && (cdcInstance->readTransferHandle !=     USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID))
                {
                    cdcInstance->state = CONSOLE_USB_CDC_STATE_SCHEDULE_READ_WRITE;
                }
                else
                {
                    cdcInstance->state = CONSOLE_USB_CDC_STATE_OPERATIONAL_ERROR;
                }
            }

            break;

        case CONSOLE_USB_CDC_STATE_SCHEDULE_READ_WRITE:

            if(Console_USB_CDC_Reset(cdcInstance))
            {
                break;
            }

            if(cdcInstance->isReadComplete == true)
            {
                if (Console_USB_CDC_ReadCompleteEventHandler(index, cdcInstance->numBytesRead) == false)
                {
                    cdcInstance->state = CONSOLE_USB_CDC_STATE_OPERATIONAL_ERROR;
                    break;
                }
            }

            if (Console_USB_CDC_WriteCompleteEventHandler(index) == false)
            {
                cdcInstance->state = CONSOLE_USB_CDC_STATE_OPERATIONAL_ERROR;
                break;
            }

            break;

        case CONSOLE_USB_CDC_STATE_OPERATIONAL_ERROR:
            /* Try again */
            (void) Console_USB_CDC_Reset(cdcInstance);
            cdcInstance->state = CONSOLE_USB_CDC_STATE_WAIT_FOR_CONFIGURATION;
            break;

        default:
               /* Nothing to do */
            break;
    }
}


/*******************************************************************************
 End of File
 */
