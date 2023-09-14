/******************************************************************************
  DRV_AT25 Library Interface Implementation

  Company:
    Microchip Technology Inc.

  File Name:
    drv_at25.c

  Summary:
    AT25 EEPROM Driver Library Interface implementation

  Description:
    The AT25 Library provides a interface to access the AT25 external EEPROM.
    This file implements the AT25 Library interface.
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
// Section: Include Files
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "driver/at25/drv_at25.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global objects
// *****************************************************************************
// *****************************************************************************

/* This is the driver instance object array. */
static DRV_AT25_OBJ gDrvAT25Obj;


// *****************************************************************************
// *****************************************************************************
// Section: DRV_AT25 Driver Local Functions
// *****************************************************************************
// *****************************************************************************

static bool lDRV_AT25_ReadData(void* rxData, uint32_t rxDataLength)
{
    bool status = false;

    /* Assert Chip Select */
    SYS_PORT_PinClear(gDrvAT25Obj.chipSelectPin);

    if (gDrvAT25Obj.spiPlib->readData((uint8_t*)rxData, rxDataLength) == true)
    {
        status = true;
    }
    else
    {
        /* De-assert the chip select */
        SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
    }

    return status;
}

static bool lDRV_AT25_WriteEnable(void)
{
    bool status = false;

    gDrvAT25Obj.at25Command[0] = (uint8_t)DRV_AT25_CMD_WRITE_ENABLE;

    /* Assert Chip Select */
    SYS_PORT_PinClear(gDrvAT25Obj.chipSelectPin);

    if(gDrvAT25Obj.spiPlib->writeData(&gDrvAT25Obj.at25Command[0], 1) == true)
    {
        status = true;
    }
    else
    {
        /* De-assert the chip select */
        SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
    }

    return status;
}

static bool lDRV_AT25_WriteMemoryAddress(uint8_t command, uint32_t address)
{
    bool status = false;
    uint32_t nBytes = 0;

    gDrvAT25Obj.at25Command[nBytes] = command;
    nBytes++;

    if (gDrvAT25Obj.flashSize > 65536U)
    {
        gDrvAT25Obj.at25Command[nBytes] = (uint8_t)(address>>16);
        nBytes++;
    }
    if (gDrvAT25Obj.flashSize > 256U)
    {
        gDrvAT25Obj.at25Command[nBytes] = (uint8_t)(address>>8);
        nBytes++;
    }
    gDrvAT25Obj.at25Command[nBytes] = (uint8_t)(address);
    nBytes++;

    /* Assert Chip Select */
    SYS_PORT_PinClear(gDrvAT25Obj.chipSelectPin);

    /* Send page write or read command and memory address */
    if(gDrvAT25Obj.spiPlib->writeData(gDrvAT25Obj.at25Command, nBytes) == true)
    {
        status = true;
    }
    else
    {
        /* De-assert the chip select */
        SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
    }

    return status;
}

static bool lDRV_AT25_WriteData(void* txData, uint32_t txDataLength, uint32_t address )
{
    bool status = false;
    uint32_t nTransferBytes = 0;

    /* Calculate the max number of bytes that can be written in the current page */
    nTransferBytes = gDrvAT25Obj.pageSize - (address % gDrvAT25Obj.pageSize);

    /* Check if the pending bytes are greater than nTransferBytes */
    nTransferBytes = txDataLength >= nTransferBytes? nTransferBytes: txDataLength;

    gDrvAT25Obj.memoryAddr = address + nTransferBytes;
    gDrvAT25Obj.bufferAddr = (uint8_t *)((uint8_t*)txData + nTransferBytes);
    gDrvAT25Obj.nPendingBytes = txDataLength - nTransferBytes;

    /* Assert Chip Select */
    SYS_PORT_PinClear(gDrvAT25Obj.chipSelectPin);
    /* Send data */
    if (gDrvAT25Obj.spiPlib->writeData((uint8_t*)txData, nTransferBytes) == true)
    {
        status = true;
    }
    else
    {
        /* De-assert the chip select */
        SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
    }
    return status;
}

