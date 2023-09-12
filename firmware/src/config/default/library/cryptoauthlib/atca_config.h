/* Auto-generated config file atca_config.h */
#ifndef ATCA_CONFIG_H
#define ATCA_CONFIG_H

/* MPLAB Harmony Common Include */
#include "definitions.h"

#ifndef ATCA_HAL_I2C
#define ATCA_HAL_I2C
#endif


/** Include Device Support Options */
#define ATCA_ATECC608_SUPPORT

/** Provide Maximum packet size for the command to be sent and received */
#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE     (1072U)
#endif


/* Polling Configuration Options  */
#ifndef ATCA_POLLING_INIT_TIME_MSEC
#define ATCA_POLLING_INIT_TIME_MSEC       1
#endif
#ifndef ATCA_POLLING_FREQUENCY_TIME_MSEC
#define ATCA_POLLING_FREQUENCY_TIME_MSEC  2
#endif
#ifndef ATCA_POLLING_MAX_TIME_MSEC
#define ATCA_POLLING_MAX_TIME_MSEC        2500
#endif

/** Define if the library is not to use malloc/free */
#ifndef ATCA_NO_HEAP
#define ATCA_NO_HEAP
#endif

/** Symmetric Commands Configurations */

/* AES Command */
#define   ATCAB_AES_EN                     (FEATURE_ENABLED)

#define ATCAB_AES_GCM_EN                   (FEATURE_ENABLED)
#define ATCAB_AES_GFM_EN                   (FEATURE_ENABLED)

/* Checkmac Command */
#define ATCAB_CHECKMAC_EN                      (FEATURE_ENABLED)

/* Gendig Command */
#define ATCAB_GENDIG_EN                        (FEATURE_ENABLED)

/* KDF Command */
#define ATCAB_KDF_EN                           (FEATURE_ENABLED)

/* MAC Command */
#define ATCAB_MAC_EN                           (FEATURE_ENABLED)

/* HMAC Command */
#define ATCAB_HMAC_EN                      (FEATURE_ENABLED)

/** Asymmetric Commands Configurations */

/* ECDH Command */
#define ATCAB_ECDH_EN                      (FEATURE_ENABLED)

#define ATCAB_ECDH_ENC_EN                  (FEATURE_ENABLED)

/* Genkey Command */
#define ATCAB_GENKEY_EN                    (FEATURE_ENABLED)

#define ATCAB_GENKEY_MAC_EN                (ATCAB_GENKEY_EN)

/* Sign Command */
#define ATCAB_SIGN_EN                      (FEATURE_ENABLED)

#define ATCAB_SIGN_INTERNAL_EN             (ATCAB_SIGN_EN)

/* VERIFY Command */
#define ATCAB_VERIFY_EN                    (FEATURE_ENABLED)

#define ATCAB_VERIFY_STORED_EN             (ATCAB_VERIFY_EN)

#define ATCAB_VERIFY_EXTERN_EN             (ATCAB_VERIFY_EN)

#define ATCAB_VERIFY_VALIDATE_EN           (ATCAB_VERIFY_EN)

#define ATCAB_VERIFY_EXTERN_STORED_MAC_EN  (ATCAB_VERIFY_EN)

/** General Device Commands Configurations */

/* Counter Command */
#define ATCAB_COUNTER_EN                   (FEATURE_ENABLED)

/* Delete Command */
#define ATCAB_DELETE_EN                    (FEATURE_DISABLED)

/* Derivekey Command */
#define ATCAB_DERIVEKEY_EN                 (FEATURE_ENABLED)

/* Info Command */
#define ATCAB_INFO_LATCH_EN                (FEATURE_ENABLED)

/* Lock Command */
#define ATCAB_LOCK_EN                      (FEATURE_ENABLED)

/* Nonce Command */
#define ATCAB_NONCE_EN                     (FEATURE_ENABLED)

/* PrivWrite Command */
#define ATCAB_PRIVWRITE_EN                 (FEATURE_ENABLED)

/* Random Command */
#define ATCAB_RANDOM_EN                    (FEATURE_ENABLED)

/* Read Command */
#define ATCAB_READ_EN                      (FEATURE_ENABLED)

#define ATCAB_READ_ENC_EN              (ATCAB_READ_EN)

/* Secureboot Command */
#define ATCAB_SECUREBOOT_EN                (FEATURE_ENABLED)

#define ATCAB_SECUREBOOT_MAC_EN        (ATCAB_SECUREBOOT_EN)

/* Selftest Command */
#define ATCAB_SELFTEST_EN                  (FEATURE_ENABLED)

/* SHA Command */
#define ATCAB_SHA_EN                       (FEATURE_ENABLED)

