#include "./app_manager.h"

static TActiveObject appAO;
static TEvent events[APP_QUEUE_MAX_CAPACITY];

static const TState *_processAppManagerFSM(TActiveObject *AO, TEvent event);

extern TActiveObject systemActorsList[ACTIVE_OBJECTS_MAX];

/* states */
const TState appAOStatesList[APP_STATES_MAX] = {
        [APP_ST_NFC_AND_SENSORS] =      {.name = APP_ST_NFC_AND_SENSORS},
        [APP_ST_USB_ONLY] =             {.name = APP_ST_USB_ONLY},
        [APP_ST_NFC_ONLY] =             {.name = APP_ST_NFC_ONLY},
};

TActiveObject *APP_Initialize(void) {
    // init super AO
    ActiveObject_Initialize(&appAO, MAIN_APP_AO_ID, events, APP_QUEUE_MAX_CAPACITY);
    appAO.state = &appAOStatesList[APP_ST_NFC_AND_SENSORS];

    return (TActiveObject *) &appAO;
}

void APP_Tasks(void) {
    const TEvent event = ActiveObject_ProcessQueue(&appAO);

    // Perform main app tasks
    switch (appAO.state->name) {
        case APP_ST_NFC_AND_SENSORS:
            // SHT3X_Tasks();
            // TODO AMBIENT_LIGHT_Tasks();
            // TODO ACCELEROMETER_Tasks();
            STORAGE_Tasks();
            NFC_Tasks();
            break;
        case APP_ST_NFC_ONLY:
            STORAGE_Tasks();
            NFC_Tasks();
            break;
        case APP_ST_USB_ONLY:
            USB_Tasks();
            break;
    }

    // switch main app state on event received
    if (event.sig) {
        const TState *nextState = _processAppManagerFSM(&appAO, event);
        appAO.state = nextState;
    }
}

static const TState *_processAppManagerFSM(TActiveObject *appAO, TEvent event) {
    TActiveObject* initAO = &systemActorsList[INIT_AO_ID];
    switch (event.sig) {
        // handle USB cable event
        case APP_SIG_USB_CABLE_CONNECTED:
            ActiveObject_Dispatch(initAO, (TEvent) {.sig = DEINIT_SIG_STORAGE});
            return &appAOStatesList[APP_ST_USB_ONLY];
        case APP_SIG_USB_CABLE_DISCONNECTED:
            ActiveObject_Dispatch(initAO, (TEvent) {.sig = INIT_SIG_STORAGE});
            return &appAOStatesList[APP_ST_NFC_AND_SENSORS];
        // Handle phone (NFC RF field) event
        case APP_SIG_NFC_RF_FIELD_APPEARS: // TODO resolve situation when NFC and USB appears at the same time
            return &appAOStatesList[APP_ST_NFC_ONLY];
        case APP_SIG_NFC_RF_FIELD_DISAPPEAR:
            return &appAOStatesList[APP_ST_NFC_AND_SENSORS];
        default:    
            return &appAOStatesList[APP_ST_NFC_AND_SENSORS];
    }
}
