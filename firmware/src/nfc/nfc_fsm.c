#include "./nfc.h"

// st25dv nfc commands registers
static const uint8_t ST25DV_UID_REG[] = {0x00, 0x18};
static const uint8_t ST25DV_MAILBOX_RAM_REG[] = {0x20, 0x08};
static const uint8_t ST25DV_MB_CTRL_DYN_REG[] = {0x20, 0x06};
static const uint8_t ST25DV_MB_MODE_REG[] = {0x00, 0x0D};
static const uint8_t ST25DV_I2C_PWD_REG[] = {0x09, 0x00};

static const TState *_idle(TActiveObject *const AO, TEvent event);

static const TState *_readUID(TActiveObject *const AO, TEvent event);

static const TState *_presentI2CPwd(TActiveObject *const AO, TEvent event);

static const TState *_allowMBModeWrite(TActiveObject *const AO, TEvent event);

static const TState *_enableFTMode(TActiveObject *const AO, TEvent event);

static const TState *_writeMailbox(TActiveObject *const AO, TEvent event);

static const TState *_error(TActiveObject *const AO, TEvent event);

static bool _refreshRetries(TActiveObject *const AO, void *const ctx);

//static bool _clearTransferBuf(TActiveObject *const AO, void *const ctx);

// error on i2c transfer queuing
static inline void _dispatchErrorOnInvalidTransfer(TNFCActiveObject *const nfcAO) {
    if (DRV_I2C_TRANSFER_HANDLE_INVALID == nfcAO->transferHandle) {
        ActiveObject_Dispatch(&(nfcAO->super), (TEvent) {.sig = NFC_ERROR});
    };
};

/** @brief checks if mailbox is empty */
static inline bool _isMailboxEmpty(TNFCActiveObject *const nfcAO) {
    for (uint16_t i = 0; i < ST25DV_MAILBOX_SIZE; i++)
        if (nfcAO->transferBuf.mailbox[i] != 0)
            return false;
    return true;
};

/* states */
const TState nfcStatesList[NFC_STATES_MAX] = {
        [NFC_ST_INIT] =                 {.name = NFC_ST_INIT},
        [NFC_ST_IDLE] =                 {.name = NFC_ST_IDLE},
        [NFC_ST_READ_UID] =             {.name = NFC_ST_READ_UID},
        [NFC_ST_PRESENT_I2C_PWD] =      {.name = NFC_ST_PRESENT_I2C_PWD, .onExit = _refreshRetries},
        [NFC_ST_ALLOW_MB_MODE_WRITE] =  {.name = NFC_ST_ALLOW_MB_MODE_WRITE, .onExit = _refreshRetries},
        [NFC_ST_ENABLE_FT_MODE] =       {.name = NFC_ST_ENABLE_FT_MODE, .onExit = _refreshRetries},
        [NFC_ST_WRITE_MAILBOX] =        {.name = NFC_ST_WRITE_MAILBOX, .onExit = _refreshRetries},
        [NFC_ST_READ_MAILBOX] =         {.name = NFC_ST_READ_MAILBOX, .onExit = _refreshRetries},
        [NFC_ST_ERROR] =                {.name = NFC_ST_ERROR}
};

/* state transitions table */
const TEventHandler nfcTransitionTable[NFC_STATES_MAX][NFC_SIG_MAX] = {
        [NFC_ST_INIT]=                  {[NFC_READ_UID]=_readUID, [NFC_ERROR]=_error},
        [NFC_ST_IDLE]=                  {[NFC_WRITE_MAILBOX]=_writeMailbox, /* TODO NFC_ST_READ_MAILBOX on RF (NFC_INT) */ [NFC_ERROR]=_error},
        [NFC_ST_READ_UID]=              {[NFC_TRANSFER_SUCCESS]=_presentI2CPwd, [NFC_TRANSFER_FAIL]=_error, [NFC_ERROR]=_error},
        /* enable Fast Transfer Mode (Mailbox MODE accessed wor W when I2C security session is open)*/
        [NFC_ST_PRESENT_I2C_PWD]=       {[NFC_TRANSFER_SUCCESS]=_allowMBModeWrite, [NFC_TRANSFER_FAIL]=_presentI2CPwd, [NFC_TRANSFER_MAX_RETRIES]=_error, [NFC_ERROR]=_error},
        [NFC_ST_ALLOW_MB_MODE_WRITE]=   {[NFC_TRANSFER_SUCCESS]=_enableFTMode, [NFC_TRANSFER_FAIL]=_allowMBModeWrite, [NFC_TRANSFER_MAX_RETRIES]=_error, [NFC_ERROR]=_error},
        [NFC_ST_ENABLE_FT_MODE]=        {[NFC_TRANSFER_SUCCESS]=_idle /* Mailbox ready */, [NFC_TRANSFER_FAIL]=_enableFTMode, [NFC_TRANSFER_MAX_RETRIES]=_error, [NFC_ERROR]=_error},
        /* Mailbox (exchange data between I2C and RF) */
        [NFC_ST_WRITE_MAILBOX]=         {[NFC_TRANSFER_SUCCESS]=_idle, [NFC_ERROR]=_error},
        [NFC_ST_READ_MAILBOX]=          {[NFC_TRANSFER_SUCCESS]=_idle, [NFC_ERROR]=_error},

        [NFC_ST_ERROR]=                 {[NFC_ERROR]=_error},
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

    _dispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_READ_UID]);
};

/** @brief  Presents I2C password, to authorize the I2C writes to protected areas. E.g. MB_CTRL_Dyn*/
static const TState *_presentI2CPwd(TActiveObject *const AO, TEvent event) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    nfcAO->retriesLeft--;
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

    _dispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_PRESENT_I2C_PWD]);
};

/** @brief  Allows to write to MB_MODE register */
static const TState *_allowMBModeWrite(TActiveObject *const AO, TEvent event) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    nfcAO->retriesLeft--;
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

    _dispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_ALLOW_MB_MODE_WRITE]);
};

static const TState *_enableFTMode(TActiveObject *const AO, TEvent event) {
    TNFCActiveObject *nfcAO = (TNFCActiveObject *) AO;

    nfcAO->retriesLeft--;
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

    _dispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_ENABLE_FT_MODE]);
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

    _dispatchErrorOnInvalidTransfer(nfcAO);

    return &(nfcStatesList[NFC_ST_WRITE_MAILBOX]);
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

//
//static inline void validateTransferHandle(NFC_ACT_OBJ *me) {
//    // error on i2c transfer queuing
//    if (DRV_I2C_TRANSFER_HANDLE_INVALID == me->transferHandle) {
//        NFC_ACT_Dispatch((QUEUE_EVENT) {.nfcSig = NFC_ERROR});
//    };
//};
//
//static inline void checkRemainingRetries(NFC_ACT_OBJ *me, QUEUE_EVENT event) {
//    // decrease remaining retries on transfer fail
//    if (NFC_TRANSFER_FAIL == event.nfcSig) {
//        if (NO_RETRIES_LEFT == --me->retriesLeft) {
//            NFC_ACT_Dispatch((QUEUE_EVENT) {.nfcSig = NFC_TRANSFER_MAX_RETRIES});
//        }
//    } else {
//        // or refresh counter on non fail event
//        me->retriesLeft = NFC_TRANSFER_RETRIES_MAX;
//    };
//};
