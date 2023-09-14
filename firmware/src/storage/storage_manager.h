/**
* @file storage_manager.h
* @author apolisskyi
* @date 12.09.2023
*
* @bried Storage manager to operate with data in flash memory
 *
 * @see https://microchip-mplab-harmony.github.io/core/GUID-8A5BD4DE-CB7C-4469-8159-D2A013406C01.html
*/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../config/default/configuration.h"
#include "../config/default/driver/driver_common.h"
#include "../config/default/definitions.h"
#include "../config/common.defs.h"
#include "../../../../libraries/active-object-fsm/src/active_object/active_object.h"
#include "../../../libraries/active-object-fsm/src/fsm/fsm.h"

#ifdef    __cplusplus
extern "C" {
#endif

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#define STORAGE_QUEUE_MAX_CAPACITY              (8)
#define READ_BLOCK_SIZE                         (1)
#define PARTITION_0_ADDRESS                     (512)
#define READ_BLOCKS_IN_PAGE                     (DRV_AT25DF_PAGE_SIZE / READ_BLOCK_SIZE)
#define BOOT_SECTOR_PAGES_TO_VALIDATE           (1) // Amount of pages on flash to verify with boot sector header in NVM
#define IS_EQUAL_PAGES                          (0)
    
extern const unsigned char FATBootSectorImage[DRV_MEMORY_BOOT_SECTOR_SIZE_PAGES * DRV_AT25DF_PAGE_SIZE];

/** @brief storage manager states */
typedef enum {
    STORAGE_NO_STATE = 0,
    STORAGE_ST_INIT,
    STORAGE_ST_IDLE,
    STORAGE_ST_READ_BOOT_SECTOR,
    STORAGE_ST_VERIFY_BOOT_SECTOR,
    STORAGE_ST_WRITE_BOOT_SECTOR,
    STORAGE_ST_FIND_LAST_NONEMPTY_PAGE,
    STORAGE_ST_STORE_DATA_IN_TAIL,
    STORAGE_ST_ERROR,
    STORAGE_STATES_MAX
} STORAGE_STATE;

/** @brief storage manager events signals */
typedef enum {
    STORAGE_NO_EVENT = 0,
    STORAGE_CHECK_MEMORY_BOOT_SECTOR,
    STORAGE_WRITE_MEMORY_BOOT_SECTOR,
    STORAGE_VERIFY_MEMORY_BOOT_SECTOR_SUCCESS,
    STORAGE_FIND_LAST_NON_EMPTY_PAGE,
    STORAGE_FIND_LAST_NON_EMPTY_PAGE_SUCCESS,
    STORAGE_STORE_DATA_IN_TAIL,
    STORAGE_TRANSFER_SUCCESS,
    STORAGE_TRANSFER_FAIL,
    STORAGE_ERROR,
    STORAGE_SIG_MAX
} STORAGE_SIG;

/**
* @brief STORAGE Active Object Type
* @extends TActiveObject
*/
typedef struct {
    TActiveObject super;
    DRV_HANDLE drvMemoryHandle;
    DRV_MEMORY_COMMAND_HANDLE transferHandle;
    uint8_t pagesToProcessAmount;
    uint16_t page;
    uint8_t pageBuffer[DRV_AT25DF_PAGE_SIZE];
} TSTORAGEActiveObject;

/**
* @brief Initialize and construct actor, should be called before tasks
* @memberof TSTORAGEActiveObject
*/
void STORAGE_Initialize(void);

/* Microchip Harmony 3 specific */

/** @brief Perform Actor tasks, mainly listen for events and process them */
void STORAGE_Tasks(void);

/**
* @brief Clean page buffer
* @memberof TSTORAGEActiveObject
*/
void STORAGE_CLearPageBuffer(TSTORAGEActiveObject *const storageAO);

/**
 * @brief Callback for SPI (MEMORY) ISR on success/error transfer.
 * @see DRV_MEMORY_COMMAND_HANDLE
 *
 * Required by Harmony framework drivers approach.
 * On success/error transfer ISR should dispatch appropriate event to Actor's queue
 * @param event
 * @param commandHandle
 * @param context
 */
void STORAGE_TransferEventHandler(DRV_MEMORY_EVENT event, DRV_MEMORY_COMMAND_HANDLE commandHandle, uintptr_t context);

#ifdef    __cplusplus
}
#endif

#endif //STORAGE_MANAGER_H