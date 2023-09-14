/**
 * @file    sht3x.h
 * @author  apolisskyi
 * @date February 2, 2023
 */

#ifndef SHT3X_H
#define    SHT3X_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../config/default/configuration.h"
#include "../../config/default/driver/driver_common.h"
#include "../../config/default/definitions.h"
#include "../../config/common.defs.h"
#include "../../../../libraries/active-object-fsm/src/active_object/active_object.h"
#include "../../../../libraries/active-object-fsm/src/fsm/fsm.h"
#include "./sht3x.config.h"

#ifdef    __cplusplus
extern "C" {
#endif

/**
 * @brief SHT3x Active Object Type
 * @extends TActiveObject
*/
typedef struct {
    TActiveObject super;
    DRV_HANDLE drvI2CHandle;
    DRV_I2C_TRANSFER_HANDLE transferHandle;
    struct {
        uint16_t status;
        uint8_t measurements[SHT3X_MEASUREMENTS_SIZE];
    } sensorRegs;
} TSHT3xActiveObject;

/** 
 * @brief Initialize and construct actor, should be called before tasks 
 * @memberof TSHT3xActiveObject
 */
void SHT3X_Initialize(void);

/* Microchip Harmony 3 specific */

/** @brief Perform Actor tasks, mainly listen for events and process them */
void SHT3X_Tasks(void);

/**
 * @brief Callback for I2C ISR on success/error transfer.
 *
 * Required by Harmony framework drivers approach.
 * On success/error transfer ISR should dispatch appropriate event to Actor's queue
 *
 * @see https://microchip-mplab-harmony.github.io/core/index.html?GUID-95F7ABE3-6864-4FC9-B11B-97B31ACF683C
 * @param event[in]             transfer event
 * @param transferHandle[in]    marks appropriate Actor to associate callback with
 * @param context[in]           should be ptr to Actor, mostly unused
 */
void
SHT3X_TransferEventHandler(DRV_I2C_TRANSFER_EVENT event, DRV_I2C_TRANSFER_HANDLE transferHandle, uintptr_t context);

#ifdef    __cplusplus
}
#endif

#endif    /* SHT3X_H */