static bool lDRV_AT25_Write( void* txData, uint32_t txDataLength, uint32_t address )
{
    bool status = false;

    if ((address + txDataLength) > gDrvAT25Obj.flashSize)
    {
        /* Writing past the flash size results in an error */
        return status;
    }

    gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_BUSY;

    /* Save the request */
    gDrvAT25Obj.memoryAddr = address;
    gDrvAT25Obj.bufferAddr = txData;
    gDrvAT25Obj.nPendingBytes = txDataLength;

    gDrvAT25Obj.state = DRV_AT25_STATE_WRITE_CMD_ADDR;

    /* Start the transfer by submitting a Write Enable request. Further commands
     * will be issued from the interrupt context.
    */
    if (lDRV_AT25_WriteEnable() == false)
    {
        gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
    }
    else
    {
        status = true;
    }

    return status;
}


static bool lDRV_AT25_ReadStatus(void)
{
    bool status = false;

    gDrvAT25Obj.at25Command[0] = (uint8_t)DRV_AT25_CMD_READ_STATUS_REG;

    /* Assert Chip Select */
    SYS_PORT_PinClear(gDrvAT25Obj.chipSelectPin);

    if(gDrvAT25Obj.spiPlib->writeRead(&gDrvAT25Obj.at25Command[0], 1, &gDrvAT25Obj.at25Command[1], 2 ) == true)
    {
        status = true;
    }
    else
    {
        /* De-assert the chip select */
        SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
    }

    return status;
}

static void lDRV_AT25_Handler( void )
{
    switch(gDrvAT25Obj.state)
    {
        case DRV_AT25_STATE_WRITE_CMD_ADDR:
            /* De-assert the chip select */
            SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
            /* Send page write command and memory address */
            if (lDRV_AT25_WriteMemoryAddress((uint8_t)DRV_AT25_CMD_PAGE_PROGRAM, gDrvAT25Obj.memoryAddr) == true)
            {
                gDrvAT25Obj.state = DRV_AT25_STATE_WRITE_DATA;
            }
            else
            {
                gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
            }
            break;
        case DRV_AT25_STATE_WRITE_DATA:

            if (lDRV_AT25_WriteData(gDrvAT25Obj.bufferAddr, gDrvAT25Obj.nPendingBytes, gDrvAT25Obj.memoryAddr) == true)
            {
                gDrvAT25Obj.state = DRV_AT25_STATE_CHECK_WRITE_STATUS;
            }
            else
            {
                gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
            }
            break;
        case DRV_AT25_STATE_CHECK_WRITE_STATUS:
            /* De-assert the chip select */
            SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
            /* Read the status of EEPROM internal write cycle */
            if (lDRV_AT25_ReadStatus() == true)
            {
                gDrvAT25Obj.state = DRV_AT25_STATE_WAIT_WRITE_COMPLETE;
            }
            else
            {
                gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
            }
            break;
        case DRV_AT25_STATE_WAIT_WRITE_COMPLETE:
            /* De-assert the chip select */
            SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
            /* Check the busy bit in the status register. 0 = Ready, 1 = busy*/
            if ((gDrvAT25Obj.at25Command[2] & 0x01U) != 0U)
            {
                /* Keep reading the status of EEPROM internal write cycle */
                if (lDRV_AT25_ReadStatus() == false)
                {
                    gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
                }
            }
            else
            {
                /* Internal write complete. Now check if more data pending */
                if ((gDrvAT25Obj.nPendingBytes) != 0U)
                {
                    /* Enable writing to EEPROM */
                    if (lDRV_AT25_WriteEnable() == false)
                    {
                        gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
                    }
                    else
                    {
                        gDrvAT25Obj.state = DRV_AT25_STATE_WRITE_CMD_ADDR;
                    }
                }
                else
                {
                    gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_COMPLETED;
                }
            }
            break;

        case DRV_AT25_STATE_READ_DATA:

            if (lDRV_AT25_ReadData((void*)gDrvAT25Obj.bufferAddr, gDrvAT25Obj.nPendingBytes) == true)
            {
                gDrvAT25Obj.state = DRV_AT25_STATE_WAIT_READ_COMPLETE;
            }
            else
            {
                gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
            }
            break;

        case DRV_AT25_STATE_WAIT_READ_COMPLETE:
            /* De-assert the chip select */
            SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);
            gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_COMPLETED;
            break;

        default:
                /* Nothing to do*/
            break;
    }
}


