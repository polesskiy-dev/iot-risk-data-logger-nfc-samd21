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
#include "./nfc.config.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef union {
   struct {
       uint32_t MsbPasswd;
       uint32_t LsbPasswd;
   };
   uint8_t pwd[2 * sizeof (uint32_t)];
} ST25DV_PASSWD;

/**
* @brief NFC Active Object Type
* @extends TActiveObject
*/
typedef struct {
   TActiveObject super;
   DRV_HANDLE drvI2CHandle;
   DRV_I2C_TRANSFER_HANDLE transferHandle;
   uint8_t retriesLeft;
   bool RFFieldPresence;
   union {
       uint8_t raw[NFC_CMD_SIZE + ST25DV_MAX_MAILBOX_LENGTH];
       struct {
           uint16_t cmd;
           uint8_t mailbox[ST25DV_MAX_MAILBOX_LENGTH];
       };
   } transferBuf;
   struct {
       uint8_t uid[NFC_UID_SIZE];
       ST25DV_PASSWD pwd;
   } st25dvRegs;
} TNFCActiveObject;

/**
* @brief Initialize and construct actor, should be called before tasks
* @memberof TNFCActiveObject
*/
void NFC_Initialize ( void );

/* Microchip Harmony 3 specific */

/** @brief Perform Actor tasks, mainly listen for events and process them */
void NFC_Tasks( void );

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

#ifdef	__cplusplus
}
#endif

#endif //NFC_H
