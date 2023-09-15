#include "./nfc.h"

#ifdef __DEBUG
static const char *const _debugStateNames[NFC_STATES_MAX] = {
    [NFC_NO_STATE] = "NFC_NO_STATE",
    [NFC_ST_INIT] = "NFC_ST_INIT",
    [NFC_ST_IDLE] = "NFC_ST_IDLE",
    [NFC_ST_READ_UID] = "NFC_ST_READ_UID",
    [NFC_ST_PRESENT_I2C_PWD] = "NFC_ST_PRESENT_I2C_PWD",
    [NFC_ST_ALLOW_MB_MODE_WRITE] = "NFC_ST_ALLOW_MB_MODE_WRITE",
    [NFC_ST_ENABLE_FT_MODE] = "NFC_ST_ENABLE_FT_MODE",
    [NFC_ST_WRITE_MAILBOX] = "NFC_ST_WRITE_MAILBOX",
    [NFC_ST_ERROR] = "NFC_ST_ERROR"
};

static const char *const _debugEventSignals[NFC_SIG_MAX] = {
    [NFC_NO_EVENT] = "NFC_NO_EVENT",
    [NFC_TRANSFER_SUCCESS] = "NFC_TRANSFER_SUCCESS",
    [NFC_TRANSFER_FAIL] = "NFC_TRANSFER_FAIL",
    [NFC_TRANSFER_TIMEOUT] = "NFC_TRANSFER_TIMEOUT",
    [NFC_TRANSFER_MAX_RETRIES] = "NFC_TRANSFER_MAX_RETRIES",
    [NFC_READ_UID] = "NFC_READ_UID",
    [NFC_PRESENT_I2C_PWD] = "NFC_PRESENT_I2C_PWD",
    [NFC_ALLOW_MB_MODE_WRITE] = "NFC_ALLOW_MB_MODE_WRITE",
    [NFC_ENABLE_FT_MODE] = "NFC_ENABLE_FT_MODE",
    [NFC_WRITE_MAILBOX] = "NFC_WRITE_MAILBOX",
    [NFC_PREPARE_MAILBOX] = "NFC_PREPARE_MAILBOX",
    [NFC_ERROR] = "NFC_ERROR"
};
#endif

extern const TState nfcStatesList[NFC_STATES_MAX];
extern const TEventHandler nfcTransitionTable[NFC_STATES_MAX][NFC_SIG_MAX];
static TEvent events[NFC_QUEUE_MAX_CAPACITY];

/** @brief nfc Active Object */
static TNFCActiveObject nfcAO;

/** NFC Local Functions */

static DRV_HANDLE _openI2CDriver(void) {
    DRV_HANDLE drvI2CHandle = DRV_I2C_Open(DRV_I2C_INDEX_0,
                                           DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING |
                                           DRV_IO_INTENT_SHARED);

    return drvI2CHandle;
};

/** NFC Global Functions */

TActiveObject *NFC_Initialize(void) {
    // init super AO
    ActiveObject_Initialize(&nfcAO.super, NFC_AO_ID, events, NFC_QUEUE_MAX_CAPACITY);
    nfcAO.super.state = &nfcStatesList[NFC_ST_INIT];

    // open I2C driver, get handler
    DRV_HANDLE drvI2CHandle = _openI2CDriver();

    // init NFC AO fields
    nfcAO.drvI2CHandle = drvI2CHandle;
    nfcAO.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    nfcAO.retriesLeft = NFC_TRANSFER_RETRIES_MAX;
    // TODO check that all fields are cleared

    // error on driver opening error
    if (DRV_HANDLE_INVALID == drvI2CHandle) {
        ActiveObject_Dispatch(&nfcAO.super, (TEvent) {.sig = NFC_ERROR});
    };

    // set I2C handler @see https://microchip-mplab-harmony.github.io/core/index.html?GUID-C99FBA78-A80D-40EE-B863-E40151E30C73
    DRV_I2C_TransferEventHandlerSet(
            nfcAO.drvI2CHandle,
            NFC_TransferEventHandler,
            (uintptr_t) &nfcAO
    );

    return (TActiveObject *) &nfcAO;
};

void NFC_Deinitialize(void) {
    nfcAO.super.state = NULL;
}

void NFC_Tasks(void) {
    if (NULL == nfcAO.super.state) return; // not initialized yet

    const TEvent event = ActiveObject_ProcessQueue(&nfcAO.super);
    if (NFC_NO_EVENT == event.sig) return;

    const TState *nextState = FSM_ProcessEventToNextState(&nfcAO.super, event, NFC_STATES_MAX, NFC_SIG_MAX,
                                                          nfcStatesList, nfcTransitionTable);

#ifdef __DEBUG
    NFC_SIG sig = event.sig;
    NFC_STATE name = nextState->name;
    SYS_DEBUG_PRINT(SYS_ERROR_INFO, "NFC Event: %s, Next State: %s\r\n", _debugEventSignals[sig], _debugStateNames[name]);
#endif

    if (FSM_IsValidState(nextState)) FSM_TraverseNextState(&nfcAO.super, nextState);
}

/**
* @see https://microchip-mplab-harmony.github.io/core/index.html?GUID-95F7ABE3-6864-4FC9-B11B-97B31ACF683C
* @param[in] event
* @param[out] transferHandle
* @param[in] context - event to emit on success
*/
void NFC_TransferEventHandler(
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
            return ActiveObject_Dispatch(&nfcAO.super, (TEvent) {.sig = NFC_TRANSFER_SUCCESS});

            /* There was an error while processing the buffer transfer request. */
        case DRV_I2C_TRANSFER_EVENT_ERROR:
            return ActiveObject_Dispatch(&nfcAO.super, (TEvent) {.sig = NFC_TRANSFER_FAIL});

            /* Transfer Handle given is expired. It means transfer
            is completed but with or without error is not known. */
        case DRV_I2C_TRANSFER_EVENT_HANDLE_EXPIRED:
        case DRV_I2C_TRANSFER_EVENT_HANDLE_INVALID:
            return ActiveObject_Dispatch(&nfcAO.super, (TEvent) {.sig = NFC_ERROR});
        default:
            SYS_DEBUG_PRINT(SYS_ERROR_INFO, "NFC_TransferEventHandler: unknown event %d\n", event);
            return;
    };
};
