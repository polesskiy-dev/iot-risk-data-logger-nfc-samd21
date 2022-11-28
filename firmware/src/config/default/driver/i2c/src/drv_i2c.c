/*******************************************************************************
  I2C Driver Implementation.

  Company:
    Microchip Technology Inc.

  File Name:
    drv_i2c.c

  Summary:
    Source code for the I2C driver dynamic implementation.

  Description:
    This file contains the source code for the dynamic implementation of the
    I2C driver.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "driver/i2c/drv_i2c.h"
#include "system/debug/sys_debug.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
/* This is the driver instance object array. */
static DRV_I2C_OBJ gDrvI2CObj[DRV_I2C_INSTANCES_NUMBER] ;

// *****************************************************************************
// *****************************************************************************
// Section: File scope functions
// *****************************************************************************
// *****************************************************************************

static inline uint32_t  _DRV_I2C_MAKE_HANDLE(uint16_t token, uint8_t drvIndex, uint8_t clientIndex)
{
    return ((token << 16) | (drvIndex << 8) | clientIndex);
}

static inline uint16_t _DRV_I2C_UPDATE_TOKEN(uint16_t token)
{
    token++;
    if (token >= DRV_I2C_TOKEN_MAX)
    {
        token = 1;
    }

    return token;
}

static DRV_I2C_CLIENT_OBJ* _DRV_I2C_DriverHandleValidate(DRV_HANDLE handle)
{
    uint32_t drvInstance = 0;
    DRV_I2C_CLIENT_OBJ* client = (DRV_I2C_CLIENT_OBJ*)NULL;

    /* This function returns the pointer to the client object that is
       associated with this handle if the handle is valid. Returns NULL
       otherwise.
    */

    if((handle != DRV_HANDLE_INVALID) && (handle != 0))
    {
        /* Extract the instance value from the handle */
        drvInstance = ((handle & DRV_I2C_INSTANCE_INDEX_MASK) >> 8);

        if (drvInstance >= DRV_I2C_INSTANCES_NUMBER)
        {
            return (NULL);
        }
        if ((handle & DRV_I2C_CLIENT_INDEX_MASK) >= gDrvI2CObj[drvInstance].nClientsMax)
        {
            return (NULL);
        }

        client = &((DRV_I2C_CLIENT_OBJ *)gDrvI2CObj[drvInstance].clientObjPool)[handle & DRV_I2C_CLIENT_INDEX_MASK];

        if ((client->clientHandle != handle) || (client->inUse == false))
        {
            return (NULL);
        }
    }
    return(client);
}

static void _DRV_I2C_PLibCallbackHandler( uintptr_t contextHandle )
{
    DRV_I2C_OBJ* dObj = (DRV_I2C_OBJ *)contextHandle;
    DRV_I2C_CLIENT_OBJ* clientObj = (DRV_I2C_CLIENT_OBJ*)NULL;

    clientObj = (DRV_I2C_CLIENT_OBJ*)dObj->activeClient;

    /* Update error into the client object*/
    clientObj->errors = dObj->i2cPlib->errorGet();

    if(clientObj->errors == DRV_I2C_ERROR_NONE)
    {
        dObj->transferStatus = DRV_I2C_TRANSFER_STATUS_COMPLETE;
    }
    else
    {
        dObj->transferStatus = DRV_I2C_TRANSFER_STATUS_ERROR;
    }

    /* Unblock the application thread */
    OSAL_SEM_PostISR( &dObj->transferDone);
}

