/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "init/init_manager.h"

void _toggleLED(uintptr_t context) {
    _LED_Toggle();
}


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main(void) {
    /* Initialize all modules */
    SYS_Initialize(NULL);

    // Debug: verify that app sin't stuck
    SYS_TIME_CallbackRegisterMS(_toggleLED, (uintptr_t) NULL, 1000, SYS_TIME_PERIODIC);

    while (true) {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();

        INIT_Tasks();
        STORAGE_Tasks();
        // SHT3X_Tasks();

    }

    /* Execution should not come here during normal operation */

    return (EXIT_FAILURE);
}


/*******************************************************************************
 End of File
*/

