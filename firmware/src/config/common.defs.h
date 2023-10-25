#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

// LED 
#define LED_Off()   _LED_Set()
#define LED_On()    _LED_Clear()

#define NO_RETRIES_LEFT (0x00)

/** @brief global active objects IDs */
typedef enum {
    NO_ID,
    INIT_AO_ID,
    MAIN_APP_AO_ID,
    STORAGE_AO_ID,
    NFC_AO_ID,
    SHT3X_AO_ID,
    AMBIENT_LIGHT_AO_ID,
    ACCELEROMETER_AO_ID,
    ACTIVE_OBJECTS_MAX
} SYSTEM_ACTIVE_OBJECT_IDS;

#endif //COMMON_DEFS_H