SYS_MODULE_OBJ DRV_I2C_Initialize( const SYS_MODULE_INDEX drvIndex, const SYS_MODULE_INIT * const init )
{
    DRV_I2C_OBJ* dObj     = (DRV_I2C_OBJ*)NULL;
    DRV_I2C_INIT* i2cInit = (DRV_I2C_INIT*)init;

    /* Validate the request */
    if(drvIndex >= DRV_I2C_INSTANCES_NUMBER)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid driver instance");
        return SYS_MODULE_OBJ_INVALID;
    }

    if(gDrvI2CObj[drvIndex].status == SYS_STATUS_READY)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Instance already initialized");
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Allocate the driver object */
    dObj = &gDrvI2CObj[drvIndex];
    dObj->inUse = true;

    /* Update the driver parameters */
    dObj->i2cPlib                           = i2cInit->i2cPlib;
    dObj->clientObjPool                     = i2cInit->clientObjPool;
    dObj->nClientsMax                       = i2cInit->numClients;
    dObj->nClients                          = 0;
    dObj->activeClient                      = (uintptr_t)NULL;
    dObj->i2cTokenCount                     = 1;
    dObj->isExclusive                       = false;
    dObj->initI2CClockSpeed                 = i2cInit->clockSpeed;
    dObj->currentTransferSetup.clockSpeed   = i2cInit->clockSpeed;

    if (OSAL_MUTEX_Create(&dObj->clientMutex) == OSAL_RESULT_FALSE)
    {
        /*  If the mutex was not created because the memory required to
            hold the mutex could not be allocated then NULL is returned. */
        return SYS_MODULE_OBJ_INVALID;
    }

    if (OSAL_MUTEX_Create(&dObj->transferMutex) == OSAL_RESULT_FALSE)
    {
        /*  If the mutex was not created because the memory required to
            hold the mutex could not be allocated then NULL is returned. */
        return SYS_MODULE_OBJ_INVALID;
    }
    if (OSAL_SEM_Create(&dObj->transferDone,OSAL_SEM_TYPE_BINARY, 0, 0) == OSAL_RESULT_FALSE)
    {
        /* There was insufficient heap memory available for the semaphore to
        be created successfully. */
        return SYS_MODULE_OBJ_INVALID;
    }

    /* Register a callback with PLIB.
     * dObj as a context parameter will be used to distinguish the events
     * from different instances. */
    dObj->i2cPlib->callbackRegister(_DRV_I2C_PLibCallbackHandler, (uintptr_t)dObj);

    /* Update the status */
    dObj->status = SYS_STATUS_READY;

    /* Return the object structure */
    return ( (SYS_MODULE_OBJ)drvIndex );
}

SYS_STATUS DRV_I2C_Status( const SYS_MODULE_OBJ object)
{
    /* Validate the request */
    if((object == SYS_MODULE_OBJ_INVALID) || (object >= DRV_I2C_INSTANCES_NUMBER))
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid system object handle");
        return SYS_STATUS_UNINITIALIZED;
    }

    return (gDrvI2CObj[object].status);
}

bool DRV_I2C_TransferSetup( const DRV_HANDLE handle, DRV_I2C_TRANSFER_SETUP* setup )
{
    DRV_I2C_CLIENT_OBJ* clientObj = NULL;

    if(setup == NULL)
    {
        return false;
    }

    /* Validate the driver handle */
    clientObj = _DRV_I2C_DriverHandleValidate(handle);

    if(clientObj == NULL)
    {
        return false;
    }

    /* Save the client specific transfer setup */
    clientObj->transferSetup = *setup;

    return true;
}

DRV_I2C_ERROR DRV_I2C_ErrorGet( const DRV_HANDLE handle )
{
    DRV_I2C_CLIENT_OBJ* clientObj = NULL;
    DRV_I2C_ERROR errors = DRV_I2C_ERROR_NONE;

    /* Validate the handle */
    clientObj = _DRV_I2C_DriverHandleValidate(handle);

    if(clientObj != NULL)
    {
        errors = clientObj->errors;
    }

    return errors;
}

