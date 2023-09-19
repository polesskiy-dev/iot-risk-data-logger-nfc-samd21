#include "./storage_manager.h"

static const TState *_error(TActiveObject *const AO, TEvent event);

static const TState *_idle(TActiveObject *const AO, TEvent event);

static const TState *_readMemoryBootSector(TActiveObject *const AO, TEvent event);

static const TState *_writeMemoryBootSector(TActiveObject *const AO, TEvent event);

static const TState *_verifyMemoryBootSector(TActiveObject *const AO, TEvent event);

static const TState *_findLastLogsNonEmptyPage(TActiveObject *const AO, TEvent event);

static const TState *_storeDataInTail(TActiveObject *const AO, TEvent event);

static const TState *_storeData(TActiveObject *const AO, TEvent event);

// error on MEMORY transfer queuing
static inline void _dispatchErrorOnInvalidTransfer(TSTORAGEActiveObject *const storageAO) {
    if (DRV_I2C_TRANSFER_HANDLE_INVALID == storageAO->transferHandle) {
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_ERROR});
    };
};

/* states */
const TState storageStatesList[STORAGE_STATES_MAX] = {
        [STORAGE_NO_STATE] =                    {.name = STORAGE_NO_STATE},
        [STORAGE_ST_INIT] =                     {.name = STORAGE_ST_INIT},
        [STORAGE_ST_READ_BOOT_SECTOR] =         {.name = STORAGE_ST_READ_BOOT_SECTOR},
        [STORAGE_ST_VERIFY_BOOT_SECTOR] =       {.name = STORAGE_ST_VERIFY_BOOT_SECTOR, .onExit = (TStateHook) STORAGE_CLearPageBuffer},
        [STORAGE_ST_WRITE_BOOT_SECTOR] =        {.name = STORAGE_ST_WRITE_BOOT_SECTOR},
        [STORAGE_ST_FIND_LAST_NONEMPTY_PAGE] =  {.name = STORAGE_ST_FIND_LAST_NONEMPTY_PAGE, .onEnter = (TStateHook) STORAGE_CLearPageBuffer, .onExit = (TStateHook) STORAGE_CLearPageBuffer},
        [STORAGE_ST_IDLE] =                     {.name = STORAGE_ST_IDLE, .onEnter = (TStateHook) STORAGE_CLearPageBuffer},
        [STORAGE_ST_STORE_DATA_IN_TAIL] =       {.name = STORAGE_ST_STORE_DATA_IN_TAIL, .onEnter = (TStateHook) STORAGE_CLearPageBuffer},
        [STORAGE_ST_STORE_DATA] =               {.name = STORAGE_ST_STORE_DATA, .onExit = (TStateHook) STORAGE_CLearPageBuffer},
        [STORAGE_ST_ERROR] =                    {.name = STORAGE_ST_ERROR}
};

/* state transitions table */
const TEventHandler storageTransitionTable[STORAGE_STATES_MAX][STORAGE_SIG_MAX] = {
        [STORAGE_ST_INIT]=                      {[STORAGE_CHECK_MEMORY_BOOT_SECTOR] = _readMemoryBootSector, [STORAGE_ERROR]=_error},
        [STORAGE_ST_READ_BOOT_SECTOR]=          {[STORAGE_TRANSFER_SUCCESS]=_verifyMemoryBootSector, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_ERROR]=_error},
        [STORAGE_ST_VERIFY_BOOT_SECTOR]=        {[STORAGE_VERIFY_MEMORY_BOOT_SECTOR_SUCCESS]=_findLastLogsNonEmptyPage, [STORAGE_WRITE_MEMORY_BOOT_SECTOR]=_writeMemoryBootSector, [STORAGE_ERROR]=_error},
        [STORAGE_ST_WRITE_BOOT_SECTOR]=         {[STORAGE_TRANSFER_SUCCESS]=_findLastLogsNonEmptyPage /* TODO check whether STORAGE_TRANSFER_SUCCESS occurs after all 10 blocks or after each*/, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_ERROR]=_error},
        [STORAGE_ST_FIND_LAST_NONEMPTY_PAGE]=   {[STORAGE_TRANSFER_SUCCESS]=_findLastLogsNonEmptyPage, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_FIND_LAST_NON_EMPTY_PAGE_SUCCESS]=_idle, [STORAGE_ERROR]=_error},
        [STORAGE_ST_IDLE]=                      {[STORAGE_STORE_DATA_IN_TAIL]=_storeDataInTail, [STORAGE_ERROR]=_error},
        [STORAGE_ST_STORE_DATA_IN_TAIL]=        {[STORAGE_TRANSFER_SUCCESS]=_storeData, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_ERROR]=_error},
        [STORAGE_ST_STORE_DATA]=                {[STORAGE_TRANSFER_SUCCESS]=_idle, [STORAGE_TRANSFER_FAIL]=_error, [STORAGE_STORE_DATA_IN_TAIL]=_storeDataInTail, [STORAGE_ERROR]=_error},
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

    /** @note read block in EEPROM is 1 byte*/
    DRV_MEMORY_AsyncRead(
            storageAO->drvMemoryHandle,
            &(storageAO->transferHandle),
            storageAO->pageBuffer,
            DRV_MEMORY_BOOT_SECTOR_FLASH_ADDRESS +
            PARTITION_0_ADDRESS, // let's check from meaningful partition data starts
            BOOT_SECTOR_PAGES_TO_VALIDATE * READ_BLOCKS_IN_PAGE
    );

    _dispatchErrorOnInvalidTransfer(storageAO);

    return &(storageStatesList[STORAGE_ST_READ_BOOT_SECTOR]);
};

