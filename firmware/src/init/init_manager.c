#include "./init_manager.h"

/** @brief All Active Objects in system list */
TActiveObject *systemActorsList[ACTIVE_OBJECTS_MAX] = {
        [INIT_AO_ID] = NULL,
        [STORAGE_AO_ID] = NULL,
        [NFC_AO_ID] = NULL,
        [SHT3X_AO_ID] = NULL,
        [AMBIENT_LIGHT_AO_ID] = NULL,
        [ACCELEROMETER_AO_ID] = NULL
};

static TInitActiveObject initAO;
static TEvent events[INIT_QUEUE_MAX_CAPACITY];

static const TState *_processInitManagerFSM(TInitActiveObject *AO, TEvent event);

/* states */
const TState initAOStatesList[INIT_STATES_MAX] = {
        [INIT_NO_STATE] = {.name = INIT_NO_STATE},
        [INIT_ST_INIT] = {.name = INIT_ST_INIT},
        [INIT_ST_IDLE] = {.name = INIT_ST_IDLE}
};

void INIT_Initialize(void) {
    systemActorsList[INIT_AO_ID] = (TActiveObject *) &initAO; // place to global AO list
    ActiveObject_Initialize(&initAO.super, INIT_AO_ID, events, INIT_QUEUE_MAX_CAPACITY);

    // init sensors on next cycle
    ActiveObject_Dispatch(&initAO.super, (TEvent) {.sig = INIT_SIG_SENSORS});
    // init NFC on next cycle
    ActiveObject_Dispatch(&initAO.super, (TEvent) {.sig = INIT_SIG_NFC});
    // init storage on next cycle
    ActiveObject_Dispatch(&initAO.super, (TEvent) {.sig = INIT_SIG_STORAGE});

    initAO.super.state = &initAOStatesList[INIT_ST_INIT];
}

void INIT_Tasks(void) {
    if (NULL == initAO.super.state) return; // not initialized yet

    const TEvent event = ActiveObject_ProcessQueue(&initAO.super);
    if (INIT_NO_EVENT == event.sig) return;

    const TState *nextState = _processInitManagerFSM(&initAO, event);
    initAO.super.state = nextState;
}

static const TState *_processInitManagerFSM(TInitActiveObject *AO, TEvent event) {
    switch (event.sig) {
        case INIT_SIG_SENSORS:
            systemActorsList[SHT3X_AO_ID] = SHT3X_Initialize();
            ActiveObject_Dispatch(systemActorsList[SHT3X_AO_ID],
                                  (TEvent) {.sig = SHT3X_READ_STATUS}); // TODO rename to emphasize self-test procedure
            // TODO init ambient light, accelerometer
            return &initAOStatesList[INIT_ST_IDLE];
        case INIT_SIG_NFC:
            systemActorsList[NFC_AO_ID] = NFC_Initialize();
            ActiveObject_Dispatch(systemActorsList[NFC_AO_ID],
                                  (TEvent) {.sig = NFC_READ_UID}); // TODO rename to emphasize self-test procedure
            return &initAOStatesList[INIT_ST_IDLE];
        case INIT_SIG_STORAGE:
            systemActorsList[STORAGE_AO_ID] = STORAGE_Initialize();
            ActiveObject_Dispatch(systemActorsList[STORAGE_AO_ID], (TEvent) {.sig = STORAGE_CHECK_MEMORY_BOOT_SECTOR});
            return &initAOStatesList[INIT_ST_IDLE];
        case DEINIT_SIG_SENSORS:
            SHT3X_Deinitialize();
            // TODO deinit ambient light, accelerometer
            return &initAOStatesList[INIT_ST_IDLE];
        case DEINIT_SIG_NFC:
            NFC_Deinitialize();
            return &initAOStatesList[INIT_ST_IDLE];
        case DEINIT_SIG_STORAGE:
            // important to release MEMORY driver before USB MSD usage
            STORAGE_Deinitialize();
            return &initAOStatesList[INIT_ST_IDLE];
        default:
            return &initAOStatesList[INIT_ST_IDLE];
    }
}
