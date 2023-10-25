#include "./nfc.h"

extern const TState nfcStatesList[NFC_STATES_MAX];
extern const TEventHandler nfcTransitionTable[NFC_STATES_MAX][NFC_SIG_MAX];
static TEvent events[NFC_QUEUE_MAX_CAPACITY];

/** @brief nfc Active Object */
static TNFCActiveObject nfcAO;

/** NFC Local Functions */

static DRV_HANDLE _openI2CDriver(void);

static void _onNFCGPOPinChange(uintptr_t context);

/* NFC Global Functions */

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
    memset(nfcAO.st25dvRegs.pwd, 0x00, NFC_PASSWORD_SIZE); // factory default password is 0x00
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

    // Register callback for NFC GPO fall events (RF presence / absence)
    EIC_CallbackRegister(EIC_PIN_3, _onNFCGPOPinChange, (uintptr_t) &nfcAO);

    return (TActiveObject *) &nfcAO;
};

void NFC_Deinitialize(void) {
    nfcAO.super.state = NULL;
    // TODO check should we close I2C driver here?
}

void NFC_Tasks(void) {
    if (NULL == nfcAO.super.state) return; // not initialized yet

    const TEvent event = ActiveObject_ProcessQueue(&nfcAO.super);
    if (NFC_NO_EVENT == event.sig) return;

    const TState *nextState = FSM_ProcessEventToNextStateFromTransitionTable(&nfcAO.super, event, NFC_STATES_MAX,
                                                                             NFC_SIG_MAX, nfcTransitionTable);

    if (FSM_IsValidState(nextState)) FSM_TraverseAOToNextState(&nfcAO.super, nextState);

#ifdef __DEBUG
    // TODO replace by simplified macro: #STRINGIFY_VAR(VAR) ("#VAR")
//    int sig = event.sig;
//    int name = nextState->name;
//    SYS_DEBUG_PRINT(SYS_ERROR_INFO, "NFC Event: %d, Next State: %d\n", sig, name);
#endif
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

static DRV_HANDLE _openI2CDriver(void) {
    DRV_HANDLE drvI2CHandle = DRV_I2C_Open(DRV_I2C_INDEX_0,
                                           DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING |
                                           DRV_IO_INTENT_SHARED);

    return drvI2CHandle;
};

/** @brief FIELD_CHANGE_EN: A pulse is emitted on GPO, when RF field appears or disappears */
static void _onNFCGPOPinChange(uintptr_t context) {
//    static volatile uint8_t gpo = 0;
    SYS_DEBUG_PRINT(SYS_ERROR_INFO, "GPO %d\n");
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) context;
    ActiveObject_Dispatch(&nfcAO->super, (TEvent) {.sig = NFC_GPO_PULSE});
};
