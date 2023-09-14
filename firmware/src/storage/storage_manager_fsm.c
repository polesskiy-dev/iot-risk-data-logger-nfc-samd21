#include "./storage_manager.h"

static const TState *_error(TActiveObject *const AO, TEvent event);

static const TState *_idle(TActiveObject *const AO, TEvent event);

static const TState *_readMemoryBootSector(TActiveObject *const AO, TEvent event);

static const TState *_writeMemoryBootSector(TActiveObject *const AO, TEvent event);

static const TState *_verifyMemoryBootSector(TActiveObject *const AO, TEvent event);

static const TState *_findLastNonEmptyPage(TActiveObject *const AO, TEvent event);

static const TState *_storeDataInTail(TActiveObject *const AO, TEvent event);

// error on MEMORY transfer queuing
static inline void _dispatchErrorOnInvalidTransfer(TSTORAGEActiveObject *const storageAO) {
    if (DRV_I2C_TRANSFER_HANDLE_INVALID == storageAO->transferHandle) {
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_ERROR});
    };
};

/* states */
const TState storageStatesList[STORAGE_STATES_MAX] = {
        [STORAGE_NO_STATE] = {.name = STORAGE_NO_STATE},
        [STORAGE_ST_INIT] = {.name = STORAGE_ST_INIT},
        [STORAGE_ST_READ_BOOT_SECTOR] = {.name = STORAGE_ST_READ_BOOT_SECTOR},
        [STORAGE_ST_VERIFY_BOOT_SECTOR] = {.name = STORAGE_ST_VERIFY_BOOT_SECTOR},
        [STORAGE_ST_WRITE_BOOT_SECTOR] = {.name = STORAGE_ST_WRITE_BOOT_SECTOR},
        [STORAGE_ST_FIND_LAST_NONEMPTY_PAGE] = {.name = STORAGE_ST_FIND_LAST_NONEMPTY_PAGE},
        [STORAGE_ST_IDLE] = {.name = STORAGE_ST_IDLE},
        [STORAGE_ST_STORE_DATA_IN_TAIL] = {.name = STORAGE_ST_STORE_DATA_IN_TAIL},
        [STORAGE_ST_ERROR] = {.name = STORAGE_ST_ERROR}
};

/* state transitions table */
const TEventHandler storageTransitionTable[STORAGE_STATES_MAX][STORAGE_SIG_MAX] = {
        [STORAGE_ST_INIT]=                      {[STORAGE_CHECK_MEMORY_BOOT_SECTOR] = _readMemoryBootSector, [STORAGE_ERROR]=_error},
        [STORAGE_ST_READ_BOOT_SECTOR]=          {[STORAGE_TRANSFER_SUCCESS]=_verifyMemoryBootSector, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_ERROR]=_error},
        [STORAGE_ST_VERIFY_BOOT_SECTOR]=        {[STORAGE_WRITE_MEMORY_BOOT_SECTOR]=_writeMemoryBootSector, [STORAGE_VERIFY_MEMORY_BOOT_SECTOR_SUCCESS]=_findLastNonEmptyPage, [STORAGE_ERROR]=_error},
        [STORAGE_ST_WRITE_BOOT_SECTOR]=         {[STORAGE_TRANSFER_SUCCESS]=_findLastNonEmptyPage /* TODO check whether STORAGE_TRANSFER_SUCCESS occurs after all 10 blocks or after each*/, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_ERROR]=_error},
        [STORAGE_ST_FIND_LAST_NONEMPTY_PAGE]=   {[STORAGE_TRANSFER_SUCCESS]=_findLastNonEmptyPage, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_FIND_LAST_NON_EMPTY_PAGE_SUCCESS]=_idle, [STORAGE_ERROR]=_error},
        [STORAGE_ST_IDLE]=                      {[STORAGE_STORE_DATA_IN_TAIL]=_storeDataInTail, [STORAGE_ERROR]=_error},
        [STORAGE_ST_STORE_DATA_IN_TAIL]=        {[STORAGE_TRANSFER_SUCCESS]=_idle, [STORAGE_ERROR]=_error},
        [STORAGE_ST_ERROR]=                     {[STORAGE_ERROR]=_error},
};

static const TState *_error(TActiveObject *const AO, TEvent event) {
    return &(storageStatesList[STORAGE_ST_ERROR]);
};

static const TState *_idle(TActiveObject *const AO, TEvent event) {
    return &(storageStatesList[STORAGE_ST_IDLE]);
};

static const TState *_readMemoryBootSector(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    DRV_MEMORY_AsyncRead(
            storageAO->drvMemoryHandle,
            &(storageAO->transferHandle),
            storageAO->pageBuffer,
            DRV_MEMORY_BOOT_SECTOR_FLASH_ADDRESS + PARTITION_0_ADDRESS, // let's check from meaningful partition data starts
            BOOT_SECTOR_PAGES_TO_VALIDATE * READ_BLOCKS_IN_PAGE
    );

    _dispatchErrorOnInvalidTransfer(storageAO);

    return &(storageStatesList[STORAGE_ST_READ_BOOT_SECTOR]);
};

static const TState *_verifyMemoryBootSector(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    if (IS_EQUAL_PAGES == memcmp(storageAO->pageBuffer, (FATBootSectorImage + PARTITION_0_ADDRESS), DRV_AT25DF_PAGE_SIZE)) {
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_VERIFY_MEMORY_BOOT_SECTOR_SUCCESS});
    } else {
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_WRITE_MEMORY_BOOT_SECTOR});
    }

    return &(storageStatesList[STORAGE_ST_VERIFY_BOOT_SECTOR]);
};

static const TState *_writeMemoryBootSector(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    DRV_MEMORY_AsyncEraseWrite(
            storageAO->drvMemoryHandle,
            &(storageAO->transferHandle),
            (void *) FATBootSectorImage,
            DRV_MEMORY_BOOT_SECTOR_FLASH_ADDRESS,
            DRV_MEMORY_BOOT_SECTOR_SIZE_PAGES
    );

    _dispatchErrorOnInvalidTransfer(storageAO);

    return &(storageStatesList[STORAGE_ST_WRITE_BOOT_SECTOR]);
}

static const TState *_findLastNonEmptyPage(TActiveObject *const AO, TEvent event) {
    // TODO implement this

    return &(storageStatesList[STORAGE_ST_FIND_LAST_NONEMPTY_PAGE]);
}

static const TState *_storeDataInTail(TActiveObject *const AO, TEvent event) {
    // TODO implement this

    return &(storageStatesList[STORAGE_ST_STORE_DATA_IN_TAIL]);
}