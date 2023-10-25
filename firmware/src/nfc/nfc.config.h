/**
* @file nfc.config.h
* @author apolisskyi
* @date 16.02.2023
*/

#ifndef NFC_CONFIG_H
#define NFC_CONFIG_H

#ifdef    __cplusplus
extern "C" {
#endif

#define NFC_QUEUE_MAX_CAPACITY              (16)

#define NFC_TRANSFER_RETRIES_MAX            (0x20)

/* all SIZE is in Bytes */
#define NFC_UID_SIZE                        (0x08)
#define NFC_ITSTS_SIZE                      (0x01)
#define NFC_CMD_SIZE                        (0x02)
#define NFC_PASSWORD_SIZE                   (0x08)
#define NFC_PASSWORD_VALIDATION_INDEX       (0x08)
#define NFC_PASSWORD_VALIDATION_SIZE        (0x01)
#define NFC_SINGLE_BYTE_REG_SIZE            (0x01)
#define ST25DV_MAILBOX_SIZE                 (0x100)

#define NFC_MAILBOX_HEAD                    (0x00)

//#define

#define ST25DV_ADDR_DATA_I2C                (0xA6 >> 1) // E2=0
#define ST25DV_ADDR_SYST_I2C                (0xAE >> 1) // E2=1

/* MB_MODE */
#define ST25DV_MB_MODE_RW_SHIFT              (0)
#define ST25DV_MB_MODE_RW_FIELD              (0xFE)
#define ST25DV_MB_MODE_RW_MASK               (0x01)

/* MB_CTRL_Dyn */
#define ST25DV_MB_CTRL_DYN_MBEN_SHIFT        (0)
#define ST25DV_MB_CTRL_DYN_MBEN_FIELD        (0xFE)
#define ST25DV_MB_CTRL_DYN_MBEN_MASK         (0x01)
#define ST25DV_MB_CTRL_DYN_HOSTPUTMSG_SHIFT  (1)
#define ST25DV_MB_CTRL_DYN_HOSTPUTMSG_FIELD  (0xFD)
#define ST25DV_MB_CTRL_DYN_HOSTPUTMSG_MASK   (0x02)
#define ST25DV_MB_CTRL_DYN_RFPUTMSG_SHIFT    (2)
#define ST25DV_MB_CTRL_DYN_RFPUTMSG_FIELD    (0xFB)
#define ST25DV_MB_CTRL_DYN_RFPUTMSG_MASK     (0x04)
#define ST25DV_MB_CTRL_DYN_STRESERVED_SHIFT  (3)
#define ST25DV_MB_CTRL_DYN_STRESERVED_FIELD  (0xF7)
#define ST25DV_MB_CTRL_DYN_STRESERVED_MASK   (0x08)
#define ST25DV_MB_CTRL_DYN_HOSTMISSMSG_SHIFT (4)
#define ST25DV_MB_CTRL_DYN_HOSTMISSMSG_FIELD (0xEF)
#define ST25DV_MB_CTRL_DYN_HOSTMISSMSG_MASK  (0x10)
#define ST25DV_MB_CTRL_DYN_RFMISSMSG_SHIFT   (5)
#define ST25DV_MB_CTRL_DYN_RFMISSMSG_FIELD   (0xDF)
#define ST25DV_MB_CTRL_DYN_RFMISSMSG_MASK    (0x20)
#define ST25DV_MB_CTRL_DYN_CURRENTMSG_SHIFT  (6)
#define ST25DV_MB_CTRL_DYN_CURRENTMSG_FIELD  (0x3F)
#define ST25DV_MB_CTRL_DYN_CURRENTMSG_MASK   (0xC0)

/* GPO */
#define ST25DV_GPO_RFUSERSTATE_SHIFT         (0)
#define ST25DV_GPO_RFUSERSTATE_FIELD         0xFE
#define ST25DV_GPO_RFUSERSTATE_MASK          0x01
#define ST25DV_GPO_RFACTIVITY_SHIFT          (1)
#define ST25DV_GPO_RFACTIVITY_FIELD          0xFD
#define ST25DV_GPO_RFACTIVITY_MASK           0x02
#define ST25DV_GPO_RFINTERRUPT_SHIFT         (2)
#define ST25DV_GPO_RFINTERRUPT_FIELD         0xFB
#define ST25DV_GPO_RFINTERRUPT_MASK          0x04
#define ST25DV_GPO_FIELDCHANGE_SHIFT         (3)
#define ST25DV_GPO_FIELDCHANGE_FIELD         0xF7
#define ST25DV_GPO_FIELDCHANGE_MASK          0x08
#define ST25DV_GPO_RFPUTMSG_SHIFT            (4)
#define ST25DV_GPO_RFPUTMSG_FIELD            0xEF
#define ST25DV_GPO_RFPUTMSG_MASK             0x10
#define ST25DV_GPO_RFGETMSG_SHIFT            (5)
#define ST25DV_GPO_RFGETMSG_FIELD            0xDF
#define ST25DV_GPO_RFGETMSG_MASK             0x20
#define ST25DV_GPO_RFWRITE_SHIFT             (6)
#define ST25DV_GPO_RFWRITE_FIELD             0xBF
#define ST25DV_GPO_RFWRITE_MASK              0x40
#define ST25DV_GPO_ENABLE_SHIFT              (7)
#define ST25DV_GPO_ENABLE_FIELD              0x7F
#define ST25DV_GPO_ENABLE_MASK               0x80
#define ST25DV_GPO_ALL_MASK                  0xFF

/** @brief nfc states */
typedef enum {
    NFC_NO_STATE = 0,
    NFC_ST_INIT,
    NFC_ST_IDLE,
    NFC_ST_READ_UID,
    NFC_ST_READ_INTERRUPT_STATUS,
    NFC_SUPER_ST_PREPARE_MAILBOX,
    NFC_ST_WRITE_MAILBOX,
    NFC_ST_READ_MAILBOX,
    NFC_ST_ERROR,
    NFC_STATES_MAX
} NFC_STATE;

/** @brief nfc events signals */
typedef enum {
    NFC_NO_EVENT = 0,
    NFC_I2C_TRANSFER_SUCCESS = 1,
    NFC_I2C_TRANSFER_FAIL,
    NFC_I2C_TRANSFER_TIMEOUT,
    NFC_I2C_TRANSFER_MAX_RETRIES,

    NFC_READ_UID,
    NFC_READ_ITSTS,

    NFC_PREPARE_MAILBOX_SUCCESS,

    NFC_WRITE_MAILBOX,

    NFC_GPO_PULSE,

    NFC_READ_MAILBOX,

    NFC_ERROR,
    NFC_SIG_MAX,
} NFC_SIG;

#ifdef    __cplusplus
}
#endif

#endif    /* NFC_CONFIG_H */
