#include "./usb_manager.h"

extern TActiveObject systemActorsList[ACTIVE_OBJECTS_MAX];

static void _onVUSBChange(uintptr_t context);

void USB_Initialize (void) {
    // Register callback for USB_VBUS_SENSE pin change events (USB cable connect / disconnect)
    EIC_CallbackRegister(EIC_PIN_15, _onVUSBChange, (uintptr_t)NULL);
}

void USB_Tasks(void) {
    /* Maintain system services */
    SYS_CONSOLE_Tasks(SYS_CONSOLE_INDEX_0);

    /* Maintain Middleware & Other Libraries */
    /* USB Device layer tasks routine */
    USB_DEVICE_Tasks(sysObj.usbDevObject0);

    /* USB FS Driver Task Routine */
    DRV_USBFSV1_Tasks(sysObj.drvUSBFSV1Object);
}

static void _onVUSBChange(uintptr_t context) {
    TActiveObject* mainAppAO = &systemActorsList[MAIN_APP_AO_ID];
    bool usbCableConnected = USB_VBUS_SENSE_Get();

    if (usbCableConnected) {
        ActiveObject_Dispatch(mainAppAO, (TEvent) {.sig = APP_SIG_USB_CABLE_CONNECTED});
    } else {
        ActiveObject_Dispatch(mainAppAO, (TEvent) {.sig = APP_SIG_USB_CABLE_DISCONNECTED});
    }
};
