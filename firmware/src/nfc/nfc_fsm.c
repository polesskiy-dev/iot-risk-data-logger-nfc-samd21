#include "./nfc.h"

// st25dv nfc commands registers
static const uint8_t ST25DV_UID_REG[] = {0x00, 0x18};
static const uint8_t ST25DV_MAILBOX_RAM_REG[] = {0x20, 0x08};
static const uint8_t ST25DV_ITSTS_DYN_REG[] = {0x20, 0x05}; // IT_STS_Dyn Interrupt status dynamic register

static const TState *_idle(TActiveObject *const AO, TEvent event);

static const TState *_readUID(TActiveObject *const AO, TEvent event);

static const TState *_prepareMailbox(TActiveObject *const AO, TEvent event);

static const TState *_writeMailbox(TActiveObject *const AO, TEvent event);

static const TState *_readInterruptStatus(TActiveObject *const AO, TEvent event);

static const TState *_handleInterruptStatus(TActiveObject *const AO, TEvent event);

static const TState *_error(TActiveObject *const AO, TEvent event);

static bool _refreshRetries(TActiveObject *const AO, void *const ctx);

//static bool _clearTransferBuf(TActiveObject *const AO, void *const ctx);

/** @brief checks if mailbox is empty */
static inline bool _isMailboxEmpty(TNFCActiveObject *const nfcAO) {
    for (uint16_t i = 0; i < ST25DV_MAILBOX_SIZE; i++)
        if (nfcAO->transferBuf.mailbox[i] != 0)
            return false;
    return true;
};

/* states */
const TState nfcStatesList[NFC_STATES_MAX] = {
        [NFC_ST_INIT] =                     {.name = NFC_ST_INIT},
        [NFC_ST_IDLE] =                     {.name = NFC_ST_IDLE},
        [NFC_ST_READ_UID] =                 {.name = NFC_ST_READ_UID},
        [NFC_ST_READ_INTERRUPT_STATUS] =    {.name = NFC_ST_READ_INTERRUPT_STATUS},
        [NFC_SUPER_ST_PREPARE_MAILBOX] =    {.name = NFC_SUPER_ST_PREPARE_MAILBOX, .onExit = _refreshRetries},
        [NFC_ST_WRITE_MAILBOX] =            {.name = NFC_ST_WRITE_MAILBOX, .onExit = _refreshRetries},
        [NFC_ST_READ_MAILBOX] =             {.name = NFC_ST_READ_MAILBOX, .onExit = _refreshRetries},
        [NFC_ST_ERROR] =                    {.name = NFC_ST_ERROR}
};

/* state transitions table */
const TEventHandler nfcTransitionTable[NFC_STATES_MAX][NFC_SIG_MAX] = {
        [NFC_ST_INIT]=                      {[NFC_READ_UID]=_readUID, [NFC_ERROR]=_error},
        [NFC_ST_IDLE]=                      {[NFC_GPO_PULSE]=_readInterruptStatus, [NFC_WRITE_MAILBOX]=_writeMailbox, [NFC_ERROR]=_error},
        [NFC_ST_READ_INTERRUPT_STATUS]=     {[NFC_TRANSFER_SUCCESS]=_handleInterruptStatus, /*[NFC_GPO_PULSE]=_readInterruptStatus*/ /*[NFC_TRANSFER_FAIL]=_error TODO */ [NFC_ERROR]=_error},
        [NFC_ST_READ_UID]=                  {[NFC_TRANSFER_SUCCESS]=_prepareMailbox, [NFC_TRANSFER_FAIL]=_error, [NFC_ERROR]=_error},
        /* Prepare mailbox (enable Fast Transfer mode) */
        [NFC_SUPER_ST_PREPARE_MAILBOX]=     {[NFC_PREPARE_MAILBOX_SUCCESS]=_idle, [NFC_TRANSFER_SUCCESS]=_prepareMailbox, [NFC_TRANSFER_FAIL]=_prepareMailbox, [NFC_TRANSFER_MAX_RETRIES]=_error, [NFC_ERROR]=_error},/* Check RF field */

        /* Mailbox (exchange data between I2C and RF) */
        [NFC_ST_WRITE_MAILBOX]=             {[NFC_TRANSFER_SUCCESS]=_idle, [NFC_ERROR]=_error},
        [NFC_ST_READ_MAILBOX]=              {[NFC_TRANSFER_SUCCESS]=_idle, [NFC_ERROR]=_error},

        [NFC_ST_ERROR]=                     {[NFC_ERROR]=_error},
};

static const TState *_idle(TActiveObject *const AO, TEvent event) {
    return &(nfcStatesList[NFC_ST_IDLE]);
};

