/*******************************************************************************
  I2C Driver Local Data Structures

  Company:
    Microchip Technology Inc.

  File Name:
    drv_i2c_local.h

  Summary:
    I2C Driver Local Data Structures

  Description:
    Driver Local Data Structures
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

#ifndef _DRV_I2C_LOCAL_H
#define _DRV_I2C_LOCAL_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "osal/osal.h"


// *****************************************************************************
// *****************************************************************************
// Section: Data Type Definitions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
/* I2C Driver client Handle Macros

  Summary:
    I2C driver client Handle Macros

  Description:
    Client handle related utility macros. I2C client handle is equal
    to client token. The token is a 32 bit number that is incremented for
    every new driver open request.

  Remarks:
    None
*/

#define DRV_I2C_CLIENT_INDEX_MASK               (0x000000FF)
#define DRV_I2C_INSTANCE_INDEX_MASK             (0x0000FF00)
#define DRV_I2C_TOKEN_MASK                      (0xFFFF0000)
#define DRV_I2C_TOKEN_MAX                       (DRV_I2C_TOKEN_MASK >> 16)

// *****************************************************************************
/* I2C Transfer Object Flags

  Summary:
    Defines the I2C Transfer Object Flags.

  Description:
    This enumeration defines transfer type (read/write) of the I2C Buffer Object.

  Remarks:
    None.
*/

typedef enum
{
    /* Indicates this buffer was submitted by a read function */
    DRV_I2C_TRANSFER_OBJ_FLAG_READ = 1 << 0,

    /* Indicates this buffer was submitted by a write function */
    DRV_I2C_TRANSFER_OBJ_FLAG_WRITE = 1 << 1,

    /* Indicates this buffer was submitted by a write followed by read function */
    DRV_I2C_TRANSFER_OBJ_FLAG_WRITE_READ = 1 << 2,

    /* Indicates this buffer was submitted by a force write function */
    DRV_I2C_TRANSFER_OBJ_FLAG_WRITE_FORCED = 1 << 3,

} DRV_I2C_TRANSFER_OBJ_FLAGS;

// *****************************************************************************
/* I2C Transfer Status

  Summary:
    Defines the transfer status of the I2C request

  Description:
    This enumeration defines the status codes of the I2C request.

  Remarks:

*/

typedef enum
{
    /* All data was transferred successfully. */
    DRV_I2C_TRANSFER_STATUS_COMPLETE,

    /* There was an error while processing transfer request. */
    DRV_I2C_TRANSFER_STATUS_ERROR,

} DRV_I2C_TRANSFER_STATUS;

// *****************************************************************************
/* I2C Driver Instance Object

  Summary:
    Object used to keep any data required for an instance of the I2C driver.

  Description:
    None.

  Remarks:
    None.
*/

typedef struct
{
    /* Flag to indicate that driver has been opened Exclusively*/
    bool                            isExclusive;

    /* Keep track of the number of clients
      that have opened this driver */
    size_t                          nClients;

    /* Maximum number of clients */
    size_t                          nClientsMax;

    bool                            inUse;

    /* The status of the driver */
    SYS_STATUS                      status;

    /* PLIB API list that will be used by the driver to access the hardware */
    const DRV_I2C_PLIB_INTERFACE*   i2cPlib;

    /* Saves the initial value of the I2C clock speed which is assigned to a client when it opens the I2C driver */
    uint32_t                        initI2CClockSpeed;

    /* Current transfer setup will be used to verify change in the transfer setup by client */
    DRV_I2C_TRANSFER_SETUP          currentTransferSetup;

    /* Memory pool for Client Objects */
    uintptr_t                       clientObjPool;

    /* This is an instance specific token counter used to generate unique client
     * handles
     */
    uint16_t                        i2cTokenCount;

    /* The client of the active transfer on this driver instance */
    uintptr_t                       activeClient;

    /* Status of the active transfer */
    volatile DRV_I2C_TRANSFER_STATUS transferStatus;

    /* Mutex to protect access to the peripheral */
    OSAL_MUTEX_DECLARE(transferMutex);

    /* Mutex to protect access to the client object pool */
    OSAL_MUTEX_DECLARE(clientMutex);

    /* Semaphore to wait for transfer to complete. This is released from ISR*/
    OSAL_SEM_DECLARE(transferDone);

} DRV_I2C_OBJ;

// *****************************************************************************
/* I2C Driver Client Object

  Summary:
    Object used to track a single client.

  Description:
    This object is used to keep the data necesssary to keep track of a single
    client.

  Remarks:
    None.
*/

typedef struct
{
    /* The hardware instance object associated with the client */
    DRV_I2C_OBJ*                    hDriver;

    /* The IO intent with which the client was opened */
    DRV_IO_INTENT                   ioIntent;

    /* Errors associated with the I2C transfer */
    volatile DRV_I2C_ERROR          errors;

    /* This flags indicates if the object is in use or is
     * available */
    bool                            inUse;

    /* Client handle assigned to this client object when it was opened */
    DRV_HANDLE                      clientHandle;

    /* Client specific transfer setup */
    DRV_I2C_TRANSFER_SETUP          transferSetup;

} DRV_I2C_CLIENT_OBJ;

#endif //#ifndef _DRV_I2C_LOCAL_H
