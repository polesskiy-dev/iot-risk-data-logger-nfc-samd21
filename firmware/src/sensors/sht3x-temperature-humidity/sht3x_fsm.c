#include "./sht3x.h"

extern TSHT3xActiveObject sht3xAO;

/* sht3x-temperature-humidity commands registers */
static const uint8_t SHT3X_CMD_READ_STATUS_REG[SHT3X_CMD_SIZE] = {0xF3, 0x2D};
static const uint8_t SHT3X_CMD_MEASURE_LPM[SHT3X_CMD_SIZE] = {0x24, 0x16};

/* Event handlers f prototypes */
static const TState *_idle(TActiveObject *const AO, TEvent event);

static const TState *_readStatus(TActiveObject *const AO, TEvent event);

static const TState *_measure(TActiveObject *const AO, TEvent event);

static const TState *_readMeasurements(TActiveObject *const AO, TEvent event);

static const TState *_error(TActiveObject *const AO, TEvent event);

static void _dispatchReadMeasurements(uintptr_t context);

static void _tempDispatchMeasure(uintptr_t context); // TODO replace it by scheduler

// error on i2c transfer queuing
static inline void _dispatchErrorOnInvalidTransfer(TSHT3xActiveObject *const sht3xAO) {
    if (DRV_I2C_TRANSFER_HANDLE_INVALID == sht3xAO->transferHandle) {
        ActiveObject_Dispatch(&(sht3xAO->super), (TEvent) {.sig = SHT3X_ERROR});
    };
};

/* states */
const TState sht3xStatesList[SHT3X_STATES_MAX] = {
        [SHT3X_NO_STATE]          = {.name = SHT3X_NO_STATE},
        [SHT3X_ST_INIT]           = {.name = SHT3X_ST_INIT},
        [SHT3X_ST_IDLE]           = {.name = SHT3X_ST_IDLE},
        [SHT3X_ST_READ_STATUS]    = {.name = SHT3X_ST_READ_STATUS},
        [SHT3X_ST_MEASURE]        = {.name = SHT3X_ST_MEASURE},
        [SHT3X_ST_READ_MEASURE]   = {.name = SHT3X_ST_READ_MEASURE},
        [SHT3X_ST_ERROR]          = {.name = SHT3X_ST_ERROR}
};

/* state transitions table */
const TEventHandler sht3xTransitionTable[SHT3X_STATES_MAX][SHT3X_SIG_MAX] = {
        [SHT3X_ST_INIT]=                {[SHT3X_READ_STATUS]=_readStatus, [SHT3X_MEASURE]=_measure, [SHT3X_ERROR]=_error},
        [SHT3X_ST_IDLE]=                {[SHT3X_READ_STATUS]=_readStatus, [SHT3X_MEASURE]=_measure, [SHT3X_ERROR]=_error},
        [SHT3X_ST_READ_STATUS]=         {[SHT3X_TRANSFER_SUCCESS]=_idle, [SHT3X_TRANSFER_FAIL]=_error, [SHT3X_ERROR]=_error},
        [SHT3X_ST_MEASURE]=             {[SHT3X_TRANSFER_SUCCESS]=NULL, [SHT3X_TRANSFER_FAIL]=_error, [SHT3X_READ_MEASURE]=_readMeasurements, [SHT3X_ERROR]=_error},
        [SHT3X_ST_READ_MEASURE]=        {[SHT3X_TRANSFER_SUCCESS]=_idle, [SHT3X_TRANSFER_FAIL]=_error, [SHT3X_ERROR]=_error},
        [SHT3X_ST_ERROR]=               {[SHT3X_ERROR]=_error}
};

static const TState *_idle(TActiveObject *const sht3xAO, TEvent event) {
    LED_Off();

    // temporary schedule measurement
    SYS_TIME_CallbackRegisterMS(
            _tempDispatchMeasure,
            (uintptr_t) NULL,
            1000,
            SYS_TIME_SINGLE
    );

    return &(sht3xStatesList[SHT3X_ST_IDLE]);
};

/**
 * @brief Detects whether the sensor is connected - by reading out it's ID register.
 * @description If the sensor does not answer or if the answer is not the expected value,
 * the test fails.
 */
static const TState *_readStatus(TActiveObject *const AO, TEvent event) {
    TSHT3xActiveObject *sht3xAO = (TSHT3xActiveObject *) AO;
    DRV_I2C_WriteReadTransferAdd(
            sht3xAO->drvI2CHandle,
            SHT3X_I2C_ADDR_DFLT,
            (void *) &SHT3X_CMD_READ_STATUS_REG,
            SHT3X_CMD_SIZE,
            &(sht3xAO->sensorRegs.status),
            SHT3X_STATUS_REG_SIZE,
            &(sht3xAO->transferHandle)
    );

    _dispatchErrorOnInvalidTransfer(sht3xAO);

    return &(sht3xStatesList[SHT3X_ST_READ_STATUS]);
};

/**
 * @brief Starts a measurement in high precision mode
 * @description Use sht3x_read() to read
 * out the values, once the measurement is done. The duration of the measurement
 * depends on the sensor in use, please consult the datasheet.
 */
static const TState *_measure(TActiveObject *const AO, TEvent event) {
    TSHT3xActiveObject *sht3xAO = (TSHT3xActiveObject *) AO;
    LED_On();

    DRV_I2C_WriteTransferAdd(
            sht3xAO->drvI2CHandle,
            SHT3X_I2C_ADDR_DFLT,
            (void *) &SHT3X_CMD_MEASURE_LPM,
            SHT3X_CMD_SIZE,
            &(sht3xAO->transferHandle)
    );

    _dispatchErrorOnInvalidTransfer(sht3xAO);

    // schedule measurement read cause SHT3x sensor needs some time to measure temperature/humidity
    SYS_TIME_CallbackRegisterMS(
            _dispatchReadMeasurements,
            (uintptr_t) NULL,
            SHT3X_MEASURE_TIME_MS,
            SYS_TIME_SINGLE
    );

    return &(sht3xStatesList[SHT3X_ST_MEASURE]);
};

/**
 * @brief Read results of measurements (usually after 15ms delay)
 */
static const TState *_readMeasurements(TActiveObject *const AO, TEvent event) {
    TSHT3xActiveObject *sht3xAO = (TSHT3xActiveObject *) AO;

    DRV_I2C_ReadTransferAdd(
            sht3xAO->drvI2CHandle,
            SHT3X_I2C_ADDR_DFLT,
            &(sht3xAO->sensorRegs.measurements),
            SHT3X_MEASUREMENTS_SIZE,
            &(sht3xAO->transferHandle)
    );

    _dispatchErrorOnInvalidTransfer(sht3xAO);

    return &(sht3xStatesList[SHT3X_ST_READ_MEASURE]);
};

static const TState *_error(TActiveObject *const AO, TEvent event) { return &(sht3xStatesList[SHT3X_ST_ERROR]); };

static void _dispatchReadMeasurements(uintptr_t context) {
    ActiveObject_Dispatch(&sht3xAO.super, (TEvent) {.sig = SHT3X_READ_MEASURE});
}

static void _tempDispatchMeasure(uintptr_t context) {
    ActiveObject_Dispatch(&sht3xAO.super, (TEvent) {.sig = SHT3X_MEASURE});
};