DRV_HANDLE DRV_I2C_Open( const SYS_MODULE_INDEX drvIndex, const DRV_IO_INTENT ioIntent )
{
    DRV_I2C_CLIENT_OBJ* clientObj = NULL;
    DRV_I2C_OBJ* dObj = NULL;
    uint32_t iClient;

    /* Validate the request */
    if (drvIndex >= DRV_I2C_INSTANCES_NUMBER)
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Invalid Driver Instance");
        return DRV_HANDLE_INVALID;
    }

    dObj = &gDrvI2CObj[drvIndex];

    if((dObj->status != SYS_STATUS_READY) || (dObj->inUse == false))
    {
        SYS_DEBUG_MESSAGE(SYS_ERROR_ERROR, "Was the driver initialized?");
        return DRV_HANDLE_INVALID;
    }

    /* Acquire the instance specific mutex to protect the instance specific
     * client pool
     */
    if (OSAL_MUTEX_Lock(&dObj->clientMutex , OSAL_WAIT_FOREVER ) == OSAL_RESULT_FALSE)
    {
        return DRV_HANDLE_INVALID;
    }

    if(dObj->isExclusive)
    {
        /* This means the another client has opened the driver in exclusive
           mode. The driver cannot be opened again
        */
        OSAL_MUTEX_Unlock( &dObj->clientMutex);
        return DRV_HANDLE_INVALID;
    }

    if((dObj->nClients > 0) && (ioIntent & DRV_IO_INTENT_EXCLUSIVE))
    {
        /* This means the driver was already opened and another driver was
           trying to open it exclusively.  We cannot give exclusive access in
           this case
        */
        OSAL_MUTEX_Unlock( &dObj->clientMutex);
        return(DRV_HANDLE_INVALID);
    }

    /* Enter here only if the lock was obtained */
    for(iClient = 0; iClient != dObj->nClientsMax; iClient++)
    {
        if(false == ((DRV_I2C_CLIENT_OBJ *)dObj->clientObjPool)[iClient].inUse)
        {
            /* This means we have a free client object to use */
            clientObj = &((DRV_I2C_CLIENT_OBJ *)dObj->clientObjPool)[iClient];

            clientObj->inUse        = true;

            clientObj->hDriver      = dObj;

            clientObj->ioIntent     = ioIntent;

            clientObj->errors       = DRV_I2C_ERROR_NONE;

            clientObj->transferSetup.clockSpeed = dObj->initI2CClockSpeed;

            if(ioIntent & DRV_IO_INTENT_EXCLUSIVE)
            {
                /* Set the driver exclusive flag */
                dObj->isExclusive = true;
            }

            dObj->nClients ++;

            /* Save the generated client handle in the client object, which will
             * be then used to verify the validity of the client handle.
             */
            clientObj->clientHandle = _DRV_I2C_MAKE_HANDLE(dObj->i2cTokenCount, drvIndex, iClient);

            /* Increment the instance specific token counter */
            dObj->i2cTokenCount = _DRV_I2C_UPDATE_TOKEN(dObj->i2cTokenCount);

            break;
        }
    }

    OSAL_MUTEX_Unlock(&dObj->clientMutex);

    return clientObj ? ((DRV_HANDLE)clientObj->clientHandle) : DRV_HANDLE_INVALID;
}

void DRV_I2C_Close( const DRV_HANDLE handle )
{
    /* This function closes the client, The client
       object is deallocated and returned to the
       pool.
    */

    DRV_I2C_CLIENT_OBJ* clientObj;
    DRV_I2C_OBJ* dObj;

    /* Validate the handle */
    clientObj = _DRV_I2C_DriverHandleValidate(handle);

    if(clientObj != NULL)
    {
        dObj = (DRV_I2C_OBJ*)clientObj->hDriver;

        /* Acquire the instance specific mutex to protect the instance specific
         * client pool
         */
        if (OSAL_MUTEX_Lock(&dObj->clientMutex , OSAL_WAIT_FOREVER ) == OSAL_RESULT_TRUE)
        {
            /* Reduce the number of clients */
            dObj->nClients --;

            /* Reset the exclusive flag */
            dObj->isExclusive = false;

            /* De-allocate the object */
            clientObj->inUse = false;

            /* Release the instance specific mutex */
            OSAL_MUTEX_Unlock( &dObj->clientMutex );
        }
    }
}

