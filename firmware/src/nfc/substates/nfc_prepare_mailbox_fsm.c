/**
 * @brief NFC Prepare Mailbox sub FSM
 * @details Mailbox MODE access for W when I2C security session is open
*/

#include "../nfc.h"

static const uint8_t ST25DV_MB_CTRL_DYN_REG[] = {0x20, 0x06};
static const uint8_t ST25DV_GPO_REG[] = {0x00, 0x00};
static const uint8_t ST25DV_MB_MODE_REG[] = {0x00, 0x0D};
static const uint8_t ST25DV_I2C_PWD_REG[] = {0x09, 0x00};

// states are placed in order of execution to prepare NFC mailbox
typedef enum {
    NFC_PREPARE_MAILBOX_ST_INIT = 0,
    NFC_PREPARE_MAILBOX_ST_PRESENT_I2C_PWD,
    NFC_PREPARE_MAILBOX_ST_ALLOW_MB_MODE_WRITE,
    NFC_PREPARE_MAILBOX_ST_ENABLE_FT_MODE,
    NFC_PREPARE_MAILBOX_ST_GPO_PULSE_ON_RF,
    NFC_PREPARE_MAILBOX_ST_FT_MODE_ENABLED
} NFC_PREPARE_MAILBOX_STATE;

static void _presentI2CPwd(TNFCActiveObject *const nfcAO);

static void _allowMBModeWrite(TNFCActiveObject *const nfcAO);

static void _enableFTMode(TNFCActiveObject *const nfcAO);

static void _enableGPOPulseOnRF(TNFCActiveObject *const nfcAO);

void NFC_ProcessPrepareMailboxFSM(TNFCActiveObject *const nfcAO, TEvent event) {
    static NFC_PREPARE_MAILBOX_STATE FTModeState = NFC_PREPARE_MAILBOX_ST_INIT;

    if (NFC_I2C_TRANSFER_SUCCESS == event.sig) {
        FTModeState++; // go to next state in chain @see NFC_PREPARE_MAILBOX_STATE
    }

    /* on NFC_I2C_TRANSFER_FAIL the state remains the same, and we retry appropriate state handler function */

    switch (FTModeState) {
        case NFC_PREPARE_MAILBOX_ST_PRESENT_I2C_PWD:
            _presentI2CPwd(nfcAO);
            break;
        case NFC_PREPARE_MAILBOX_ST_ALLOW_MB_MODE_WRITE:
            _allowMBModeWrite(nfcAO);
            break;
        case NFC_PREPARE_MAILBOX_ST_ENABLE_FT_MODE:
            _enableFTMode(nfcAO);
            break;
        case NFC_PREPARE_MAILBOX_ST_GPO_PULSE_ON_RF:
            _enableGPOPulseOnRF(nfcAO);
            break;
        case NFC_PREPARE_MAILBOX_ST_FT_MODE_ENABLED:
            ActiveObject_Dispatch(&nfcAO->super,
                                  (TEvent) {.sig = NFC_PREPARE_MAILBOX_SUCCESS}); // notify parent FSM that mailbox is ready
            break;
        default:
            break; // TODO assert?
    }
}

/** @brief  Presents I2C password, to authorize the I2C writes to protected areas. E.g. MB_CTRL_Dyn*/
static void _presentI2CPwd(TNFCActiveObject *const nfcAO) {
    memset(nfcAO->transferBuf.raw, 0, NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE);

    memcpy(nfcAO->transferBuf.cmd, ST25DV_I2C_PWD_REG, NFC_CMD_SIZE);

    /* Build I2C Message with Password + Validation code 0x09 + Password */
    for (uint8_t i = 0; i < NFC_PASSWORD_SIZE; i++) {
        nfcAO->transferBuf.mailbox[i] = nfcAO->st25dvRegs.pwd[i];
        nfcAO->transferBuf.mailbox[i + NFC_PASSWORD_SIZE + NFC_PASSWORD_VALIDATION_SIZE] = nfcAO->st25dvRegs.pwd[i];
    };
    nfcAO->transferBuf.mailbox[NFC_PASSWORD_VALIDATION_INDEX] = 0x09;

    DRV_I2C_WriteTransferAdd(
            nfcAO->drvI2CHandle,
            ST25DV_ADDR_SYST_I2C,
            nfcAO->transferBuf.raw,
            NFC_CMD_SIZE + NFC_PASSWORD_SIZE + NFC_PASSWORD_VALIDATION_SIZE + NFC_PASSWORD_SIZE,
            &(nfcAO->transferHandle)
    );

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);
    nfcAO->retriesLeft--;
    NFC_VerifyRetries(nfcAO);
};