static const TState *_readUID(TActiveObject *const AO, TEvent event) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    DRV_I2C_WriteReadTransferAdd(
            nfcAO->drvI2CHandle,
            ST25DV_ADDR_SYST_I2C,
            (void *const) &ST25DV_UID_REG,
            NFC_CMD_SIZE,
            &(nfcAO->st25dvRegs.uid),
            NFC_UID_SIZE,
            &(nfcAO->transferHandle)
    );

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_READ_UID]);
};

/** @brief  Write to Mailbox
 * @note don't forget to write to me->transferBuf.mailbox first,
 * */
static const TState *_writeMailbox(TActiveObject *const AO, TEvent event) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    nfcAO->retriesLeft--;
    memset(nfcAO->transferBuf.raw, 0, NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE);

    // TODO take data from event

    memcpy(nfcAO->transferBuf.cmd, ST25DV_MAILBOX_RAM_REG, NFC_CMD_SIZE);

    DRV_I2C_WriteTransferAdd(
            nfcAO->drvI2CHandle,
            ST25DV_ADDR_DATA_I2C,
            nfcAO->transferBuf.raw,
            NFC_CMD_SIZE + ST25DV_MAILBOX_SIZE,
            &(nfcAO->transferHandle)
    );

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_WRITE_MAILBOX]);
};

static const TState *_readInterruptStatus(TActiveObject *const AO, TEvent event) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    nfcAO->st25dvRegs.interruptStatus.raw = 0x00;

    DRV_I2C_WriteReadTransferAdd(
            nfcAO->drvI2CHandle,
            ST25DV_ADDR_DATA_I2C,
            (void *const) &ST25DV_ITSTS_DYN_REG,
            NFC_CMD_SIZE,
            &(nfcAO->st25dvRegs.interruptStatus.raw),
            NFC_ITSTS_SIZE,
            &(nfcAO->transferHandle)
    );

    NFC_DispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_READ_INTERRUPT_STATUS]);
}

static const TState *_handleInterruptStatus(TActiveObject *const AO, TEvent event) {
    static uint8_t invoked = 0;
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    if (nfcAO->st25dvRegs.interruptStatus.bitFields.RF_ACTIVITY) {
        // TODO handle RF activity
    };

    if (nfcAO->st25dvRegs.interruptStatus.bitFields.RF_INTERRUPT) {
        // TODO handle RF interrupt
    };

    if (nfcAO->st25dvRegs.interruptStatus.bitFields.FIELD_FALLING) {
        // TODO handle FIELD falling
    };

    if (nfcAO->st25dvRegs.interruptStatus.bitFields.FIELD_RISING) {
        // TODO handle FIELD rising
    };

    if (nfcAO->st25dvRegs.interruptStatus.bitFields.RF_PUT_MSG) {
        // TODO handle RF put message
    };

    if (nfcAO->st25dvRegs.interruptStatus.bitFields.RF_GET_MSG) {
        // TODO handle RF get message
    };

    if (nfcAO->st25dvRegs.interruptStatus.bitFields.RF_WRITE) {
        // TODO handle RF write
    };

    SYS_DEBUG_PRINT(SYS_ERROR_INFO, "GPO IT_STS_Dyn: %X, invoked %d\n", nfcAO->st25dvRegs.interruptStatus.raw, invoked++);

    return &(nfcStatesList[NFC_ST_IDLE]);
}

static const TState *_prepareMailbox(TActiveObject *const AO, TEvent event) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    NFC_ProcessPrepareMailboxFSM(nfcAO, event);

    return &(nfcStatesList[NFC_SUPER_ST_PREPARE_MAILBOX]);
};

static const TState *_error(TActiveObject *const AO, TEvent event) {
    SYS_ASSERT(!(NFC_ERROR == event.nfcSig), "__FILE__ NFC Error");
    SYS_ASSERT(!(NFC_TRANSFER_FAIL == event.nfcSig), "__FILE__ NFC Transfer Error");
    SYS_ASSERT(!(NFC_TRANSFER_MAX_RETRIES == event.nfcSig), "__FILE__ NFC Transfer max retries Error");

    return &(nfcStatesList[NFC_ST_ERROR]);
};

/** @brief refresh retries counter, mostly intent for utilizing in leaving states hooks */
static bool _refreshRetries(TActiveObject *const AO, void *const ctx) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    nfcAO->retriesLeft = NFC_TRANSFER_RETRIES_MAX;

    return true;
};

// error on i2c transfer queuing
void NFC_DispatchErrorOnInvalidTransfer(TNFCActiveObject *const nfcAO) {
    if (DRV_I2C_TRANSFER_HANDLE_INVALID == nfcAO->transferHandle) {
        ActiveObject_Dispatch(&(nfcAO->super), (TEvent) {.sig = NFC_ERROR});
    };
};

void NFC_VerifyRetries(TNFCActiveObject *const nfcAO) {
    if (NO_RETRIES_LEFT == nfcAO->retriesLeft) {
        ActiveObject_Dispatch(&(nfcAO->super), (TEvent) {.sig = NFC_TRANSFER_MAX_RETRIES});
    }
};
