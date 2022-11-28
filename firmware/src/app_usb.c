/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <usb/usb_device_cdc.h>
#include "app_usb.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************

uint8_t CACHE_ALIGN cdcReadBuffer[APP_USB_READ_BUFFER_SIZE];
uint8_t CACHE_ALIGN cdcWriteBuffer[APP_USB_WRITE_BUFFER_SIZE];

/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_USB_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_USB_DATA app_usbData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/

USB_DEVICE_CDC_EVENT_RESPONSE APP_USB_USBDeviceCDCEventHandler
(
    USB_DEVICE_CDC_INDEX index,
    USB_DEVICE_CDC_EVENT event,
    void * pData,
    uintptr_t userData
)
{
    APP_USB_DATA * appDataObject;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;
    USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE * eventDataRead;
    
    appDataObject = (APP_USB_DATA *)userData;

    switch(event)
    {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  
             */

            USB_DEVICE_ControlSend(appDataObject->deviceHandle,
                    &appDataObject->getLineCodingData,
                    sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host 
             */

            USB_DEVICE_ControlReceive(appDataObject->deviceHandle,
                    &appDataObject->setLineCodingData,
                    sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. 
             */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *)pData;
            appDataObject->controlLineStateData.dtr = controlLineStateData->dtr;
            appDataObject->controlLineStateData.carrier =
                    controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle,
                    USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration 
             */

            appDataObject->breakData = ((USB_DEVICE_CDC_EVENT_DATA_SEND_BREAK *)
                    pData)->breakDuration;
            
            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(appDataObject->deviceHandle,
                    USB_DEVICE_CONTROL_STATUS_OK);
            
            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data, store the same */
            eventDataRead = (USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *)pData;
            
            /* Notify state machine that a read was completed */
            
            
            /* Update the number of bytes read */
            appDataObject->numBytesRead = eventDataRead->length;
            
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data 
             */

            USB_DEVICE_ControlStatus(appDataObject->deviceHandle,
                    USB_DEVICE_CONTROL_STATUS_OK);
            
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We don't
             * do much with this data in this demo.
             */
            
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            /* This means that the data write got completed and we can schedule
             * the next read. Notify the state machine by setting a flag. 
             */

            /* Notify state machine that a write was completed */
            
            
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/***********************************************
 * Application USB Device Layer Event Handler.
 ***********************************************/
void APP_USB_USBDeviceEventHandler 
(
    USB_DEVICE_EVENT event, 
    void * eventData, 
    uintptr_t context 
)
{
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;

    switch(event)
    {
        case USB_DEVICE_EVENT_SOF:

            app_usbData.sofEventHasOccurred = true;
            
            break;

        case USB_DEVICE_EVENT_RESET:

            /* Update LED to show reset state */
            USB_LED_Set();

            app_usbData.isConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*)eventData;
            
            if ( configuredEventData->configurationValue == 1)
            {
                /* Mark that the device is now configured */
                

                /* Update LED to show configured state */
                

                /* Register the CDC Device application event handler here.
                 * Note how the appData object pointer is passed as the
                 * user data 
                 */
                

            }
            
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(app_usbData.deviceHandle);
            
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(app_usbData.deviceHandle);
            
            USB_LED_Set();
            
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            USB_LED_Set();
            
            break;

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
        default:
            
            break;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/*
 ****************************************************
 * This function is called in every step of the     *
 * application state machine.                       *
 ****************************************************
 */

bool APP_USB_StateReset(void)
{
    /* This function returns true if the device was reset */

    bool retVal;

    if(app_usbData.isConfigured == false)
    {
        app_usbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;
        app_usbData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        app_usbData.writeTransferHandle =
                USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        app_usbData.isReadComplete = true;
        app_usbData.isWriteComplete = true;
        retVal = true;
    }
    else
    {
        retVal = false;
    }

    return(retVal);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_USB_Initialize ( void )

  Remarks:
    See prototype in app_usb.h.
 */

void APP_USB_Initialize ( void )
{
    /* Place the state machine in its initial state. */
    app_usbData.state = APP_USB_STATE_INIT;

    /* Device Layer Handle  */
    app_usbData.deviceHandle = USB_DEVICE_HANDLE_INVALID ;

    /* Device configured status */
    app_usbData.isConfigured = false;

    /* Initial get line coding state */
    app_usbData.getLineCodingData.dwDTERate = 9600;
    app_usbData.getLineCodingData.bParityType = 0;
    app_usbData.getLineCodingData.bParityType = 0;
    app_usbData.getLineCodingData.bDataBits = 8;

    /* Read Transfer Handle */
    app_usbData.readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Write Transfer Handle */
    app_usbData.writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

    /* Initialize the read complete flag */
    app_usbData.isReadComplete = true;

    /*Initialize the write complete flag*/
    app_usbData.isWriteComplete = true;

    /* Reset other flags */
    app_usbData.sofEventHasOccurred = false;

    /* Set up the read buffer */
    app_usbData.cdcReadBuffer = &cdcReadBuffer[0];

    /* Set up the read buffer */
    app_usbData.cdcWriteBuffer = &cdcWriteBuffer[0];
}


/******************************************************************************
  Function:
    void APP_USB_Tasks ( void )

  Remarks:
    See prototype in app_usb.h.
 */

void APP_USB_Tasks ( void )
{
    /* Update the application state machine based
     * on the current state 
     */
    
    switch(app_usbData.state)
    {
        case APP_USB_STATE_INIT:

            /* Open the device layer */
            

            if(app_usbData.deviceHandle != USB_DEVICE_HANDLE_INVALID)
            {
                /* Register a callback with device layer to get event 
                 * notification (for end point 0) 
                 */
                

                app_usbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;
            }
            else
            {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. 
                 */
            }

            break;

        case APP_USB_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device was configured */
            if(app_usbData.isConfigured)
            {
                /* If the device is configured then lets start reading */
                app_usbData.state = APP_USB_STATE_SCHEDULE_READ;
            }
            
            break;

        case APP_USB_STATE_SCHEDULE_READ:

            if(APP_USB_StateReset())
            {
                break;
            }

            /* If a read is complete, then schedule a read
             * else wait for the current read to complete 
             */
            app_usbData.state = APP_USB_STATE_WAIT_FOR_READ_COMPLETE;
            
            if(app_usbData.isReadComplete == true)
            {
                app_usbData.isReadComplete = false;
                app_usbData.readTransferHandle = 
                        USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
                
                /* Schedule read */
                
                
                if(app_usbData.readTransferHandle ==
                        USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID)
                {
                    app_usbData.state = APP_USB_STATE_ERROR;
                    break;
                }
            }

            break;

        case APP_USB_STATE_WAIT_FOR_READ_COMPLETE:

            if(APP_USB_StateReset())
            {
                break;
            }

            /* Check if a character was received, The isReadComplete flag gets 
             * updated in the CDC event handler. 
             */
            if(app_usbData.isReadComplete)
            {
                app_usbData.state = APP_USB_STATE_SCHEDULE_WRITE;
            }

            break;


        case APP_USB_STATE_SCHEDULE_WRITE:

            if(APP_USB_StateReset())
            {
                break;
            }

            /* Process received data and check if 
             * data matches a command and then process it.
             * Only the first character is considered.
             */
            switch (app_usbData.cdcReadBuffer[0])
            {
                /* h,H - Menu Command */
                case 'h':
                case 'H':
                    app_usbData.isCommand = true;
                    app_usbData.numBytesWrite =
                            sprintf((char*)app_usbData.cdcWriteBuffer,
                            "Getting Started Menu:\r\n"
                            "1 - Toggle board LED\r\n"
                            "2 - Show latest temperature value\r\n"
                            "3 - Show past 5 temperature values from EEPROM\r\n"
                            "4 - Show latest light sensor value\r\n"
                            "h - Show this menu\r\n");
                    
                    break;
                
                /* 1 - Led Toggle Command */
                case '1':
                    app_usbData.isCommand = true;
                    app_usbData.numBytesWrite =
                        sprintf((char*)app_usbData.cdcWriteBuffer,
                            "Toggled LED State\r\n");
                    
                    /*Toggle the E70 Xplained board LED*/
                    
                    
                    break;
                
                /* 2 - Display Temperature Command */
                case '2':
                    app_usbData.isCommand = true;
                    int8_t temp;
                    
                    /* Fetch the latest temperature from the sensor */
                    
                    
                    if (temp != INT8_MIN)
                    {
                        app_usbData.numBytesWrite = 
                            sprintf((char*)app_usbData.cdcWriteBuffer,
                            "Temperature = %d F\r\n",
                            temp);
                    }
                    else
                        app_usbData.numBytesWrite = 
                                sprintf((char*)app_usbData.cdcWriteBuffer,
                                    "Couldn't fetch temperature\r\n");
                    
                    break;
                
                /* 3 - Display Past 5 Temperatures Command */
                case '3':
                    app_usbData.isCommand = true;
                    int8_t temps[5], j;
                    uint32_t k = 0;
                    
                    /* Fetch last 5 temperature values from the EEPROM */
                    
                    
                    if (result)
                    {
                        for(j=0; j<5; j++)
                        {
                            k += sprintf((char*)&app_usbData.cdcWriteBuffer[k],
                                    "Temperature %d = %d F\r\n", j+1, temps[j]);
                        }
                        app_usbData.numBytesWrite = k;
                    }
                    else
                        app_usbData.numBytesWrite = 
                                sprintf((char*)app_usbData.cdcWriteBuffer,
                                    "Couldn't fetch temperature\r\n");
                    
                    break;
                
                /* 4 - Display Light Sensor Value Command */
                case '4':
                    app_usbData.isCommand = true;
                    uint8_t brightness = 0;
                    uint16_t adc_count = 0;
                    float adc_voltage = 0;
                    
                    /* Start ADC conversion */
                    AFEC1_ConversionStart();
                    
                    /* Wait till ADC conversion result is available */
                    while(!AFEC1_ChannelResultIsReady(AFEC_CH6))
                    {
                        /* Takes approximately 4 uS
                         * for the conversion to complete
                         */
                    };
                    
                    /* Read the ADC result */
                    
                    
                    /* Calculate voltage */
                    adc_voltage = (float)adc_count * 3.3 / 4095U;
                    
                    /* Calculate percentage */
                    brightness = (uint8_t) ((3.3 - adc_voltage)/3.3 * 100);
                    
                    /* Format result */
                    app_usbData.numBytesWrite = sprintf(
                            (char*)app_usbData.cdcWriteBuffer,
                            "Brightness = %d%%\r\n",
                            brightness);
                    
                    break;
                
                /* Do nothing */
                default:
                    /* Clear command flag */
                    app_usbData.isCommand = false;
                    
                    break;
            }
            
            /* Schedule write only if a valid command was processed */
            if(app_usbData.isCommand)
            {
                app_usbData.isWriteComplete = false;
                app_usbData.writeTransferHandle =
                        USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
                
                /* Schedule write */
                
                
                app_usbData.state = APP_USB_STATE_WAIT_FOR_WRITE_COMPLETE;
            }
            else
            {
                app_usbData.state = APP_USB_STATE_SCHEDULE_READ;
            }

            break;

        case APP_USB_STATE_WAIT_FOR_WRITE_COMPLETE:

            if(APP_USB_StateReset())
            {
                break;
            }

            /* Check if a character was sent. The isWriteComplete
             * flag gets updated in the CDC event handler 
             */

            if(app_usbData.isWriteComplete == true)
            {
                app_usbData.state = APP_USB_STATE_SCHEDULE_READ;
            }

            break;

        case APP_USB_STATE_ERROR:
        default:
            
            break;
    }
}


/*******************************************************************************
 End of File
 */
