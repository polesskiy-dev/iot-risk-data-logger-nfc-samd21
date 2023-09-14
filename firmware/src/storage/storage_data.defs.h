/**
 * @file storage_data.defs.h
 * @brief Description of structures to be stored in flash memory
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "../config/default/configuration.h"
#include "../config/default/driver/driver_common.h"
#include "../config/default/definitions.h"

#ifdef    __cplusplus
extern "C" {
#endif

#ifndef STORAGE_DATA_DEFS_H
#define STORAGE_DATA_DEFS_H

// timestamp + sht3x + ambient light + gap, should be factor of 256
#define SENSOR_RECURRING_STORAGE_DATA_SIZE  (4 + 4 + 4 + 4)
#define START_STOP_STORAGE_DATA_SIZE        (4 + 4 + 4 + 4 + 4)

// TODO define in sensor
typedef struct {
    uint16_t temperature;
    uint16_t humidity;
} TSHT3xTemperatureHumiditySensorData;;

// TODO define in sensor
typedef struct {
    uint32_t ambientLight;
} TAmbientLightSensorData;

typedef struct {
    uint32_t timestamp;
    TSHT3xTemperatureHumiditySensorData sht3XTemperatureHumiditySensorData;
    TAmbientLightSensorData ambientLightSensorData;
} TSensorsStorageData;

typedef struct {
    uint32_t latitude;
    uint32_t longitude;
} TLocation;

typedef struct {
    uint32_t timestamp;
    uint32_t operatorID;
    uint32_t batteryVoltage;
    TLocation location;
} TLogsStartStopStorageData;

#ifdef    __cplusplus
}
#endif

#endif //STORAGE_DATA_DEFS_H
