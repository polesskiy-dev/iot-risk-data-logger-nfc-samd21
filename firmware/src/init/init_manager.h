#ifndef INIT_MANAGER_H
#define INIT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../config/default/configuration.h"
#include "../config/default/definitions.h"
#include "../config/common.defs.h"
#include "../../../libraries/active-object-fsm/src/active_object/active_object.h"
#include "../../../libraries/active-object-fsm/src/fsm/fsm.h"
#include "./init.config.h"
#include "../sensors/sht3x-temperature-humidity/sht3x.h"
#include "../nfc/nfc.h"
#include "../storage/storage_manager.h"

#ifdef    __cplusplus
extern "C" {
#endif

#define INIT_QUEUE_MAX_CAPACITY              (8)

/** @brief init manager states */
typedef enum {
    INIT_NO_STATE = 0,
    INIT_ST_INIT,
    INIT_ST_IDLE,
    INIT_STATES_MAX
} INIT_STATE;

/** @brief init manager events signals */
typedef enum {
    INIT_NO_EVENT = 0,
    INIT_SIG_SENSORS,
    INIT_SIG_NFC,
    INIT_SIG_STORAGE,
    DEINIT_SIG_SENSORS,
    DEINIT_SIG_NFC,
    DEINIT_SIG_STORAGE,
    INIT_SIG_MAX
} INIT_SIG;

/** 
* @brief init manager active object
 * @extends TActiveObject
 */
typedef struct {
    TActiveObject super;
} TInitActiveObject;

/**
* @brief Initialize and construct actor, should be called before tasks
* @memberof TInitActiveObject
*/
void INIT_Initialize(void);

/* Microchip Harmony 3 specific */

/** @brief Perform Actor tasks, mainly listen for events and process them */
void INIT_Tasks(void);

#ifdef    __cplusplus
}
#endif

#endif //INIT_MANAGER_H

