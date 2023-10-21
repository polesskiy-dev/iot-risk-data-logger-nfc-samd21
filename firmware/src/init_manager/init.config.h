#ifndef INIT_CONFIG_H
#define INIT_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../config/default/configuration.h"
#include "../config/default/definitions.h"
#include "../config/common.defs.h"
#include "../../../libraries/active-object-fsm/src/active_object/active_object.h"
#include "../../../libraries/active-object-fsm/src/fsm/fsm.h"

#ifdef    __cplusplus
extern "C" {
#endif

/** @brief global active objects IDs */
typedef enum {
    NO_ID,
    INIT_AO_ID,
    STORAGE_AO_ID,
    NFC_AO_ID,
    SHT3X_AO_ID,
    AMBIENT_LIGHT_AO_ID,
    ACCELEROMETER_AO_ID,
    ACTIVE_OBJECTS_MAX
} SYSTEM_ACTIVE_OBJECT_IDS;

#ifdef    __cplusplus
}
#endif

#endif //INIT_CONFIG_H

