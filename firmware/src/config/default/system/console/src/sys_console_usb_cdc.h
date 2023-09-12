/*******************************************************************************
  Console System Service Local Data Structures

  Company:
    Microchip Technology Inc.

  File Name:
    sys_console_usb_cdc.h

  Summary:
    Console System Service local declarations and definitions for USB CDC I/O
    device.

  Description:
    This file contains the Console System Service local declarations and
    definitions for USB CDC I/O device.
*******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END

#ifndef SYS_CONSOLE_USB_CDC_H
#define SYS_CONSOLE_USB_CDC_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "sys_console_local.h"
#include "osal/osal.h"
#include "configuration.h"
#include "usb/usb_device_cdc.h"
#include "system/console/src/sys_console_usb_cdc_definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************
typedef void (* CONSOLE_USB_CDC_CALLBACK)( uintptr_t context );

typedef enum
{
    /* Application opens and attaches the device here */
    CONSOLE_USB_CDC_STATE_INIT = 0,

    /* Application waits for device configuration*/
    CONSOLE_USB_CDC_STATE_WAIT_FOR_CONFIGURATION,

    /* Schedule Read/Write operation */
    CONSOLE_USB_CDC_STATE_SCHEDULE_READ_WRITE,

    /* Application Non-Recoverable Error state*/
    CONSOLE_USB_CDC_STATE_CRITICAL_ERROR,

    /* Application Recoverable Error state*/
    CONSOLE_USB_CDC_STATE_OPERATIONAL_ERROR

} CONSOLE_USB_CDC_STATE;

typedef struct
{
    /* Application's current state*/
    USB_DEVICE_CDC_INDEX                    cdcInstanceIndex;

    CONSOLE_USB_CDC_STATE                   state;

    /* Set Line Coding Data */
    USB_CDC_LINE_CODING                     setLineCodingData;

    /* Get Line Coding Data */
    USB_CDC_LINE_CODING                     getLineCodingData;

    /* Read transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE          readTransferHandle;

    /* Write transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE          writeTransferHandle;

    /* True if a character was read */
    volatile bool                           isReadComplete;

    /* True if a character was written*/
    volatile bool                           isWriteComplete;

    /* Number of bytes read from Host */
    volatile uint32_t                       numBytesRead;

    /* USB CDC read buffer */
    uint8_t*                                cdcReadBuffer;

    /* USB CDC Write buffer */
    uint8_t*                                cdcWriteBuffer;

    /* SYS Console read buffer */
    uint8_t*                                consoleReadBuffer;

    /* SYS Console Write buffer */
    uint8_t*                                consoleWriteBuffer;

    /* SYS Console read buffer size */
    uint32_t                                consoleReadBufferSize;

    /* SYS Console write buffer size */
    uint32_t                                consoleWriteBufferSize;

    uint32_t                                wrInIndex;

    uint32_t                                wrOutIndex;

    uint32_t                                rdInIndex;

    uint32_t                                rdOutIndex;

    bool                                    isWriteScheduled;
	
	bool									isPortOpened;

    /* Mutex to protect access to the shared variables from multiple threads */
    OSAL_MUTEX_DECLARE(mutexTransferObjects);

}CONS_USB_CDC_INSTANCE;

typedef struct
{
    /* Device layer handle returned by device layer open function */
    USB_DEVICE_HANDLE                       deviceHandle;

    /* Device configured state */
    bool                                    isConfigured;

    CONS_USB_CDC_INSTANCE                   cdcInstance[SYS_CONSOLE_USB_CDC_MAX_INSTANCES];

} CONS_USB_CDC_DEVICE;

void Console_USB_CDC_Initialize (uint32_t index, const void* initData);
SYS_CONSOLE_STATUS Console_USB_CDC_Status(uint32_t index);
void Console_USB_CDC_Tasks(uint32_t index, SYS_MODULE_OBJ object);
ssize_t Console_USB_CDC_Read(uint32_t index, void* pRdBuffer, size_t count);
ssize_t Console_USB_CDC_ReadFreeBufferCountGet(uint32_t index);
ssize_t Console_USB_CDC_ReadCountGet(uint32_t index);
ssize_t Console_USB_CDC_Write(uint32_t index, const void* pWrBuffer, size_t size );
ssize_t Console_USB_CDC_WriteFreeBufferCountGet(uint32_t index);
ssize_t Console_USB_CDC_WriteCountGet(uint32_t index);
bool Console_USB_CDC_Flush(uint32_t index);



// DOM-IGNORE-BEGIN
#ifdef __cplusplus

    }

#endif
// DOM-IGNORE-END

#endif //#ifndef SYS_CONSOLE_UART_H