/* This function will be called by SPI PLIB when transfer is completed */
static void lSPIEventHandler(uintptr_t context )
{
    lDRV_AT25_Handler ();

    /* If transfer is complete, notify the application */
    if (gDrvAT25Obj.transferStatus != DRV_AT25_TRANSFER_STATUS_BUSY)
    {
        if ((gDrvAT25Obj.eventHandler) != NULL)
        {
            gDrvAT25Obj.eventHandler(gDrvAT25Obj.transferStatus, gDrvAT25Obj.context);
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: DRV_AT25 Driver Global Functions
// *****************************************************************************
// *****************************************************************************
/* MISRA C-2012 Rule 11.3, 11.8 deviated below. Deviation record ID -
  H3_MISRAC_2012_R_11_3_DR_1 & H3_MISRAC_2012_R_11_8_DR_1*/

SYS_MODULE_OBJ DRV_AT25_Initialize(
    const SYS_MODULE_INDEX drvIndex,
    const SYS_MODULE_INIT * const init
)
{
    DRV_AT25_INIT* at25Init = (DRV_AT25_INIT *)init;

    /* Validate the request */
    if(drvIndex >= DRV_AT25_INSTANCES_NUMBER)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    if(gDrvAT25Obj.inUse == true)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    gDrvAT25Obj.status                = SYS_STATUS_UNINITIALIZED;

    gDrvAT25Obj.inUse                 = true;
    gDrvAT25Obj.nClients              = 0;
    gDrvAT25Obj.transferStatus        = DRV_AT25_TRANSFER_STATUS_COMPLETED;
    gDrvAT25Obj.writeCompleted        = true;

    gDrvAT25Obj.spiPlib               = at25Init->spiPlib;
    gDrvAT25Obj.nClientsMax           = at25Init->numClients;
    gDrvAT25Obj.pageSize              = at25Init->pageSize;
    gDrvAT25Obj.flashSize             = at25Init->flashSize;
    gDrvAT25Obj.blockStartAddress     = at25Init->blockStartAddress;
    gDrvAT25Obj.chipSelectPin         = at25Init->chipSelectPin;
    gDrvAT25Obj.holdPin               = at25Init->holdPin;
    gDrvAT25Obj.writeProtectPin       = at25Init->writeProtectPin;

    gDrvAT25Obj.spiPlib->callbackRegister(lSPIEventHandler, 0U);

    /* De-assert Chip Select, Hold and Write protect pin to begin with. */
    SYS_PORT_PinSet(gDrvAT25Obj.chipSelectPin);

    if (gDrvAT25Obj.holdPin != SYS_PORT_PIN_NONE)
    {
        SYS_PORT_PinSet(gDrvAT25Obj.holdPin);
    }
    if (gDrvAT25Obj.writeProtectPin != SYS_PORT_PIN_NONE)
    {
        SYS_PORT_PinSet(gDrvAT25Obj.writeProtectPin);
    }

    /* Update the status */
    gDrvAT25Obj.status                = SYS_STATUS_READY;

    /* Return the object structure */
    return ( (SYS_MODULE_OBJ)drvIndex );

}
/* MISRAC 2012 deviation block end */

SYS_STATUS DRV_AT25_Status( const SYS_MODULE_INDEX drvIndex )
{
    /* Return the driver status */
    return (gDrvAT25Obj.status);
}

DRV_HANDLE DRV_AT25_Open(
    const SYS_MODULE_INDEX drvIndex,
    const DRV_IO_INTENT ioIntent
)
{
    /* Validate the request */
    if (drvIndex >= DRV_AT25_INSTANCES_NUMBER)
    {
        return DRV_HANDLE_INVALID;
    }

    if((gDrvAT25Obj.status != SYS_STATUS_READY) || (gDrvAT25Obj.inUse == false) \
            || (gDrvAT25Obj.nClients >= gDrvAT25Obj.nClientsMax))
    {
        return DRV_HANDLE_INVALID;
    }

    gDrvAT25Obj.nClients++;

    return ((DRV_HANDLE)0);
}

void DRV_AT25_Close( const DRV_HANDLE handle )
{
    if((handle != DRV_HANDLE_INVALID) && (handle == 0U))
    {
        gDrvAT25Obj.nClients--;
    }
}

void DRV_AT25_EventHandlerSet(
    const DRV_HANDLE handle,
    const DRV_AT25_EVENT_HANDLER eventHandler,
    const uintptr_t context
)
{
    if((handle != DRV_HANDLE_INVALID) && (handle == 0U))
    {
        gDrvAT25Obj.eventHandler = eventHandler;
        gDrvAT25Obj.context = context;
    }
}

bool DRV_AT25_Read(
    const DRV_HANDLE handle,
    void* rxData,
    uint32_t rxDataLength,
    uint32_t address
)
{
    bool isRequestAccepted = false;

    if((handle == DRV_HANDLE_INVALID) || (handle > 0U) || (rxData == NULL) \
            || (rxDataLength == 0U) || (gDrvAT25Obj.transferStatus == DRV_AT25_TRANSFER_STATUS_BUSY))
    {
        return isRequestAccepted;
    }

    if ((address + rxDataLength) > gDrvAT25Obj.flashSize)
    {
        /* Writing past the flash size results in an error */
        return isRequestAccepted;
    }

    gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_BUSY;

    /* save the request */
    gDrvAT25Obj.nPendingBytes = rxDataLength;
    gDrvAT25Obj.bufferAddr = rxData;
    gDrvAT25Obj.memoryAddr = address;

    gDrvAT25Obj.state = DRV_AT25_STATE_READ_DATA;

    if (lDRV_AT25_WriteMemoryAddress((uint8_t)DRV_AT25_CMD_READ, address) == true)
    {
        isRequestAccepted = true;
    }
    else
    {
        gDrvAT25Obj.transferStatus = DRV_AT25_TRANSFER_STATUS_ERROR;
    }

    return isRequestAccepted;
}

bool DRV_AT25_Write( const DRV_HANDLE handle, void *txData, uint32_t txDataLength, uint32_t address )
{
    if((handle == DRV_HANDLE_INVALID) || (handle > 0U) || (txData == NULL) \
            || (txDataLength == 0U) || (gDrvAT25Obj.transferStatus == DRV_AT25_TRANSFER_STATUS_BUSY))
    {
        return false;
    }
    else
    {
        return lDRV_AT25_Write(txData, txDataLength, address);
    }
}

bool DRV_AT25_PageWrite(const DRV_HANDLE handle, void *txData, uint32_t address )
{
    return DRV_AT25_Write(handle, txData, gDrvAT25Obj.pageSize, address );
}

DRV_AT25_TRANSFER_STATUS DRV_AT25_TransferStatusGet(const DRV_HANDLE handle)
{
    if((handle == DRV_HANDLE_INVALID) || (handle > 0U))
    {
        return DRV_AT25_TRANSFER_STATUS_ERROR;
    }
    else
    {
        return gDrvAT25Obj.transferStatus;
    }
}

bool DRV_AT25_GeometryGet(const DRV_HANDLE handle, DRV_AT25_GEOMETRY *geometry)
{
    uint32_t flash_size = 0;

    if((handle == DRV_HANDLE_INVALID) || (handle > 0U))
    {
        return false;
    }

    flash_size = gDrvAT25Obj.flashSize;

    if ((flash_size == 0U) ||
        (gDrvAT25Obj.blockStartAddress >= flash_size))
    {
        return false;
    }

    flash_size = flash_size - gDrvAT25Obj.blockStartAddress;

    /* Flash size should be at-least of a Write Block size */
    if (flash_size < gDrvAT25Obj.pageSize)
    {
        return false;
    }

    /* Read block size and number of blocks */
    geometry->readBlockSize = 1;
    geometry->readNumBlocks = flash_size;

    /* Write block size and number of blocks */
    geometry->writeBlockSize = gDrvAT25Obj.pageSize;
    geometry->writeNumBlocks = (flash_size / gDrvAT25Obj.pageSize);

    /* Erase block size and number of blocks */
    geometry->eraseBlockSize = 1;
    geometry->eraseNumBlocks = flash_size;

    /* Number of regions */
    geometry->readNumRegions = 1;
    geometry->writeNumRegions = 1;
    geometry->eraseNumRegions = 1;

    /* Block start address */
    geometry->blockStartAddress = gDrvAT25Obj.blockStartAddress;

    return true;
}