#define ATCAB_SHA_HMAC_EN              (ATCAB_SHA_EN)

#define ATCAB_SHA_CONTEXT_EN           (ATCAB_SHA_EN)

/* UpdateExtra Command */
#define ATCAB_UPDATEEXTRA_EN               (FEATURE_ENABLED)

/* Write Command */
#define ATCAB_WRITE_EN               (FEATURE_ENABLED)

#define ATCAB_WRITE_ENC_EN               (ATCAB_WRITE_EN)

/* Host side Cryptographic functionality required by the library  */

/* Crypto Hardware AES Configurations */
#define ATCAB_AES_EXTRAS_EN                (CALIB_AES_EN || TALIB_AES_EN)

#define ATCAB_AES_CBC_ENCRYPT_EN       (ATCAB_AES_EXTRAS_EN)

#define ATCAB_AES_CBC_DECRYPT_EN       (ATCAB_AES_EXTRAS_EN)

#define ATCAB_AES_CBCMAC_EN            (ATCAB_AES_CBC_ENCRYPT_EN)

#define ATCAB_AES_CTR_EN               (ATCAB_AES_EXTRAS_EN)

#define ATCAB_AES_CTR_RAND_IV_EN       (ATCAB_AES_CTR_EN && ATCAB_AES_RANDOM_IV_EN)

#define ATCAB_AES_CCM_EN               (ATCAB_AES_CBCMAC_EN && ATCAB_AES_CTR_EN)

#define ATCAB_AES_CCM_RAND_IV_EN       (ATCAB_AES_CCM_EN && ATCAB_AES_RANDOM_IV_EN)

#define ATCAB_AES_CMAC_EN              (ATCAB_AES_CBC_ENCRYPT_EN)

/* Crypto Software SHA Configurations */
#define ATCAC_SHA1_EN                      (FEATURE_ENABLED)

#define ATCAC_SHA256_EN                    (FEATURE_ENABLED)

#define ATCAC_SHA256_HMAC_EN               (ATCAC_SHA256_EN)

#define ATCAC_SHA256_HMAC_CTR_EN           (ATCAC_SHA256_HMAC_EN)

#define ATCAC_PBKDF2_SHA256_EN             (ATCAC_SHA256_HMAC_EN)

/* External Crypto libraries configurations for host side operations */

#define ATCAC_RANDOM_EN                    (ATCA_HOSTLIB_EN)

#define ATCAC_SIGN_EN                      (ATCA_HOSTLIB_EN)

#define ATCAC_VERIFY_EN                    (ATCA_HOSTLIB_EN)

/** Define platform malloc/free */
#define ATCA_PLATFORM_MALLOC    malloc
#define ATCA_PLATFORM_FREE      free

#define atca_delay_ms   hal_delay_ms
#define atca_delay_us   hal_delay_us

/* \brief How long to wait after an initial wake failure for the POST to
 *         complete.
 * If Power-on self test (POST) is enabled, the self test will run on waking
 * from sleep or during power-on, which delays the wake reply.
 */
#ifndef ATCA_POST_DELAY_MSEC
#define ATCA_POST_DELAY_MSEC 25
#endif

#ifndef ATCA_PRINTF
#define ATCA_PRINTF
#endif


#ifndef ATCA_PREPROCESSOR_WARNING
#define ATCA_PREPROCESSOR_WARNING     false
#endif

/* Define generic interfaces to the processor libraries */

#define PLIB_I2C_ERROR          SERCOM_I2C_ERROR
#define PLIB_I2C_ERROR_NONE     SERCOM_I2C_ERROR_NONE
#define PLIB_I2C_TRANSFER_SETUP SERCOM_I2C_TRANSFER_SETUP

typedef bool (* atca_i2c_plib_read)( uint16_t, uint8_t *, uint32_t );
typedef bool (* atca_i2c_plib_write)( uint16_t, uint8_t *, uint32_t );
typedef bool (* atca_i2c_plib_is_busy)( void );
typedef PLIB_I2C_ERROR (* atca_i2c_error_get)( void );
typedef bool (* atca_i2c_plib_transfer_setup)(PLIB_I2C_TRANSFER_SETUP* setup, uint32_t srcClkFreq);

typedef struct atca_plib_i2c_api
{
    atca_i2c_plib_read              read;
    atca_i2c_plib_write             write;
    atca_i2c_plib_is_busy           is_busy;
    atca_i2c_error_get              error_get;
    atca_i2c_plib_transfer_setup    transfer_setup;
} atca_plib_i2c_api_t;




extern atca_plib_i2c_api_t sercom0_plib_i2c_api;



#define ATCA_TEST_MULTIPLE_INSTANCES


#endif // ATCA_CONFIG_H
