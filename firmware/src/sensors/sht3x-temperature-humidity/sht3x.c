#include "./sht3x.h"

#ifdef __DEBUG
static const char *const _debugStateNames[SHT3X_STATES_MAX] = {
    [SHT3X_NO_STATE] = "SHT3X_NO_STATE",
    [SHT3X_ST_INIT] = "SHT3X_ST_INIT",
    [SHT3X_ST_IDLE] = "SHT3X_ST_IDLE",
    [SHT3X_ST_READ_STATUS] = "SHT3X_ST_READ_STATUS",
    [SHT3X_ST_MEASURE] = "SHT3X_ST_MEASURE",
    [SHT3X_ST_READ_MEASURE] = "SHT3X_ST_READ_MEASURE",
    [SHT3X_ST_ERROR] = "SHT3X_ST_ERROR"
};

static const char *const _debugEventSignals[SHT3X_SIG_MAX] = {
    [SHT3X_NO_EVENT] = "SHT3X_NO_EVENT",
    [SHT3X_TRANSFER_SUCCESS] = "SHT3X_TRANSFER_SUCCESS",
    [SHT3X_TRANSFER_FAIL] = "SHT3X_TRANSFER_FAIL",
    [SHT3X_READ_STATUS] = "SHT3X_READ_STATUS",
    [SHT3X_MEASURE] = "SHT3X_MEASURE",
    [SHT3X_READ_MEASURE] = "SHT3X_READ_MEASURE",
    [SHT3X_ERROR] = "SHT3X_ERROR"
};
#endif

extern const TState sht3xStatesList[SHT3X_STATES_MAX];
extern const TEventHandler sht3xTransitionTable[SHT3X_STATES_MAX][SHT3X_SIG_MAX];
static TEvent events[SHT3X_QUEUE_MAX_CAPACITY];

/** @brief sht3x environment sensor Active Object */
static TSHT3xActiveObject sht3xAO;

/** SHT3X Local Functions */

static DRV_HANDLE _openI2CDriver(void) {
    DRV_HANDLE drvI2CHandle = DRV_I2C_Open(DRV_I2C_INDEX_0,
                                           DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING |
                                           DRV_IO_INTENT_SHARED);
    return drvI2CHandle;
};

/** SHT3X Global Functions */

TActiveObject *SHT3X_Initialize(void) {
    // init super AO
    ActiveObject_Initialize(&sht3xAO.super, SHT3X_AO_ID, events, SHT3X_QUEUE_MAX_CAPACITY);
    sht3xAO.super.state = &sht3xStatesList[SHT3X_ST_INIT];

    // open I2C driver, get handler
    DRV_HANDLE drvI2CHandle = _openI2CDriver();

    // init AO fields
    sht3xAO.drvI2CHandle = drvI2CHandle;
    sht3xAO.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    sht3xAO.sensorRegs.status = 0;
    // TODO check that all fields are cleared

    // error on i2c driver open
    if (DRV_HANDLE_INVALID == drvI2CHandle) {
        ActiveObject_Dispatch(&sht3xAO.super, (TEvent) {.sig = SHT3X_ERROR});
    };

    // set I2C handler @see https://microchip-mplab-harmony.github.io/core/index.html?GUID-C99FBA78-A80D-40EE-B863-E40151E30C73
    DRV_I2C_TransferEventHandlerSet(
            sht3xAO.drvI2CHandle,
            SHT3X_TransferEventHandler,
            (uintptr_t) &sht3xAO
    );

    return (TActiveObject *) &sht3xAO;
}

void SHT3X_Deinitialize(void) {
    sht3xAO.super.state = NULL;
}

void SHT3X_Tasks(void) {
    if (NULL == sht3xAO.super.state) return; // not initialized yet

    const TEvent event = ActiveObject_ProcessQueue(&sht3xAO.super);
    if (SHT3X_NO_EVENT == event.sig) return;

    const TState *nextState = FSM_ProcessEventToNextStateFromTransitionTable(&sht3xAO.super, event, SHT3X_STATES_MAX,
                                                                             SHT3X_SIG_MAX, sht3xTransitionTable);

#ifdef __DEBUG
    // TODO replace by simplified macro: #STRINGIFY_VAR(VAR) ("#VAR") 
    SHT3X_SIG sig = event.sig;
    SHT3X_STATE name = nextState->name;
    SYS_DEBUG_PRINT(SYS_ERROR_INFO, "SHT3x Event: %s, Next State: %s\r\n", _debugEventSignals[sig], _debugStateNames[name]);
    // TODO SYS_DEBUG_PRINT(SYS_ERROR_INFO, "SHT3x Event: %s, Next State: %s\r\n",  STRINGIFY_VAR(sig), STRINGIFY_VAR(name));
#endif

    if (FSM_IsValidState(nextState)) FSM_TraverseAOToNextState(&sht3xAO.super, nextState);
};

/**
 * @see https://microchip-mplab-harmony.github.io/core/index.html?GUID-95F7ABE3-6864-4FC9-B11B-97B31ACF683C
 * @param[in] event
 * @param[out] transferHandle
 * @param[in] context
 */
void SHT3X_TransferEventHandler(
        DRV_I2C_TRANSFER_EVENT event,
        DRV_I2C_TRANSFER_HANDLE transferHandle,
        uintptr_t context
) {
    switch (event) {
        /* Transfer request is pending */
        case DRV_I2C_TRANSFER_EVENT_PENDING:
            return;

            /* All data from or to the buffer was transferred successfully. */
        case DRV_I2C_TRANSFER_EVENT_COMPLETE:
            return ActiveObject_Dispatch(&sht3xAO.super, (TEvent) {.sig = SHT3X_TRANSFER_SUCCESS});

            /* There was an error while processing the buffer transfer request. */
        case DRV_I2C_TRANSFER_EVENT_ERROR:
            return ActiveObject_Dispatch(&sht3xAO.super, (TEvent) {.sig = SHT3X_TRANSFER_FAIL});

            /* Transfer Handle given is expired. It means transfer
            is completed but with or without error is not known. */
        case DRV_I2C_TRANSFER_EVENT_HANDLE_EXPIRED:
        case DRV_I2C_TRANSFER_EVENT_HANDLE_INVALID:
            return ActiveObject_Dispatch(&sht3xAO.super, (TEvent) {.sig = SHT3X_ERROR});
        default:
            SYS_DEBUG_PRINT(SYS_ERROR_INFO, "SHT3X_TransferEventHandler: unknown event %d\n", event);
            return;
    };
};