/** @brief  Allows to write to MB_MODE register */
static void _allowMBModeWrite(TNFCActiveObject *const nfcAO) {
    memset(nfcAO->transferBuf.raw, 0, NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE);

    memcpy(nfcAO->transferBuf.cmd, ST25DV_MB_MODE_REG, NFC_CMD_SIZE);
    nfcAO->transferBuf.mailbox[NFC_MAILBOX_HEAD] = ST25DV_MB_MODE_RW_MASK;

    DRV_I2C_WriteTransferAdd(
            nfcAO->drvI2CHandle,
            ST25DV_ADDR_SYST_I2C,
            nfcAO->transferBuf.raw,
            NFC_CMD_SIZE + NFC_SINGLE_BYTE_REG_SIZE,
            &(nfcAO->transferHandle)
    );

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);
    nfcAO->retriesLeft--;
    NFC_VerifyRetries(nfcAO);
};

/** @brief  Enables Fast Transfer Mode */
static void _enableFTMode(TNFCActiveObject *const nfcAO) {
    memset(nfcAO->transferBuf.raw, 0, NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE);

    memcpy(nfcAO->transferBuf.cmd, ST25DV_MB_CTRL_DYN_REG, NFC_CMD_SIZE);
    nfcAO->transferBuf.mailbox[NFC_MAILBOX_HEAD] = ST25DV_MB_CTRL_DYN_MBEN_MASK >> ST25DV_MB_CTRL_DYN_MBEN_SHIFT;

    DRV_I2C_WriteTransferAdd(
            nfcAO->drvI2CHandle,
            ST25DV_ADDR_DATA_I2C,
            nfcAO->transferBuf.raw,
            NFC_CMD_SIZE + NFC_SINGLE_BYTE_REG_SIZE,
            &(nfcAO->transferHandle)
    );

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);
    nfcAO->retriesLeft--;
    NFC_VerifyRetries(nfcAO);
};

/** @brief  Enables GPO pulse on RF Get/Put message (factory default disabled) */
static void _enableGPOPulseOnRF(TNFCActiveObject *const nfcAO) {
    memset(nfcAO->transferBuf.raw, 0, NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE);

    memcpy(nfcAO->transferBuf.cmd, ST25DV_GPO_REG, NFC_CMD_SIZE);
    nfcAO->transferBuf.mailbox[NFC_MAILBOX_HEAD] =
            ST25DV_GPO_ENABLE_MASK | ST25DV_GPO_RFPUTMSG_MASK | ST25DV_GPO_RFGETMSG_MASK | ST25DV_GPO_FIELDCHANGE_MASK;

    DRV_I2C_WriteTransferAdd(
            nfcAO->drvI2CHandle,
            ST25DV_ADDR_SYST_I2C,
            nfcAO->transferBuf.raw,
            NFC_CMD_SIZE + NFC_SINGLE_BYTE_REG_SIZE,
            &(nfcAO->transferHandle)
    );

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);
    nfcAO->retriesLeft--;
    NFC_VerifyRetries(nfcAO);
}

//static void _readGPODyn(TNFCActiveObject *const nfcAO) {
//
//    memset(nfcAO->transferBuf.raw, 0, NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE);
//
//    memcpy(nfcAO->transferBuf.cmd, ST25DV_GPO_REG, NFC_CMD_SIZE);
//
//    DRV_I2C_WriteReadTransferAdd(
//            nfcAO->drvI2CHandle,
//            ST25DV_ADDR_SYST_I2C,
//            nfcAO->transferBuf.raw,
//            NFC_CMD_SIZE,
//            &gpoDyn,
//            1,
//            &(nfcAO->transferHandle)
//    );
//
//    NFC_DispatchErrorOnInvalidTransfer(nfcAO);
//    nfcAO->retriesLeft--;
//    NFC_VerifyRetries(nfcAO);
//}
