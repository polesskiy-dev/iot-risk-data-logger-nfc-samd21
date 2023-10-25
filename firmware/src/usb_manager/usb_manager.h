/**
 * @file usb.h
 * @author apolisskyi
 *
 * @brief handle USB system tasks
 *
 * @details Rather simple module, most of the USB functionality is implemented in Harmony 3.
 * But we need to run the USB (CDC, MSD) only on cable connect (power from USB) to not waste battery and compute power.
 */

#ifndef USB_MANAGER_H
#define USB_MANAGER_H

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
#include "../app_manager/app_manager.h"

#ifdef    __cplusplus
extern "C" {
#endif

void USB_Initialize (void);

/* Microchip Harmony 3 specific */

/** @brief Perform Actor tasks, mainly listen for events and process them */
void USB_Tasks(void);

#ifdef    __cplusplus
}
#endif

#endif //USB_MANAGER_H
