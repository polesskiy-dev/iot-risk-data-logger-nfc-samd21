/**
 * @file app_manager.h
 *
 * @brief handle sub application tasks
 *
 * @details This module runs all sub applications tasks
 * According to the app state, some of them may be disabled e.g. on USB cable connect we'll run only USB app tasks
 */

#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "configuration.h"
#include "definitions.h"
#include "device.h"
#include "../../../libraries/active-object-fsm/src/active_object/active_object.h"
#include "../../../libraries/active-object-fsm/src/fsm/fsm.h"
#include "../init_manager/init_manager.h"
#include "../usb_manager/usb_manager.h"
#include "../config/common.defs.h"

#ifdef    __cplusplus
extern "C" {
#endif

#define APP_QUEUE_MAX_CAPACITY              (4)

/** @brief app manager states */
typedef enum {
    APP_NO_STATE = 0,
    APP_ST_INIT,
    APP_ST_USB_ONLY,
    APP_ST_NFC_ONLY,
    APP_ST_NFC_AND_SENSORS,
    APP_STATES_MAX
} APP_STATE;

/** @brief app manager events signals */
typedef enum {
    APP_NO_EVENT = 0,
    USB_CABLE_CONNECTED,
    USB_CABLE_DISCONNECTED,
    NFC_RF_FIELD_APPEARS,
    NFC_RF_FIELD_DISAPPEAR,
    APP_SIG_MAX
} APP_SIG;

/**
* @memberof TAppActiveObject
*/
TActiveObject *APP_Initialize(void);

/* Microchip Harmony 3 specific */

/** @brief Perform App Actor tasks, mainly listen for events and process them */
void APP_Tasks(void);

#ifdef    __cplusplus
}
#endif

#endif //APP_MANAGER_H
