#ifndef SHT3X_CONFIG_H
#define SHT3X_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define SHT3X_I2C_ADDR_DFLT             (0x44)

#define SHT3X_CMD_SIZE                  (2)
#define SHT3X_STATUS_REG_SIZE           (2)
#define SHT3X_MEASUREMENTS_SIZE         (6)

#define SHT3X_MEASURE_TIME_MS           (15)

#define SHT3X_QUEUE_MAX_CAPACITY        (8)

/**
 * @brief sht3x-temperature-humidity states
 */
typedef enum {
    SHT3X_NO_STATE,
    SHT3X_ST_INIT,
    SHT3X_ST_IDLE,
    SHT3X_ST_READ_STATUS,
    SHT3X_ST_MEASURE,
    SHT3X_ST_READ_MEASURE,
    SHT3X_ST_ERROR,
    SHT3X_STATES_MAX
} SHT3X_STATE;

typedef enum {
    SHT3X_NO_EVENT,
    SHT3X_TRANSFER_SUCCESS,
    SHT3X_TRANSFER_FAIL,
    SHT3X_READ_STATUS,
    SHT3X_MEASURE,
    SHT3X_READ_MEASURE,
    SHT3X_ERROR,
    SHT3X_SIG_MAX,
} SHT3X_SIG;

#ifdef __cplusplus
}
#endif

#endif // SHT3X_CONFIG_H