/*******************************************************************************
  Console System Service Local Data Structures

  Company:
    Microchip Technology Inc.

  File Name:
    sys_console_usb_cdc_definitions.h

  Summary:
    Console System Service USB CDC device implementation.

  Description:
    This file contains the definitions required by the USB CDC device.
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

#ifndef SYS_CONSOLE_USB_CDC_DEFINITIONS_H    /* Guard against multiple inclusion */
#define SYS_CONSOLE_USB_CDC_DEFINITIONS_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

typedef struct
{
    uint32_t		                    	cdcInstanceIndex;
	
	/* SYS Console read buffer size */
    uint32_t								consoleReadBufferSize;	
	
	/* SYS Console read buffer size */
	uint32_t								consoleWriteBufferSize;
	
	/* SYS Console USB CDC Read Buffer */
	uint8_t*								cdcReadBuffer;
	
	/* SYS Console USB CDC Write Buffer */
	uint8_t*								cdcWriteBuffer;
	
    /* SYS Console read buffer */
    uint8_t*                                consoleReadBuffer;

    /* SYS Console Write buffer */
    uint8_t*                                consoleWriteBuffer;

} SYS_CONSOLE_USB_CDC_INIT_DATA;

// DOM-IGNORE-BEGIN
#ifdef __cplusplus

}

#endif
// DOM-IGNORE-END

#endif /* SYS_CONSOLE_USB_CDC_DEFINITIONS_H */