static const TState *_verifyMemoryBootSector(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    if (IS_EQUAL_PAGES ==
        memcmp(storageAO->pageBuffer, (FATBootSectorImage + PARTITION_0_ADDRESS), DRV_AT25DF_PAGE_SIZE)) {
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_VERIFY_MEMORY_BOOT_SECTOR_SUCCESS});
    } else {
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_WRITE_MEMORY_BOOT_SECTOR});
    }

    return &(storageStatesList[STORAGE_ST_VERIFY_BOOT_SECTOR]);
};

static const TState *_writeMemoryBootSector(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    /** @note write block in EEPROM is 256 bytes and erase block is 4096*/
    DRV_MEMORY_AsyncEraseWrite(
            storageAO->drvMemoryHandle,
            &(storageAO->transferHandle),
            (void *) FATBootSectorImage,
            DRV_MEMORY_BOOT_SECTOR_FLASH_ADDRESS,
            DRV_MEMORY_BOOT_SECTOR_SIZE_PAGES
    );

    _dispatchErrorOnInvalidTransfer(storageAO);

    // set current page to next after boot sector
    storageAO->flash.currentPage = DRV_MEMORY_BOOT_SECTOR_SIZE_PAGES;

    return &(storageStatesList[STORAGE_ST_WRITE_BOOT_SECTOR]);
}

static const TState *_findLastLogsNonEmptyPage(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    DRV_MEMORY_AsyncRead(
            storageAO->drvMemoryHandle,
            &(storageAO->transferHandle),
            storageAO->pageBuffer,
            (storageAO->flash.currentPage * DRV_AT25DF_PAGE_SIZE) +
            BOOT_SECTOR_SIZE, // offset from boot sector check page by page
            READ_BLOCKS_IN_PAGE
    );

    // if storageAO.pageBuffer empty - inspected page is suitable for new data, if not, then increment storageAO.page and repeat
    if (IS_EQUAL_PAGES == memcmp(storageAO->pageBuffer, (void *) ERASED_PAGE_PATTERN, DRV_AT25DF_PAGE_SIZE)) {
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_FIND_LAST_NON_EMPTY_PAGE_SUCCESS});
    } else {
        storageAO->flash.currentPage++;
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {.sig = STORAGE_FIND_LAST_NON_EMPTY_PAGE});
    }

    return &(storageStatesList[STORAGE_ST_FIND_LAST_NONEMPTY_PAGE]);
}

static const TState *_storeDataInTail(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    // read current page
    DRV_MEMORY_AsyncRead(
            storageAO->drvMemoryHandle,
            &(storageAO->transferHandle),
            storageAO->pageBuffer,
            (storageAO->flash.currentPage * DRV_AT25DF_PAGE_SIZE),
            READ_BLOCKS_IN_PAGE
    );

    _dispatchErrorOnInvalidTransfer(storageAO);

    storageAO->dataToStore = event.payload;
    storageAO->dataToStoreSize = event.size;

    return &(storageStatesList[STORAGE_ST_STORE_DATA_IN_TAIL]);
}

static const TState *_storeData(TActiveObject *const AO, TEvent event) {
    TSTORAGEActiveObject *storageAO = (TSTORAGEActiveObject *) AO;

    uint8_t freePlaceInPageAddr = 0;

    // find free place in page (should be equal to data size)
    while (freePlaceInPageAddr < END_OF_PAGE_ADDRESS) {
        if (IS_ENOUGH_PLACE_TO_STORE ==
            memcmp(storageAO->pageBuffer + freePlaceInPageAddr, (void *) ERASED_PAGE_PATTERN,
                   storageAO->dataToStoreSize))
            break;
        freePlaceInPageAddr += storageAO->dataToStoreSize; // offset is same as data size
    };

    if (freePlaceInPageAddr >= END_OF_PAGE_ADDRESS) {
        // no free place in page, increment page and repeat
        storageAO->flash.currentPage++;
        ActiveObject_Dispatch(&(storageAO->super), (TEvent) {
                .sig = STORAGE_STORE_DATA_IN_TAIL,
                .payload = storageAO->dataToStore,
                .size = storageAO->dataToStoreSize
        });

        return &(storageStatesList[STORAGE_ST_STORE_DATA]);
    };

    // append data to page buffer
    memcpy(storageAO->pageBuffer + freePlaceInPageAddr, storageAO->dataToStore, storageAO->dataToStoreSize);

    // write page buffer to flash
    DRV_MEMORY_AsyncWrite(
            storageAO->drvMemoryHandle,
            &(storageAO->transferHandle),
            storageAO->pageBuffer,
            (storageAO->flash.currentPage * DRV_AT25DF_PAGE_SIZE),
            WRITE_BLOCKS_IN_PAGE);

    _dispatchErrorOnInvalidTransfer(storageAO);

    return &(storageStatesList[STORAGE_ST_STORE_DATA]);
}