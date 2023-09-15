#include "./storage_manager.h"

#ifdef __DEBUG
static const char *const _debugStateNames[STORAGE_STATES_MAX] = {
    [STORAGE_NO_STATE] = "STORAGE_NO_STATE",
    [STORAGE_ST_INIT] = "STORAGE_ST_INIT",
    [STORAGE_ST_READ_BOOT_SECTOR] = "STORAGE_ST_READ_BOOT_SECTOR",
    [STORAGE_ST_VERIFY_BOOT_SECTOR] = "STORAGE_ST_VERIFY_BOOT_SECTOR",
    [STORAGE_ST_WRITE_BOOT_SECTOR] = "STORAGE_ST_WRITE_BOOT_SECTOR",
    [STORAGE_ST_FIND_LAST_NONEMPTY_PAGE] = "STORAGE_ST_FIND_LAST_NONEMPTY_PAGE",
    [STORAGE_ST_STORE_DATA_IN_TAIL] = "STORAGE_ST_STORE_DATA_IN_TAIL",
    [STORAGE_ST_IDLE] = "STORAGE_ST_IDLE",
    [STORAGE_ST_ERROR] = "STORAGE_ST_ERROR"
};

static const char *const _debugEventSignals[STORAGE_SIG_MAX] = {
    [STORAGE_NO_EVENT] = "STORAGE_NO_EVENT",
    [STORAGE_CHECK_MEMORY_BOOT_SECTOR] = "STORAGE_CHECK_MEMORY_BOOT_SECTOR",
    [STORAGE_WRITE_MEMORY_BOOT_SECTOR] = "STORAGE_WRITE_MEMORY_BOOT_SECTOR",
    [STORAGE_VERIFY_MEMORY_BOOT_SECTOR_SUCCESS] = "STORAGE_VERIFY_MEMORY_BOOT_SECTOR_SUCCESS",
    [STORAGE_FIND_LAST_NON_EMPTY_PAGE] = "STORAGE_FIND_LAST_NON_EMPTY_PAGE",
    [STORAGE_FIND_LAST_NON_EMPTY_PAGE_SUCCESS] = "STORAGE_FIND_LAST_NON_EMPTY_PAGE_SUCCESS",
    [STORAGE_STORE_DATA_IN_TAIL] = "STORAGE_STORE_DATA_IN_TAIL",
    [STORAGE_TRANSFER_SUCCESS] = "STORAGE_TRANSFER_SUCCESS",
    [STORAGE_TRANSFER_FAIL] = "STORAGE_TRANSFER_FAIL",
    [STORAGE_ERROR] = "STORAGE_ERROR"
};
#endif

extern const TState storageStatesList[STORAGE_STATES_MAX];
extern const TEventHandler storageTransitionTable[STORAGE_STATES_MAX][STORAGE_SIG_MAX];
static TEvent events[STORAGE_QUEUE_MAX_CAPACITY];
static TSTORAGEActiveObject storageAO;

TActiveObject *STORAGE_Initialize(void) {
    // init super AO
    ActiveObject_Initialize(&storageAO.super, STORAGE_AO_ID, events, STORAGE_QUEUE_MAX_CAPACITY);
    storageAO.super.state = &storageStatesList[STORAGE_ST_INIT];

    // open MEMORY driver, get handler
    DRV_HANDLE drvMemoryHandle = DRV_MEMORY_Open(sysObj.drvMemory1, /* TODO make it 0 driver*/
                                                 DRV_IO_INTENT_READWRITE | DRV_IO_INTENT_NONBLOCKING);

    // init AO fields
    storageAO.drvMemoryHandle = drvMemoryHandle;
    storageAO.transferHandle = DRV_I2C_TRANSFER_HANDLE_INVALID;
    storageAO.pagesToProcessAmount = 0;
    storageAO.page = 0;
    STORAGE_CLearPageBuffer(&storageAO);

    // error on driver opening error
    if (DRV_HANDLE_INVALID == storageAO.drvMemoryHandle) {
        ActiveObject_Dispatch(&storageAO.super, (TEvent) {.sig = STORAGE_ERROR});
    }

    // set MEMORY handler
    DRV_MEMORY_TransferHandlerSet(
            storageAO.drvMemoryHandle,
            STORAGE_TransferEventHandler,
            (uintptr_t) &storageAO
    );

    return (TActiveObject *) &storageAO;
};

void STORAGE_Deinitialize(void) {
    storageAO.super.state = NULL;
    DRV_MEMORY_Close(storageAO.drvMemoryHandle);
};

void STORAGE_Tasks(void) {
    if (NULL == storageAO.super.state) return; // not initialized yet

    const TEvent event = ActiveObject_ProcessQueue(&storageAO.super);
    if (STORAGE_NO_EVENT == event.sig) return;

    const TState *nextState = FSM_ProcessEventToNextState(&storageAO.super, event, STORAGE_STATES_MAX, STORAGE_SIG_MAX,
                                                          storageStatesList, storageTransitionTable);

#ifdef __DEBUG
    STORAGE_SIG sig = event.sig;
    STORAGE_STATE name = nextState->name;
    SYS_DEBUG_PRINT(SYS_ERROR_INFO, "STORAGE Event: %s, Next State: %s\r\n", _debugEventSignals[sig], _debugStateNames[name]);
#endif

    if (FSM_IsValidState(nextState)) FSM_TraverseNextState(&storageAO.super, nextState);
};

void STORAGE_CLearPageBuffer(TSTORAGEActiveObject *const AO) {
    memset(AO->pageBuffer, 0, DRV_AT25DF_PAGE_SIZE);
}

void STORAGE_TransferEventHandler(DRV_MEMORY_EVENT event, DRV_MEMORY_COMMAND_HANDLE commandHandle, uintptr_t context) {
    switch (event) {
        case DRV_MEMORY_EVENT_COMMAND_COMPLETE: {
            return ActiveObject_Dispatch((TActiveObject *) context, (TEvent) {.sig = STORAGE_TRANSFER_SUCCESS});
        }
        case DRV_MEMORY_EVENT_COMMAND_ERROR: {
            return ActiveObject_Dispatch((TActiveObject *) context, (TEvent) {.sig = STORAGE_TRANSFER_FAIL});
        }
        default: {
            break;
        }
    }
}