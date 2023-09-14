/*******************************************************************************
  AT25 Driver Definitions Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_at25_definitions.h

  Summary:
    AT25 Driver Definitions Header File

  Description:
    This file provides implementation-specific definitions for the AT25
    driver's system interface.
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

#ifndef DRV_AT25_DEFINITIONS_H
#define DRV_AT25_DEFINITIONS_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <device.h>
#include "system/ports/sys_ports.h"


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef void (* DRV_AT25_PLIB_CALLBACK)( uintptr_t context);

typedef bool (* DRV_AT25_PLIB_WRITE_READ)(void* pTransmitData, size_t txSize, void * pReceiveData, size_t rxSize);

typedef bool (* DRV_AT25_PLIB_WRITE)(void* pTransmitData, size_t txSize);

typedef bool (* DRV_AT25_PLIB_READ)(void* pReceiveData, size_t rxSize);

typedef bool (* DRV_AT25_PLIB_IS_BUSY)(void);

typedef void (* DRV_AT25_PLIB_CALLBACK_REGISTER)(DRV_AT25_PLIB_CALLBACK callBack, uintptr_t context);

// *****************************************************************************
/* AT25 Driver PLIB Interface Data

  Summary:
    Defines the data required to initialize the AT25 driver PLIB Interface.

  Description:
    This data type defines the data required to initialize the AT25 driver
    PLIB Interface.

  Remarks:
    None.
*/

typedef struct
{

    /* AT25 PLIB writeRead API */
    DRV_AT25_PLIB_WRITE_READ                writeRead;

    /* AT25 PLIB write API */
    DRV_AT25_PLIB_WRITE                     writeData;

    /* AT25 PLIB read API */
    DRV_AT25_PLIB_READ                      readData;

    /* AT25 PLIB Transfer status API */
    DRV_AT25_PLIB_IS_BUSY                   isBusy;

    /* AT25 PLIB callback register API */
    DRV_AT25_PLIB_CALLBACK_REGISTER         callbackRegister;

} DRV_AT25_PLIB_INTERFACE;

// *****************************************************************************
/* AT25 Driver Initialization Data

  Summary:
    Defines the data required to initialize the AT25 driver

  Description:
    This data type defines the data required to initialize or the AT25 driver.

  Remarks:
    None.
*/

typedef struct
{
    /* Identifies the PLIB API set to be used by the driver to access the
     * peripheral. */
    const DRV_AT25_PLIB_INTERFACE*      spiPlib;

    /* Number of clients */
    size_t                              numClients;

    SYS_PORT_PIN                        chipSelectPin;

    SYS_PORT_PIN                        holdPin;

    SYS_PORT_PIN                        writeProtectPin;

    /* Page size (in Bytes) of the EEPROM */
    uint32_t                            pageSize;

    /* Total size (in Bytes) of the EEPROM */
    uint32_t                            flashSize;

    uint32_t                            blockStartAddress;

} DRV_AT25_INIT;


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // #ifndef DRV_AT25_DEFINITIONS_H

/*******************************************************************************
 End of File
*/
