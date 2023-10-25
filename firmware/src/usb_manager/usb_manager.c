#include "./usb_manager.h"

//static void _onVUSBChange(uintptr_t context);

void USB_Initialize (void) {
    // Register callback for NFC GPO fall events (RF presence / absence)
//    EIC_CallbackRegister(EIC_PIN_?, _onVUSBChange, NULL);
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

//static void _onVUSBChange(uintptr_t context) {
//    // TODO
//};
