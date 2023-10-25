/**
* @file nfc.h
* @author apolisskyi
* @date 16.02.2023
*
* @bried NFC ST25DV Actor declarations
*/

#ifndef NFC_H
#define NFC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../config/default/configuration.h"
#include "../config/default/driver/driver_common.h"
#include "../config/default/definitions.h"
#include "../config/common.defs.h"
#include "../../../../libraries/active-object-fsm/src/active_object/active_object.h"
#include "../../../../libraries/active-object-fsm/src/fsm/fsm.h"
#include "../init_manager/init.config.h"
#include "./nfc.config.h"

#ifdef    __cplusplus
extern "C" {
#endif

/**
* @brief NFC Active Object Type
* @extends TActiveObject
*/
typedef struct {
    TActiveObject super;
    DRV_HANDLE drvI2CHandle;
    DRV_I2C_TRANSFER_HANDLE transferHandle;
    uint8_t retriesLeft;
    union {
        uint8_t raw[NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE];
        struct {
            uint8_t cmd[NFC_CMD_SIZE];
            uint8_t mailbox[ST25DV_MAILBOX_SIZE];
        };
    } transferBuf;
    struct {
        uint8_t uid[NFC_UID_SIZE];
        uint8_t pwd[NFC_PASSWORD_SIZE];
        union {
            uint8_t raw;
            struct {
                unsigned RF_USER:1;
                unsigned RF_ACTIVITY:1;
                unsigned RF_INTERRUPT:1;
                unsigned FIELD_FALLING:1;
                unsigned FIELD_RISING:1;
                unsigned RF_PUT_MSG:1;
                unsigned RF_GET_MSG:1;
                unsigned RF_WRITE:1;
            } bitFields;
        } interruptStatus;
    } st25dvRegs;
} TNFCActiveObject;

/**
* @brief Initialize and construct actor, should be called before tasks
* @memberof TNFCActiveObject
*/
TActiveObject *NFC_Initialize(void);

/**
 * @brief Deinitialize the actor
 * @details Sets to NO_STATE, all pending events will be lost
 * @memberof TNFCActiveObject
 */
void NFC_Deinitialize(void);

/* Microchip Harmony 3 specific */

/** @brief Perform Actor tasks, mainly listen for events and process them */
void NFC_Tasks(void);

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
void NFC_TransferEventHandler(DRV_I2C_TRANSFER_EVENT event, DRV_I2C_TRANSFER_HANDLE transferHandle, uintptr_t context);

/** @brief dispatch error on i2c transfer queuing */
void NFC_DispatchErrorOnInvalidTransfer(TNFCActiveObject *const nfcAO);

void NFC_VerifyRetries(TNFCActiveObject *const nfcAO);

void NFC_ProcessPrepareMailboxFSM(TNFCActiveObject *const nfcAO, TEvent event);

#ifdef    __cplusplus
}
#endif

#endif //NFC_H