static bool _DRV_I2C_WriteReadTransfer (
    const DRV_HANDLE handle,
    uint16_t address,
    void* const writeBuffer,
    const size_t writeSize,
    void* const readBuffer,
    const size_t readSize,
    DRV_I2C_TRANSFER_OBJ_FLAGS transferFlags
)
{
    DRV_I2C_CLIENT_OBJ* clientObj = (DRV_I2C_CLIENT_OBJ *)NULL;
    DRV_I2C_OBJ* hDriver = (DRV_I2C_OBJ*)NULL;
    bool isSuccess = false;
    bool isReqAccepted = false;

    /* Validate the driver handle */
    clientObj = _DRV_I2C_DriverHandleValidate(handle);

    if (clientObj == NULL)
    {
        return isSuccess;
    }

    if (transferFlags == DRV_I2C_TRANSFER_OBJ_FLAG_READ)
    {
        if((readSize == 0) || (readBuffer == NULL))
        {
            return isSuccess;
        }
    }
    else if ((transferFlags == DRV_I2C_TRANSFER_OBJ_FLAG_WRITE) || (transferFlags == DRV_I2C_TRANSFER_OBJ_FLAG_WRITE_FORCED))
    {
        if((writeSize == 0) || (writeBuffer == NULL))
        {
            return isSuccess;
        }
    }
    else
    {
        if((writeSize == 0) || (writeBuffer == NULL) || (readSize == 0) || (readBuffer == NULL))
        {
            return isSuccess;
        }
    }

    hDriver = clientObj->hDriver;


    /* Block other threads from accessing the PLIB */
    if (OSAL_MUTEX_Lock(&hDriver->transferMutex, OSAL_WAIT_FOREVER ) == OSAL_RESULT_TRUE)
    {
        /* Error is cleared for every new transfer */
        clientObj->errors = DRV_I2C_ERROR_NONE;

        /* Errors if any, will be saved in the activeClient in the driver callback */
        hDriver->activeClient = (uintptr_t)clientObj;

        /* Check if the transfer setup for this client is different than the current transfer setup */
        if (hDriver->currentTransferSetup.clockSpeed != clientObj->transferSetup.clockSpeed)
        {
            /* Set the new transfer setup */
            hDriver->i2cPlib->transferSetup(&clientObj->transferSetup, 0);

            hDriver->currentTransferSetup.clockSpeed = clientObj->transferSetup.clockSpeed;
        }

        switch(transferFlags)
        {
            case DRV_I2C_TRANSFER_OBJ_FLAG_READ:
                if (hDriver->i2cPlib->read(address, readBuffer, readSize) == true)
                {
                    isReqAccepted = true;
                }
                break;

            case DRV_I2C_TRANSFER_OBJ_FLAG_WRITE:
                if (hDriver->i2cPlib->write(address, writeBuffer, writeSize) == true)
                {
                    isReqAccepted = true;
                }
                break;


            case DRV_I2C_TRANSFER_OBJ_FLAG_WRITE_READ:
                if (hDriver->i2cPlib->writeRead(address, writeBuffer, writeSize, readBuffer, readSize) == true)
                {
                    isReqAccepted = true;
                }
                break;

            default:
                break;
        }

        if (isReqAccepted == true)
        {
            /* Wait till transfer completes. This semaphore is released from ISR */
            if (OSAL_SEM_Pend( &hDriver->transferDone, OSAL_WAIT_FOREVER ) == OSAL_RESULT_TRUE)
            {
                if (hDriver->transferStatus == DRV_I2C_TRANSFER_STATUS_COMPLETE)
                {
                    isSuccess = true;
                }
            }
        }
        else
        {
            /* Update error into the client object*/
            clientObj->errors = hDriver->i2cPlib->errorGet();

            if(clientObj->errors == DRV_I2C_ERROR_NONE)
            {
                hDriver->transferStatus = DRV_I2C_TRANSFER_STATUS_COMPLETE;
            }
            else
            {
                hDriver->transferStatus = DRV_I2C_TRANSFER_STATUS_ERROR;
            }
        }

        /* Release the mutex to allow other threads to access the PLIB */
        OSAL_MUTEX_Unlock(&hDriver->transferMutex);
    }

    return isSuccess;
}

bool DRV_I2C_ReadTransfer(
    const DRV_HANDLE handle,
    uint16_t address,
    void* const readBuffer,
    const size_t readSize
)
{
    return _DRV_I2C_WriteReadTransfer(
        handle,
        address,
        NULL,
        0,
        readBuffer,
        readSize,
        DRV_I2C_TRANSFER_OBJ_FLAG_READ
    );
}

bool DRV_I2C_WriteTransfer(
    const DRV_HANDLE handle,
    uint16_t address,
    void* const writeBuffer,
    const size_t writeSize
)
{
    return _DRV_I2C_WriteReadTransfer(
        handle,
        address,
        writeBuffer,
        writeSize,
        NULL,
        0,
        DRV_I2C_TRANSFER_OBJ_FLAG_WRITE
    );
}


bool DRV_I2C_WriteReadTransfer (
    const DRV_HANDLE handle,
    uint16_t address,
    void* const writeBuffer,
    const size_t writeSize,
    void* const readBuffer,
    const size_t readSize
)
{
    return _DRV_I2C_WriteReadTransfer(
        handle,
        address,
        writeBuffer,
        writeSize,
        readBuffer,
        readSize,
        DRV_I2C_TRANSFER_OBJ_FLAG_WRITE_READ
    );
}
/*******************************************************************************
 End of File
*/
