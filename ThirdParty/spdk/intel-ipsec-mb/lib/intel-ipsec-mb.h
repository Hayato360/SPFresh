/*******************************************************************************
  Copyright (c) 2012-2022, Intel Corporation

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Intel Corporation nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef IMB_IPSEC_MB_H
#define IMB_IPSEC_MB_H

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 128-bit data type that is not in sdtint.h */
typedef struct {
        uint64_t low;
        uint64_t high;
} imb_uint128_t;

/**
 * Macros for aligning data structures and function inlines
 */
#if defined __linux__ || defined __FreeBSD__
/**< Linux/FreeBSD */
#define DECLARE_ALIGNED(decl, alignval) \
        decl __attribute__((aligned(alignval)))
#define __forceinline \
        static inline __attribute__((always_inline))

#if __GNUC__ >= 4
#define IMB_DLL_EXPORT __attribute__((visibility("default")))
#define IMB_DLL_LOCAL  __attribute__((visibility("hidden")))
#else /* GNU C 4.0 and later */
#define IMB_DLL_EXPORT
#define IMB_DLL_LOCAL
#endif /**< different C compiler */

#else
/* Windows */

#ifdef __MINGW32__
/* MinGW-w64 */
#define DECLARE_ALIGNED(decl, alignval) \
        decl __attribute__((aligned(alignval)))
#undef __forceinline
#define __forceinline \
        static inline __attribute__((always_inline))

#else
/* MSVS */
#define DECLARE_ALIGNED(decl, alignval)         \
        __declspec(align(alignval)) decl
#define __forceinline \
        static __forceinline

#endif /* __MINGW__ */

/**
 * Windows DLL export is done via DEF file
 */
#define IMB_DLL_EXPORT
#define IMB_DLL_LOCAL

#endif /* defined __linux__ || defined __FreeBSD__ */

/**
 * Library version
 */
#define IMB_VERSION_STR "1.3.0"
#define IMB_VERSION_NUM 0x10300

/**
 * Macro to translate version number
 */
#define IMB_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))

/**
 * Custom ASSERT and DIM macros
 */
#ifdef DEBUG
#include <assert.h>
#define IMB_ASSERT(x) assert(x)
#else
#define IMB_ASSERT(x)
#endif

#ifndef IMB_DIM
#define IMB_DIM(x) (sizeof(x) / sizeof(x[0]))
#endif

/**
 * Architecture definitions
 */
typedef enum {
        IMB_ARCH_NONE = 0,
        IMB_ARCH_NOAESNI,
        IMB_ARCH_SSE,
        IMB_ARCH_AVX,
        IMB_ARCH_AVX2,
        IMB_ARCH_AVX512,
        IMB_ARCH_NUM,
} IMB_ARCH;

/**
 * Algorithm constants
 */
#define IMB_DES_KEY_SCHED_SIZE (16 * 8) /**< 16 rounds x 8 bytes */
#define IMB_DES_BLOCK_SIZE 8

#define IMB_AES_BLOCK_SIZE 16

#define IMB_SHA1_DIGEST_SIZE_IN_BYTES   20
#define IMB_SHA224_DIGEST_SIZE_IN_BYTES 28
#define IMB_SHA256_DIGEST_SIZE_IN_BYTES 32
#define IMB_SHA384_DIGEST_SIZE_IN_BYTES 48
#define IMB_SHA512_DIGEST_SIZE_IN_BYTES 64

#define IMB_SHA1_BLOCK_SIZE 64    /**< 512 bits is 64 byte blocks */
#define IMB_SHA_256_BLOCK_SIZE 64 /**< 512 bits is 64 byte blocks */
#define IMB_SHA_384_BLOCK_SIZE 128
#define IMB_SHA_512_BLOCK_SIZE 128

#define IMB_KASUMI_KEY_SIZE         16
#define IMB_KASUMI_IV_SIZE          8
#define IMB_KASUMI_BLOCK_SIZE       8
#define IMB_KASUMI_DIGEST_SIZE      4

/**
 * Minimum Ethernet frame size to calculate CRC32
 * Source Address (6 bytes) + Destination Address (6 bytes) + Type/Len (2 bytes)
 */
#define IMB_DOCSIS_CRC32_MIN_ETH_PDU_SIZE 14
#define IMB_DOCSIS_CRC32_TAG_SIZE         4

/**
 * Job structure definitions
 */

typedef enum {
        IMB_STATUS_BEING_PROCESSED  = 0,
        IMB_STATUS_COMPLETED_CIPHER = 1,
        IMB_STATUS_COMPLETED_AUTH   = 2,
        IMB_STATUS_COMPLETED        = 3, /**< COMPLETED_CIPHER |
					   COMPLETED_AUTH */
        IMB_STATUS_INVALID_ARGS     = 4,
        IMB_STATUS_INTERNAL_ERROR,
        IMB_STATUS_ERROR
} IMB_STATUS;

/**
 * Library error types
 */
typedef enum {
        IMB_ERR_MIN = 2000,
        IMB_ERR_NULL_MBMGR,
        IMB_ERR_JOB_NULL_SRC,
        IMB_ERR_JOB_NULL_DST,
        IMB_ERR_JOB_NULL_KEY,
        IMB_ERR_JOB_NULL_IV,
        IMB_ERR_JOB_NULL_AUTH,
        IMB_ERR_JOB_NULL_AAD,
        IMB_ERR_JOB_CIPH_LEN,
        IMB_ERR_JOB_AUTH_LEN,
        IMB_ERR_JOB_IV_LEN,
        IMB_ERR_JOB_KEY_LEN,
        IMB_ERR_JOB_AUTH_TAG_LEN,
        IMB_ERR_JOB_AAD_LEN,
        IMB_ERR_JOB_SRC_OFFSET,
        IMB_ERR_JOB_CHAIN_ORDER,
        IMB_ERR_CIPH_MODE,
        IMB_ERR_HASH_ALGO,
        IMB_ERR_JOB_NULL_AUTH_KEY,
        IMB_ERR_JOB_NULL_SGL_CTX,
        IMB_ERR_JOB_NULL_NEXT_IV,
        IMB_ERR_JOB_PON_PLI,
        IMB_ERR_NULL_SRC,
        IMB_ERR_NULL_DST,
        IMB_ERR_NULL_KEY,
        IMB_ERR_NULL_EXP_KEY,
        IMB_ERR_NULL_IV,
        IMB_ERR_NULL_AUTH,
        IMB_ERR_NULL_AAD,
        IMB_ERR_CIPH_LEN,
        IMB_ERR_AUTH_LEN,
        IMB_ERR_IV_LEN,
        IMB_ERR_KEY_LEN,
        IMB_ERR_AUTH_TAG_LEN,
        IMB_ERR_AAD_LEN,
        IMB_ERR_SRC_OFFSET,
        IMB_ERR_NULL_AUTH_KEY,
        IMB_ERR_NULL_CTX,
        IMB_ERR_NO_AESNI_EMU,
        IMB_ERR_JOB_NULL_HMAC_OPAD,
        IMB_ERR_JOB_NULL_HMAC_IPAD,
        IMB_ERR_JOB_NULL_XCBC_K1_EXP,
        IMB_ERR_JOB_NULL_XCBC_K2,
        IMB_ERR_JOB_NULL_XCBC_K3,
        IMB_ERR_JOB_CIPH_DIR,
        IMB_ERR_JOB_NULL_GHASH_INIT_TAG,
        IMB_ERR_MISSING_CPUFLAGS_INIT_MGR,
        IMB_ERR_NULL_JOB,
        IMB_ERR_QUEUE_SPACE,
        IMB_ERR_NULL_BURST,
        IMB_ERR_BURST_SIZE,
        IMB_ERR_BURST_OOO,
        IMB_ERR_SELFTEST,
        /* add new error types above this comment */
        IMB_ERR_MAX       /* don't move this one */
} IMB_ERR;

/**
 * IMB_ERR_MIN should be higher than __ELASTERROR
 * to avoid overlap with standard error values
 */
#ifdef __ELASTERROR
#if __ELASTERROR > 2000
#error "Library error codes conflict with errno.h - please update IMB_ERR_MIN!"
#endif
#endif

/**
 * Define enums from API v0.53, so applications that were using this version
 * will still be compiled successfully.
 * Note: this list has been extended with new names after version 0.55.
 * This list does not need to be extended for new enums.
 */
#ifndef NO_COMPAT_IMB_API_053
/* Previous cipher mode enums */
#define CBC                     IMB_CIPHER_CBC
#define CNTR                    IMB_CIPHER_CNTR
#define NULL_CIPHER             IMB_CIPHER_NULL
#define DOCSIS_SEC_BPI          IMB_CIPHER_DOCSIS_SEC_BPI
#define GCM                     IMB_CIPHER_GCM
#define CUSTOM_CIPHER           IMB_CIPHER_CUSTOM
#define DES                     IMB_CIPHER_DES
#define DOCSIS_DES              IMB_CIPHER_DOCSIS_DES
#define CCM                     IMB_CIPHER_CCM
#define DES3                    IMB_CIPHER_DES3
#define PON_AES_CNTR            IMB_CIPHER_PON_AES_CNTR
#define ECB                     IMB_CIPHER_ECB
#define CNTR_BITLEN             IMB_CIPHER_CNTR_BITLEN

/* Previous hash algo enums */
#define SHA1                    IMB_AUTH_HMAC_SHA_1
#define SHA_224                 IMB_AUTH_HMAC_SHA_224
#define SHA_256                 IMB_AUTH_HMAC_SHA_256
#define SHA_384                 IMB_AUTH_HMAC_SHA_384
#define SHA_512                 IMB_AUTH_HMAC_SHA_512
#define AES_XCBC                IMB_AUTH_AES_XCBC
#define MD5                     IMB_AUTH_MD5
#define NULL_HASH               IMB_AUTH_NULL
#define AES_GMAC                IMB_AUTH_AES_GMAC
#define CUSTOM_HASH             IMB_AUTH_CUSTOM
#define AES_CCM                 IMB_AUTH_AES_CCM
#define AES_CMAC                IMB_AUTH_AES_CMAC
#define PLAIN_SHA1              IMB_AUTH_SHA_1
#define PLAIN_SHA_224           IMB_AUTH_SHA_224
#define PLAIN_SHA_256           IMB_AUTH_SHA_256
#define PLAIN_SHA_384           IMB_AUTH_SHA_384
#define PLAIN_SHA_512           IMB_AUTH_SHA_512
#define AES_CMAC_BITLEN         IMB_AUTH_AES_CMAC_BITLEN
#define PON_CRC_BIP             IMB_AUTH_PON_CRC_BIP

/* Previous cipher direction enums */
#define ENCRYPT                 IMB_DIR_ENCRYPT
#define DECRYPT                 IMB_DIR_DECRYPT

/* Previous chain order enums */
#define HASH_CIPHER             IMB_ORDER_HASH_CIPHER
#define CIPHER_HASH             IMB_ORDER_CIPHER_HASH

/* Previous key size enums */
#define AES_128_BYTES           IMB_KEY_128_BYTES
#define AES_192_BYTES           IMB_KEY_192_BYTES
#define AES_256_BYTES           IMB_KEY_256_BYTES
#define IMB_KEY_AES_128_BYTES   IMB_KEY_128_BYTES
#define IMB_KEY_AES_192_BYTES   IMB_KEY_192_BYTES
#define IMB_KEY_AES_256_BYTES   IMB_KEY_256_BYTES
#define AES_KEY_SIZE_BYTES      IMB_KEY_SIZE_BYTES

#define MB_MGR                  IMB_MGR
#define JOB_AES_HMAC            IMB_JOB
#define JOB_STS                 IMB_STATUS
#define IMB_JOB_STS             IMB_STATUS
#define JOB_CIPHER_MODE         IMB_CIPHER_MODE
#define JOB_CIPHER_DIRECTION    IMB_CIPHER_DIRECTION
#define JOB_HASH_ALG            IMB_HASH_ALG
#define JOB_CHAIN_ORDER         IMB_CHAIN_ORDER
#define MAX_JOBS                IMB_MAX_JOBS

#define STS_BEING_PROCESSED     IMB_STATUS_BEING_PROCESSED
#define STS_COMPLETED_AES       IMB_STATUS_COMPLETED_CIPHER
#define STS_COMPLETED_HMAC      IMB_STATUS_COMPLETED_AUTH
#define STS_COMPLETED           IMB_STATUS_COMPLETED
#define STS_INVALID_ARGS        IMB_STATUS_INVALID_ARGS
#define STS_INTERNAL_ERROR      IMB_STATUS_INTERNAL_ERROR
#define STS_ERROR               IMB_STATUS_ERROR

#define MAX_TAG_LEN             IMB_MAX_TAG_LEN
#define GCM_IV_DATA_LEN         IMB_GCM_IV_DATA_LEN
#define GCM_128_KEY_LEN         IMB_GCM_128_KEY_LEN
#define GCM_192_KEY_LEN         IMB_GCM_192_KEY_LEN
#define GCM_256_KEY_LEN         IMB_GCM_256_KEY_LEN

#define DES_KEY_SCHED_SIZE      IMB_DES_KEY_SCHED_SIZE
#define DES_BLOCK_SIZE          IMB_DES_BLOCK_SIZE

#define AES_BLOCK_SIZE          IMB_AES_BLOCK_SIZE

#define SHA1_DIGEST_SIZE_IN_BYTES   IMB_SHA1_DIGEST_SIZE_IN_BYTES
#define SHA224_DIGEST_SIZE_IN_BYTES IMB_SHA224_DIGEST_SIZE_IN_BYTES
#define SHA256_DIGEST_SIZE_IN_BYTES IMB_SHA256_DIGEST_SIZE_IN_BYTES
#define SHA384_DIGEST_SIZE_IN_BYTES IMB_SHA384_DIGEST_SIZE_IN_BYTES
#define SHA512_DIGEST_SIZE_IN_BYTES IMB_SHA512_DIGEST_SIZE_IN_BYTES

#define SHA1_BLOCK_SIZE    IMB_SHA1_BLOCK_SIZE
#define SHA_256_BLOCK_SIZE IMB_SHA_256_BLOCK_SIZE
#define SHA_384_BLOCK_SIZE IMB_SHA_384_BLOCK_SIZE
#define SHA_512_BLOCK_SIZE IMB_SHA_512_BLOCK_SIZE

#define KASUMI_KEY_SIZE    IMB_KASUMI_KEY_SIZE
#define KASUMI_IV_SIZE     IMB_KASUMI_IV_SIZE
#define KASUMI_BLOCK_SIZE  IMB_KASUMI_BLOCK_SIZE
#define KASUMI_DIGEST_SIZE IMB_KASUMI_DIGEST_SIZE

#define DOCSIS_CRC32_MIN_ETH_PDU_SIZE IMB_DOCSIS_CRC32_MIN_ETH_PDU_SIZE
#define DOCSIS_CRC32_TAG_SIZE         IMB_DOCSIS_CRC32_TAG_SIZE

/* Previous fields in IMB_JOB/JOB_AES_HMAC */
#define aes_enc_key_expanded enc_keys
#define aes_dec_key_expanded dec_keys
#define aes_key_len_in_bytes key_len_in_bytes
#endif /* !NO_COMPAT_IMB_API_053 */

typedef enum {
        IMB_CIPHER_CBC = 1,
        IMB_CIPHER_CNTR,
        IMB_CIPHER_NULL,
        IMB_CIPHER_DOCSIS_SEC_BPI,
        IMB_CIPHER_GCM,
        IMB_CIPHER_CUSTOM,
        IMB_CIPHER_DES,
        IMB_CIPHER_DOCSIS_DES,
        IMB_CIPHER_CCM,
        IMB_CIPHER_DES3,
        IMB_CIPHER_PON_AES_CNTR,
        IMB_CIPHER_ECB,
        IMB_CIPHER_CNTR_BITLEN,       /**< 128-EEA2/NEA2 (3GPP) */
        IMB_CIPHER_ZUC_EEA3,          /**< 128-EEA3/NEA3 (3GPP) */
        IMB_CIPHER_SNOW3G_UEA2_BITLEN,/**< 128-UEA2 (3GPP) */
        IMB_CIPHER_KASUMI_UEA1_BITLEN,/**< 128-UEA1 (3GPP) */
        IMB_CIPHER_CBCS_1_9,          /**< MPEG CENC (ISO 23001-7) */
        IMB_CIPHER_CHACHA20,
        IMB_CIPHER_CHACHA20_POLY1305, /**< AEAD CHACHA20 */
        IMB_CIPHER_CHACHA20_POLY1305_SGL, /**< AEAD CHACHA20 with SGL support*/
        IMB_CIPHER_SNOW_V,
        IMB_CIPHER_SNOW_V_AEAD,
        IMB_CIPHER_GCM_SGL,
        IMB_CIPHER_NUM
} IMB_CIPHER_MODE;

typedef enum {
        IMB_DIR_ENCRYPT = 1,
        IMB_DIR_DECRYPT
} IMB_CIPHER_DIRECTION;

typedef enum {
        IMB_AUTH_HMAC_SHA_1 = 1,    /**< HMAC-SHA1 */
        IMB_AUTH_HMAC_SHA_224,      /**< HMAC-SHA224 */
        IMB_AUTH_HMAC_SHA_256,      /**< HMAC-SHA256 */
        IMB_AUTH_HMAC_SHA_384,      /**< HMAC-SHA384 */
        IMB_AUTH_HMAC_SHA_512,      /**< HMAC-SHA512 */
        IMB_AUTH_AES_XCBC,
        IMB_AUTH_MD5,               /**< HMAC-MD5 */
        IMB_AUTH_NULL,
        IMB_AUTH_AES_GMAC,
        IMB_AUTH_CUSTOM,
        IMB_AUTH_AES_CCM,            /**< AES128-CCM */
        IMB_AUTH_AES_CMAC,           /**< AES128-CMAC */
        IMB_AUTH_SHA_1,              /**< SHA1 */
        IMB_AUTH_SHA_224,            /**< SHA224 */
        IMB_AUTH_SHA_256,            /**< SHA256 */
        IMB_AUTH_SHA_384,            /**< SHA384 */
        IMB_AUTH_SHA_512,            /**< SHA512 */
        IMB_AUTH_AES_CMAC_BITLEN,    /**< 128-EIA2/NIA2 (3GPP) */
        IMB_AUTH_PON_CRC_BIP,
        IMB_AUTH_ZUC_EIA3_BITLEN,    /**< 128-EIA3/NIA3 (3GPP) */
        IMB_AUTH_DOCSIS_CRC32,       /**< with DOCSIS_SEC_BPI only */
        IMB_AUTH_SNOW3G_UIA2_BITLEN, /**< 128-UIA2 (3GPP) */
        IMB_AUTH_KASUMI_UIA1,        /**< 128-UIA1 (3GPP) */
        IMB_AUTH_AES_GMAC_128,       /**< AES-GMAC (128-bit key) */
        IMB_AUTH_AES_GMAC_192,       /**< AES-GMAC (192-bit key) */
        IMB_AUTH_AES_GMAC_256,       /**< AES-GMAC (256-bit key) */
        IMB_AUTH_AES_CMAC_256,       /**< AES256-CMAC */
        IMB_AUTH_POLY1305,           /**< POLY1305 */
        IMB_AUTH_CHACHA20_POLY1305,  /**< AEAD POLY1305 */
        IMB_AUTH_CHACHA20_POLY1305_SGL, /**< AEAD CHACHA20 with SGL support */
        IMB_AUTH_ZUC256_EIA3_BITLEN,    /**< 256-EIA3/NIA3 (3GPP) */
        IMB_AUTH_SNOW_V_AEAD,           /**< SNOW-V-AEAD */
        IMB_AUTH_GCM_SGL,               /**< AES-GCM with SGL support */
        IMB_AUTH_CRC32_ETHERNET_FCS,    /**< CRC32-ETHERNET-FCS */
        IMB_AUTH_CRC32_SCTP,            /**< CRC32-SCTP */
        IMB_AUTH_CRC32_WIMAX_OFDMA_DATA,/**< CRC32-WIMAX-OFDMA-DATA */
        IMB_AUTH_CRC24_LTE_A,           /**< CRC32-LTE-A */
        IMB_AUTH_CRC24_LTE_B,           /**< CRC32-LTE-B */
        IMB_AUTH_CRC16_X25,             /**< CRC16-X25 */
        IMB_AUTH_CRC16_FP_DATA,         /**< CRC16-FP-DATA */
        IMB_AUTH_CRC11_FP_HEADER,       /**< CRC11-FP-HEADER */
        IMB_AUTH_CRC10_IUUP_DATA,       /**< CRC10-IUUP-DATA */
        IMB_AUTH_CRC8_WIMAX_OFDMA_HCS,  /**< CRC8-WIMAX-OFDMA-HCS */
        IMB_AUTH_CRC7_FP_HEADER,        /**< CRC7-FP-HEADER */
        IMB_AUTH_CRC6_IUUP_HEADER,      /**< CRC6-IUUP-HEADER */
        IMB_AUTH_GHASH,                 /**< GHASH */
        IMB_AUTH_NUM
} IMB_HASH_ALG;

typedef enum {
        IMB_ORDER_CIPHER_HASH = 1,
        IMB_ORDER_HASH_CIPHER
} IMB_CHAIN_ORDER;

typedef enum {
        IMB_KEY_128_BYTES = 16,
        IMB_KEY_192_BYTES = 24,
        IMB_KEY_256_BYTES = 32
} IMB_KEY_SIZE_BYTES;

typedef enum {
        IMB_SGL_INIT = 0,
        IMB_SGL_UPDATE,
        IMB_SGL_COMPLETE,
        IMB_SGL_ALL
} IMB_SGL_STATE;

/**
 * Input/output SGL segment structure.
 */
struct IMB_SGL_IOV {
    const void *in; /**< Input segment */
    void *out; /**< Output segment */
    uint64_t len; /** Length of segment */
};

/**
 * Job structure.
 *
 * For AES, enc_keys and dec_keys are
 * expected to point to expanded keys structure.
 * - AES-CTR, AES-ECB and AES-CCM, only enc_keys is used
 * - DOCSIS (AES-CBC + AES-CFB), both pointers are used
 *   enc_keys has to be set always for the partial block
 *
 * For DES, enc_keys and dec_keys are
 * expected to point to DES key schedule.
 * - same key schedule used for enc and dec operations
 *
 * For 3DES, enc_keys and dec_keys are
 * expected to point to an array of 3 pointers for
 * the corresponding 3 key schedules.
 * - same key schedule used for enc and dec operations
 */

typedef struct IMB_JOB {
        const void *enc_keys;  /**< Encryption key pointer */
        const void *dec_keys;  /**< Decryption key pointer */
        uint64_t key_len_in_bytes; /**< Key length in bytes */
        union {
                const uint8_t *src; /**< Input buffer.
                                May be ciphertext or plaintext.
                                In-place ciphering allowed. */
                const struct IMB_SGL_IOV *sgl_io_segs;
                /**< Pointer to array of input/output SGL segments */
        };
        union {
                uint8_t *dst; /**< Output buffer.
                               May be ciphertext or plaintext.
                               In-place ciphering allowed, i.e. dst = src. */
                uint64_t num_sgl_io_segs;
                /**< Number of input/output SGL segments */
        };
        union {
                uint64_t cipher_start_src_offset_in_bytes;
                /**< Offset into input buffer to start ciphering (in bytes) */
                uint64_t cipher_start_src_offset_in_bits;
                /**< Offset into input buffer to start ciphering (in bits) */
                uint64_t cipher_start_offset_in_bits;
                /**< Offset into input buffer to start ciphering (in bits) */
        }; /**< Offset into input buffer to start ciphering */
        union {
                uint64_t msg_len_to_cipher_in_bytes;
                /**< Length of message to cipher (in bytes) */
                uint64_t msg_len_to_cipher_in_bits;
                /**< Length of message to cipher (in bits) */
        }; /**< Length of message to cipher */
        uint64_t hash_start_src_offset_in_bytes;
        union {
                uint64_t msg_len_to_hash_in_bytes;
                /**< Length of message to hash (in bytes) */
                uint64_t msg_len_to_hash_in_bits;
                /**< Length of message to hash (in bits) */
        }; /**< Length of message to hash */
        const uint8_t *iv;	/**< Initialization Vector (IV) */
        uint64_t iv_len_in_bytes; /**< IV length in bytes */
        uint8_t *auth_tag_output; /**< Authentication tag output */
        uint64_t auth_tag_output_len_in_bytes; /**< Authentication tag output
	                                            length in bytes */

        /* Start hash algorithm-specific fields */
        union {
                struct _HMAC_specific_fields {
                        const uint8_t *_hashed_auth_key_xor_ipad;
                        /**< Hashed result of HMAC key xor'd
			 * with ipad (0x36). */
                        const uint8_t *_hashed_auth_key_xor_opad;
                        /**< Hashed result of HMAC key xor'd
			 * with opad (0x5c). */
                } HMAC; /**< HMAC specific fields */
                struct _AES_XCBC_specific_fields {
                        const uint32_t *_k1_expanded;
                        /**< k1 expanded key pointer (16-byte aligned) */
                        const uint8_t *_k2;
                        /**< k2 expanded key pointer (16-byte aligned) */
                        const uint8_t *_k3;
                        /**< k3 expanded key pointer (16-byte aligned) */
                } XCBC; /**< AES-XCBC specific fields */
                struct _AES_CCM_specific_fields {
                        const void *aad;
                        /**< Additional Authentication Data (AAD) */
                        uint64_t aad_len_in_bytes; /**< Length of AAD */
                } CCM; /**< AES-CCM specific fields */
                struct _AES_CMAC_specific_fields {
                        const void *_key_expanded;
                        /**< Expanded key (16-byte aligned) */
                        const void *_skey1; /**< S key 1 (16-byte aligned) */
                        const void *_skey2; /**< S key 2 (16-byte aligned) */
                } CMAC; /**< AES-CMAC specific fields */
                struct _AES_GCM_specific_fields {
                        const void *aad;
                        /**< Additional Authentication Data (AAD) */
                        uint64_t aad_len_in_bytes;    /**< Length of AAD */
                        struct gcm_context_data *ctx;
                        /**< AES-GCM context (for SGL only) */
                } GCM; /**< AES-GCM specific fields */
                struct _ZUC_EIA3_specific_fields {
                        const uint8_t *_key;
                        /**< Authentication key (16-byte aligned) */
                        const uint8_t *_iv;
                        /**< Authentication 25-byte IV (16-byte aligned) */
                        const uint8_t *_iv23;
                        /**< Authentication 23-byte IV (16-byte aligned) */
                } ZUC_EIA3; /**< ZUC-EIA3 specific fields */
                struct _SNOW3G_UIA2_specific_fields {
                        const void *_key;
                        /**< Authentication key (16-byte aligned) */
                        const void *_iv;
                        /**< Authentication IV (16-byte aligned) */
                } SNOW3G_UIA2; /**< SNOW3G-UIA2 specific fields */
                struct _KASUMI_UIA1_specific_fields {
                        const void *_key;
                        /**< Authentication key (16-byte aligned) */
                } KASUMI_UIA1; /**< KASUMI-UIA2 specific fields */
                struct _AES_GMAC_specific_fields {
                        const struct gcm_key_data *_key;
                        /**< Authentication key */
                        const void *_iv;
                        /**< Authentication IV */
                        uint64_t iv_len_in_bytes;
                        /**< Authentication IV length in bytes */
                } GMAC; /**< AES-GMAC specific fields */
                struct _GHASH_specific_fields {
                        const struct gcm_key_data *_key;
                        /**< Expanded GHASH key */
                        const void *_init_tag; /**< initial tag value */
                } GHASH; /**< GHASH specific fields */
                struct _POLY1305_specific_fields {
                        const void *_key;
                        /**< Poly1305 key */
                } POLY1305; /**< Poly1305 specific fields */
                struct _CHACHA20_POLY1305_specific_fields {
                        const void *aad;
                        /**< Additional Authentication Data (AAD) */
                        uint64_t aad_len_in_bytes;
                        /**< Length of AAD */
                        struct chacha20_poly1305_context_data *ctx;
                        /**< Chacha20-Poly1305 context (for SGL only) */
                } CHACHA20_POLY1305; /**< Chacha20-Poly1305 specific fields */
                struct _SNOW_V_AEAD_specific_fields {
                        const void *aad;
                        /**< Additional Authentication Data (AAD) */
                        uint64_t aad_len_in_bytes;
                        /**< Length of AAD */
                        void *reserved;
                        /**< Reserved bytes */
                } SNOW_V_AEAD; /**< SNOW-V AEAD specific fields */
        } u; /**< Hash algorithm-specific fields */

        IMB_STATUS status; /**< Job status */
        IMB_CIPHER_MODE cipher_mode; /**< Cipher mode */
        IMB_CIPHER_DIRECTION cipher_direction; /**< Cipher direction */
        IMB_HASH_ALG hash_alg; /**< Hashing algorithm */
        IMB_CHAIN_ORDER chain_order;
        /**< Chain order (IMB_ORDER_CIPHER_HASH / IMB_ORDER_HASH_CIPHER).*/

        void *user_data; /**< Pointer 1 to user data */
        void *user_data2; /**< Pointer 2 to user data */

        int (*cipher_func)(struct IMB_JOB *);
        /**< Customer cipher function */
        int (*hash_func)(struct IMB_JOB *);
        /**< Customer hash function */

        IMB_SGL_STATE sgl_state;
        /**< SGL state (IMB_SGL_INIT/IMB_SGL_UPDATE/IMB_SGL_COMPLETE/
                        IMB_SGL_ALL) */

        union {
                struct _CBCS_specific_fields {
                        void *next_iv;
                        /**< Pointer to next IV (last ciphertext block) */
                } CBCS; /**< CBCS specific fields */
        } cipher_fields; /**< Cipher algorithm-specific fields */
} IMB_JOB;


/* KASUMI */

/* 64 precomputed words for key schedule */
#define KASUMI_KEY_SCHEDULE_SIZE  64

/**
 * Structure to maintain internal key scheduling
 */
typedef struct kasumi_key_sched_s {
    /**< Kasumi internal scheduling */
    uint16_t sk16[KASUMI_KEY_SCHEDULE_SIZE];      /**< key schedule */
    uint16_t msk16[KASUMI_KEY_SCHEDULE_SIZE];     /**< modified key schedule */
} kasumi_key_sched_t;

/* GCM data structures */
#define IMB_GCM_BLOCK_LEN   16

/**
 * @brief holds GCM operation context
 *
 * init, update and finalize context data
 */
struct gcm_context_data {
        uint8_t  aad_hash[IMB_GCM_BLOCK_LEN];
        uint64_t aad_length;
        uint64_t in_length;
        uint8_t  partial_block_enc_key[IMB_GCM_BLOCK_LEN];
        uint8_t  orig_IV[IMB_GCM_BLOCK_LEN];
        uint8_t  current_counter[IMB_GCM_BLOCK_LEN];
        uint64_t partial_block_length;
};
#undef IMB_GCM_BLOCK_LEN

/**
 * @brief holds Chacha20-Poly1305 operation context
 */
struct chacha20_poly1305_context_data {
        uint64_t hash[3]; /**< Intermediate computation of hash value */
        uint64_t aad_len; /**< Total AAD length */
        uint64_t hash_len; /**< Total length to digest (excluding AAD) */
        uint8_t last_ks[64]; /**< Last 64 bytes of KS */
        uint8_t poly_key[32]; /**< Poly key */
        uint8_t poly_scratch[16]; /**< Scratchpad to compute Poly on 16 bytes */
        uint64_t last_block_count; /**< Last block count used in last segment */
        uint64_t remain_ks_bytes;/**< Amount of bytes still to use of keystream
				   (up to 63 bytes) */
        uint64_t remain_ct_bytes; /**< Amount of ciphertext bytes still to use
				    of previous segment to authenticate
				    (up to 16 bytes) */
        uint8_t IV[12]; /**< IV (12 bytes) */
};

/**
 * Authenticated Tag Length in bytes.
 * Valid values are 16 (most likely), 12 or 8.
 */
#define IMB_MAX_TAG_LEN (16)

/**
 * IV data is limited to 16 bytes as follows:
 * 12 bytes is provided by an application -
 *    pre-counter block j0: 4 byte salt (from Security Association)
 *    concatenated with 8 byte Initialization Vector (from IPSec ESP
 *    Payload).
 * 4 byte value 0x00000001 is padded automatically by the library -
 *    there is no need to add these 4 bytes on application side anymore.
 */
#define IMB_GCM_IV_DATA_LEN (12)

#define IMB_GCM_128_KEY_LEN (16)
#define IMB_GCM_192_KEY_LEN (24)
#define IMB_GCM_256_KEY_LEN (32)

#define IMB_GCM_ENC_KEY_LEN 16
#define IMB_GCM_KEY_SETS    (15) /**< exp key + 14 exp round keys*/

/**
 * @brief holds intermediate key data needed to improve performance
 *
 * gcm_key_data hold internal key information used by gcm128, gcm192 and gcm256.
 */
#ifdef __WIN32
__declspec(align(64))
#endif /* WIN32 */
struct gcm_key_data {
        uint8_t expanded_keys[IMB_GCM_ENC_KEY_LEN * IMB_GCM_KEY_SETS];
        union {
                /**< Storage for precomputed hash keys */
                struct {
                        /**
                         * This is needed for schoolbook multiply purposes.
                         * (HashKey<<1 mod poly), (HashKey^2<<1 mod poly), ...,
                         * (Hashkey^48<<1 mod poly)
                         */
                        uint8_t shifted_hkey[IMB_GCM_ENC_KEY_LEN * 8];
                        /**
                         * This is needed for Karatsuba multiply purposes.
                         * Storage for XOR of High 64 bits and low 64 bits
                         * of HashKey mod poly.
                         *
                         * (HashKey<<1 mod poly), (HashKey^2<<1 mod poly), ...,
                         * (Hashkey^128<<1 mod poly)
                         */
                        uint8_t shifted_hkey_k[IMB_GCM_ENC_KEY_LEN * 8];
                } sse_avx;
                struct {
                        /**
                         * This is needed for schoolbook multiply purposes.
                         * (HashKey<<1 mod poly), (HashKey^2<<1 mod poly), ...,
                         * (Hashkey^48<<1 mod poly)
                         */
                        uint8_t shifted_hkey[IMB_GCM_ENC_KEY_LEN * 8];
                } avx2_avx512;
                struct {
                        /**
                         * (HashKey<<1 mod poly), (HashKey^2<<1 mod poly), ...,
                         * (Hashkey^48<<1 mod poly)
                         */
                        uint8_t shifted_hkey[IMB_GCM_ENC_KEY_LEN * 48];
                } vaes_avx512;
        } ghash_keys;
}
#ifdef LINUX
__attribute__((aligned(64)));
#else
;
#endif

#undef IMB_GCM_ENC_KEY_LEN
#undef IMB_GCM_KEY_SETS

/* API data type definitions */
struct IMB_MGR;

typedef void (*init_mb_mgr_t)(struct IMB_MGR *);
typedef IMB_JOB *(*get_next_job_t)(struct IMB_MGR *);
typedef IMB_JOB *(*submit_job_t)(struct IMB_MGR *);
typedef IMB_JOB *(*get_completed_job_t)(struct IMB_MGR *);
typedef IMB_JOB *(*flush_job_t)(struct IMB_MGR *);
typedef uint32_t (*queue_size_t)(struct IMB_MGR *);
typedef uint32_t (*burst_fn_t)(struct IMB_MGR *,
                               const uint32_t,
                               struct IMB_JOB **);
typedef uint32_t (*submit_cipher_burst_t)(struct IMB_MGR *,
                                          struct IMB_JOB *,
                                          const uint32_t,
                                          const IMB_CIPHER_MODE cipher,
                                          const IMB_CIPHER_DIRECTION dir,
                                          const IMB_KEY_SIZE_BYTES key_size);
typedef uint32_t (*submit_hash_burst_t)(struct IMB_MGR *,
                                        struct IMB_JOB *,
                                        const uint32_t,
                                        const IMB_HASH_ALG hash);
typedef void (*keyexp_t)(const void *, void *, void *);
typedef void (*cmac_subkey_gen_t)(const void *, void *, void *);
typedef void (*hash_one_block_t)(const void *, void *);
typedef void (*hash_fn_t)(const void *, const uint64_t, void *);
typedef void (*xcbc_keyexp_t)(const void *, void *, void *, void *);
typedef int (*des_keysched_t)(uint64_t *, const void *);
typedef void (*aes_cfb_t)(void *, const void *, const void *, const void *,
                          uint64_t);
typedef void (*aes_gcm_enc_dec_t)(const struct gcm_key_data *,
                                  struct gcm_context_data *,
                                  uint8_t *, uint8_t const *, uint64_t,
                                  const uint8_t *, uint8_t const *, uint64_t,
                                  uint8_t *, uint64_t);
typedef void (*aes_gcm_enc_dec_iv_t)(const struct gcm_key_data *,
                                     struct gcm_context_data *, uint8_t *,
                                     uint8_t const *, const uint64_t,
                                     const uint8_t *, uint8_t const *,
                                     const uint64_t, uint8_t *,
                                     const uint64_t, const uint64_t);
typedef void (*aes_gcm_init_t)(const struct gcm_key_data *,
                               struct gcm_context_data *,
                               const uint8_t *, uint8_t const *, uint64_t);
typedef void (*aes_gcm_init_var_iv_t)(const struct gcm_key_data *,
                                      struct gcm_context_data *,
                                      const uint8_t *, const uint64_t,
                                      const uint8_t *, const uint64_t);
typedef void (*aes_gcm_enc_dec_update_t)(const struct gcm_key_data *,
                                         struct gcm_context_data *,
                                         uint8_t *, const uint8_t *, uint64_t);
typedef void (*aes_gcm_enc_dec_finalize_t)(const struct gcm_key_data *,
                                           struct gcm_context_data *,
                                           uint8_t *, uint64_t);
typedef void (*aes_gcm_precomp_t)(struct gcm_key_data *);
typedef void (*aes_gcm_pre_t)(const void *, struct gcm_key_data *);

typedef void (*aes_gmac_init_t)(const struct gcm_key_data *,
                                struct gcm_context_data *,
                                const uint8_t *, const uint64_t);
typedef void (*aes_gmac_update_t)(const struct gcm_key_data *,
                                  struct gcm_context_data *,
                                  const uint8_t *, const uint64_t);
typedef void (*aes_gmac_finalize_t)(const struct gcm_key_data *,
                                  struct gcm_context_data *,
                                  uint8_t *, const uint64_t);

typedef void (*chacha_poly_init_t)(const void *,
                                   struct chacha20_poly1305_context_data *,
                                   const void *, const void *, const uint64_t);
typedef void (*chacha_poly_enc_dec_update_t)(const void *,
                                     struct chacha20_poly1305_context_data *,
                                     void *, const void *, const uint64_t);
typedef void (*chacha_poly_finalize_t)(struct chacha20_poly1305_context_data *,
                                    void *, const uint64_t);
typedef void (*ghash_t)(const struct gcm_key_data *, const void *,
                        const uint64_t, void *, const uint64_t);

typedef void (*zuc_eea3_1_buffer_t)(const void *, const void *, const void *,
                                    void *, const uint32_t);

typedef void (*zuc_eea3_4_buffer_t)(const void * const *, const void * const *,
                                    const void * const *, void **,
                                    const uint32_t *);

typedef void (*zuc_eea3_n_buffer_t)(const void * const *, const void * const *,
                                    const void * const *, void **,
                                    const uint32_t *, const uint32_t);

typedef void (*zuc_eia3_1_buffer_t)(const void *, const void *, const void *,
                                    const uint32_t, uint32_t *);

typedef void (*zuc_eia3_n_buffer_t)(const void * const *, const void * const *,
                                    const void * const *,
                                    const uint32_t *, uint32_t **,
                                    const uint32_t);


typedef void (*kasumi_f8_1_buffer_t)(const kasumi_key_sched_t *,
                                     const uint64_t, const void *, void *,
                                     const uint32_t);
typedef void (*kasumi_f8_1_buffer_bit_t)(const kasumi_key_sched_t *,
                                         const uint64_t, const void *,
                                         void *,
                                         const uint32_t, const uint32_t);
typedef void (*kasumi_f8_2_buffer_t)(const kasumi_key_sched_t *,
                                     const uint64_t,  const uint64_t,
                                     const void *, void *,
                                     const uint32_t,
                                     const void *, void *,
                                     const uint32_t);
typedef void (*kasumi_f8_3_buffer_t)(const kasumi_key_sched_t *,
                                     const uint64_t,  const uint64_t,
                                     const uint64_t,
                                     const void *, void *,
                                     const void *, void *,
                                     const void *, void *,
                                     const uint32_t);
typedef void (*kasumi_f8_4_buffer_t)(const kasumi_key_sched_t *,
                                     const uint64_t,  const uint64_t,
                                     const uint64_t,  const uint64_t,
                                     const void *, void *,
                                     const void *, void *,
                                     const void *, void *,
                                     const void *, void *,
                                     const uint32_t);
typedef void (*kasumi_f8_n_buffer_t)(const kasumi_key_sched_t *,
                                     const uint64_t *, const void * const *,
                                     void **, const uint32_t *,
                                     const uint32_t);
typedef void (*kasumi_f9_1_buffer_user_t)(const kasumi_key_sched_t *,
                                          const uint64_t, const void *,
                                          const uint32_t, void *,
                                          const uint32_t);
typedef void (*kasumi_f9_1_buffer_t)(const kasumi_key_sched_t *,
                                     const void *,
                                     const uint32_t, void *);
typedef int (*kasumi_init_f8_key_sched_t)(const void *,
                                          kasumi_key_sched_t *);
typedef int (*kasumi_init_f9_key_sched_t)(const void *,
                                          kasumi_key_sched_t *);
typedef size_t (*kasumi_key_sched_size_t)(void);


/**
 * Snow3G key scheduling structure
 */
typedef struct snow3g_key_schedule_s {
        /* KEY */
        uint32_t k[4];
} snow3g_key_schedule_t;

typedef void (*snow3g_f8_1_buffer_t)(const snow3g_key_schedule_t *,
                                     const void *, const void *,
                                     void *, const uint32_t);

typedef void (*snow3g_f8_1_buffer_bit_t)(const snow3g_key_schedule_t *,
                                         const void *, const void *, void *,
                                         const uint32_t, const uint32_t);

typedef void (*snow3g_f8_2_buffer_t)(const snow3g_key_schedule_t *,
                                     const void *, const void *,
                                     const void *, void *, const uint32_t,
                                     const void *, void *, const uint32_t);

typedef void (*snow3g_f8_4_buffer_t)(const snow3g_key_schedule_t *,
                                     const void *, const void *, const void *,
                                     const void *, const void *, void *,
                                     const uint32_t, const void *, void *,
                                     const uint32_t, const void *, void *,
                                     const uint32_t, const void *, void *,
                                     const uint32_t);

typedef void (*snow3g_f8_8_buffer_t)(const snow3g_key_schedule_t *,
                                     const void *, const void *, const void *,
                                     const void *, const void *, const void *,
                                     const void *, const void *, const void *,
                                     void *, const uint32_t, const void *,
                                     void *, const uint32_t, const void *,
                                     void *, const uint32_t, const void *,
                                     void *, const uint32_t, const void *,
                                     void *, const uint32_t, const void *,
                                     void *, const uint32_t, const void *,
                                     void *, const uint32_t, const void *,
                                     void *, const uint32_t);

typedef void
(*snow3g_f8_8_buffer_multikey_t)(const snow3g_key_schedule_t * const [],
                                 const void * const [], const void * const [],
                                 void *[], const uint32_t[]);

typedef void (*snow3g_f8_n_buffer_t)(const snow3g_key_schedule_t *,
                                     const void * const [],
                                     const void * const [],
                                     void *[], const uint32_t[],
                                     const uint32_t);

typedef void
(*snow3g_f8_n_buffer_multikey_t)(const snow3g_key_schedule_t * const [],
                                 const void * const [],
                                 const void * const [],
                                 void *[], const uint32_t[],
                                 const uint32_t);

typedef void (*snow3g_f9_1_buffer_t)(const snow3g_key_schedule_t *,
                                     const void *, const void *,
                                     const uint64_t, void *);

typedef int (*snow3g_init_key_sched_t)(const void *,
                                       snow3g_key_schedule_t *);

typedef size_t (*snow3g_key_sched_size_t)(void);

typedef uint32_t (*hec_32_t)(const uint8_t *);
typedef uint64_t (*hec_64_t)(const uint8_t *);

typedef uint32_t (*crc32_fn_t)(const void *, const uint64_t);
/* Multi-buffer manager flags passed to alloc_mb_mgr() */

#define IMB_FLAG_SHANI_OFF (1ULL << 0) /**< disable use of SHANI extension */
#define IMB_FLAG_AESNI_OFF (1ULL << 1) /**< disable use of AESNI extension */
#define IMB_FLAG_GFNI_OFF (1ULL << 2) /**< disable use of GFNI extension */

/**
 * Multi-buffer manager detected features
 * - if bit is set then hardware supports given extension
 * - valid after call to init_mb_mgr() or alloc_mb_mgr()
 * - some HW supported features can be disabled via IMB_FLAG_xxx (see above)
 */

#define IMB_FEATURE_SHANI      (1ULL << 0)
#define IMB_FEATURE_AESNI      (1ULL << 1)
#define IMB_FEATURE_PCLMULQDQ  (1ULL << 2)
#define IMB_FEATURE_CMOV       (1ULL << 3)
#define IMB_FEATURE_SSE4_2     (1ULL << 4)
#define IMB_FEATURE_AVX        (1ULL << 5)
#define IMB_FEATURE_AVX2       (1ULL << 6)
#define IMB_FEATURE_AVX512F    (1ULL << 7)
#define IMB_FEATURE_AVX512DQ   (1ULL << 8)
#define IMB_FEATURE_AVX512CD   (1ULL << 9)
#define IMB_FEATURE_AVX512BW   (1ULL << 10)
#define IMB_FEATURE_AVX512VL   (1ULL << 11)
#define IMB_FEATURE_AVX512_SKX (IMB_FEATURE_AVX512F | IMB_FEATURE_AVX512DQ | \
                                IMB_FEATURE_AVX512CD | IMB_FEATURE_AVX512BW | \
                                IMB_FEATURE_AVX512VL)
#define IMB_FEATURE_VAES       (1ULL << 12)
#define IMB_FEATURE_VPCLMULQDQ (1ULL << 13)
#define IMB_FEATURE_SAFE_DATA  (1ULL << 14)
#define IMB_FEATURE_SAFE_PARAM (1ULL << 15)
#define IMB_FEATURE_GFNI       (1ULL << 16)
#define IMB_FEATURE_AVX512_IFMA (1ULL << 17)
#define IMB_FEATURE_BMI2       (1ULL << 18)
#define IMB_FEATURE_AESNI_EMU  (1ULL << 19)
#define IMB_FEATURE_SELF_TEST  (1ULL << 20)     /* self-test feature present */
#define IMB_FEATURE_SELF_TEST_PASS (1ULL << 21) /* self-test passed */

/**
 * CPU flags needed for each implementation
 */
#define IMB_CPUFLAGS_NO_AESNI   (IMB_FEATURE_SSE4_2 | IMB_FEATURE_CMOV)
#define IMB_CPUFLAGS_SSE        (IMB_CPUFLAGS_NO_AESNI | IMB_FEATURE_AESNI | \
                                 IMB_FEATURE_PCLMULQDQ)
#define IMB_CPUFLAGS_SSE_T2     (IMB_CPUFLAGS_SSE | IMB_FEATURE_SHANI)
#define IMB_CPUFLAGS_SSE_T3     (IMB_CPUFLAGS_SSE_T2 | IMB_FEATURE_GFNI)
#define IMB_CPUFLAGS_AVX        (IMB_CPUFLAGS_SSE | IMB_FEATURE_AVX)
#define IMB_CPUFLAGS_AVX2       (IMB_CPUFLAGS_AVX | IMB_FEATURE_AVX2 | \
                                 IMB_FEATURE_BMI2)
#define IMB_CPUFLAGS_AVX512     (IMB_CPUFLAGS_AVX2 | IMB_FEATURE_AVX512_SKX)
#define IMB_CPUFLAGS_AVX512_T2  (IMB_CPUFLAGS_AVX512 | IMB_FEATURE_VAES | \
                                 IMB_FEATURE_VPCLMULQDQ | IMB_FEATURE_GFNI | \
                                 IMB_FEATURE_AVX512_IFMA | IMB_FEATURE_SHANI)
#define IMB_CPUFLAGS_AVX2_T2    (IMB_CPUFLAGS_AVX2 | IMB_FEATURE_SHANI | \
                                 IMB_FEATURE_VAES | IMB_FEATURE_VPCLMULQDQ | \
                                 IMB_FEATURE_GFNI)
#define IMB_CPUFLAGS_AVX_T2     (IMB_CPUFLAGS_AVX | IMB_FEATURE_SHANI | \
                                 IMB_FEATURE_GFNI)

/* TOP LEVEL (IMB_MGR) Data structure fields */

#define IMB_MAX_BURST_SIZE 128
#define IMB_MAX_JOBS (IMB_MAX_BURST_SIZE * 2)

typedef struct IMB_MGR {

        uint64_t flags;	  /**< passed to alloc_mb_mgr() */
        uint64_t features; /**< reflects features of multi-buffer instance */

        uint64_t reserved[5]; /**< reserved for the future */
        uint32_t used_arch; /**< Architecture being used */

	int imb_errno; /**< per mb_mgr error status */

        /**
         * ARCH handlers / API
         * Careful as changes here can break ABI compatibility
         * (always include function pointers at the end of the list,
         * before "earliest_job")
         */
        get_next_job_t          get_next_job;
        submit_job_t            submit_job;
        submit_job_t            submit_job_nocheck;
        get_completed_job_t     get_completed_job;
        flush_job_t             flush_job;
        queue_size_t            queue_size;
        keyexp_t                keyexp_128;
        keyexp_t                keyexp_192;
        keyexp_t                keyexp_256;
        cmac_subkey_gen_t       cmac_subkey_gen_128;
        xcbc_keyexp_t           xcbc_keyexp;
        des_keysched_t          des_key_sched;
        hash_one_block_t        sha1_one_block;
        hash_one_block_t        sha224_one_block;
        hash_one_block_t        sha256_one_block;
        hash_one_block_t        sha384_one_block;
        hash_one_block_t        sha512_one_block;
        hash_one_block_t        md5_one_block;
        hash_fn_t               sha1;
        hash_fn_t               sha224;
        hash_fn_t               sha256;
        hash_fn_t               sha384;
        hash_fn_t               sha512;
        aes_cfb_t               aes128_cfb_one;

        aes_gcm_enc_dec_t       gcm128_enc;
        aes_gcm_enc_dec_t       gcm192_enc;
        aes_gcm_enc_dec_t       gcm256_enc;
        aes_gcm_enc_dec_t       gcm128_dec;
        aes_gcm_enc_dec_t       gcm192_dec;
        aes_gcm_enc_dec_t       gcm256_dec;
        aes_gcm_init_t          gcm128_init;
        aes_gcm_init_t          gcm192_init;
        aes_gcm_init_t          gcm256_init;
        aes_gcm_enc_dec_update_t gcm128_enc_update;
        aes_gcm_enc_dec_update_t gcm192_enc_update;
        aes_gcm_enc_dec_update_t gcm256_enc_update;
        aes_gcm_enc_dec_update_t gcm128_dec_update;
        aes_gcm_enc_dec_update_t gcm192_dec_update;
        aes_gcm_enc_dec_update_t gcm256_dec_update;
        aes_gcm_enc_dec_finalize_t gcm128_enc_finalize;
        aes_gcm_enc_dec_finalize_t gcm192_enc_finalize;
        aes_gcm_enc_dec_finalize_t gcm256_enc_finalize;
        aes_gcm_enc_dec_finalize_t gcm128_dec_finalize;
        aes_gcm_enc_dec_finalize_t gcm192_dec_finalize;
        aes_gcm_enc_dec_finalize_t gcm256_dec_finalize;
        aes_gcm_precomp_t       gcm128_precomp;
        aes_gcm_precomp_t       gcm192_precomp;
        aes_gcm_precomp_t       gcm256_precomp;
        aes_gcm_pre_t           gcm128_pre;
        aes_gcm_pre_t           gcm192_pre;
        aes_gcm_pre_t           gcm256_pre;

        zuc_eea3_1_buffer_t eea3_1_buffer;
        zuc_eea3_4_buffer_t eea3_4_buffer;
        zuc_eea3_n_buffer_t eea3_n_buffer;
        zuc_eia3_1_buffer_t eia3_1_buffer;

        kasumi_f8_1_buffer_t      f8_1_buffer;
        kasumi_f8_1_buffer_bit_t  f8_1_buffer_bit;
        kasumi_f8_2_buffer_t      f8_2_buffer;
        kasumi_f8_3_buffer_t      f8_3_buffer;
        kasumi_f8_4_buffer_t      f8_4_buffer;
        kasumi_f8_n_buffer_t      f8_n_buffer;
        kasumi_f9_1_buffer_t      f9_1_buffer;
        kasumi_f9_1_buffer_user_t f9_1_buffer_user;
        kasumi_init_f8_key_sched_t kasumi_init_f8_key_sched;
        kasumi_init_f9_key_sched_t kasumi_init_f9_key_sched;
        kasumi_key_sched_size_t    kasumi_key_sched_size;

        snow3g_f8_1_buffer_bit_t snow3g_f8_1_buffer_bit;
        snow3g_f8_1_buffer_t snow3g_f8_1_buffer;
        snow3g_f8_2_buffer_t snow3g_f8_2_buffer;
        snow3g_f8_4_buffer_t snow3g_f8_4_buffer;
        snow3g_f8_8_buffer_t snow3g_f8_8_buffer;
        snow3g_f8_n_buffer_t snow3g_f8_n_buffer;
        snow3g_f8_8_buffer_multikey_t snow3g_f8_8_buffer_multikey;
        snow3g_f8_n_buffer_multikey_t snow3g_f8_n_buffer_multikey;
        snow3g_f9_1_buffer_t snow3g_f9_1_buffer;
        snow3g_init_key_sched_t snow3g_init_key_sched;
        snow3g_key_sched_size_t snow3g_key_sched_size;

        ghash_t                 ghash;
        zuc_eia3_n_buffer_t     eia3_n_buffer;
        aes_gcm_init_var_iv_t   gcm128_init_var_iv;
        aes_gcm_init_var_iv_t   gcm192_init_var_iv;
        aes_gcm_init_var_iv_t   gcm256_init_var_iv;

        aes_gmac_init_t         gmac128_init;
        aes_gmac_init_t         gmac192_init;
        aes_gmac_init_t         gmac256_init;
        aes_gmac_update_t       gmac128_update;
        aes_gmac_update_t       gmac192_update;
        aes_gmac_update_t       gmac256_update;
        aes_gmac_finalize_t     gmac128_finalize;
        aes_gmac_finalize_t     gmac192_finalize;
        aes_gmac_finalize_t     gmac256_finalize;
        hec_32_t                hec_32;
        hec_64_t                hec_64;
        cmac_subkey_gen_t       cmac_subkey_gen_256;
        aes_gcm_pre_t           ghash_pre;
        crc32_fn_t              crc32_ethernet_fcs;
        crc32_fn_t              crc16_x25;
        crc32_fn_t              crc32_sctp;
        crc32_fn_t              crc24_lte_a;
        crc32_fn_t              crc24_lte_b;
        crc32_fn_t              crc16_fp_data;
        crc32_fn_t              crc11_fp_header;
        crc32_fn_t              crc7_fp_header;
        crc32_fn_t              crc10_iuup_data;
        crc32_fn_t              crc6_iuup_header;
        crc32_fn_t              crc32_wimax_ofdma_data;
        crc32_fn_t              crc8_wimax_ofdma_hcs;

        chacha_poly_init_t           chacha20_poly1305_init;
        chacha_poly_enc_dec_update_t chacha20_poly1305_enc_update;
        chacha_poly_enc_dec_update_t chacha20_poly1305_dec_update;
        chacha_poly_finalize_t       chacha20_poly1305_finalize;

        burst_fn_t get_next_burst;
        burst_fn_t submit_burst;
        burst_fn_t submit_burst_nocheck;
        burst_fn_t flush_burst;
        submit_cipher_burst_t submit_cipher_burst;
        submit_cipher_burst_t submit_cipher_burst_nocheck;
        submit_hash_burst_t submit_hash_burst;
        submit_hash_burst_t submit_hash_burst_nocheck;

        /* in-order scheduler fields */
        int              earliest_job; /**< byte offset, -1 if none */
        int              next_job;     /**< byte offset */
        IMB_JOB     jobs[IMB_MAX_JOBS];

        /* out of order managers */
        void *aes128_ooo;
        void *aes192_ooo;
        void *aes256_ooo;
        void *docsis128_sec_ooo;
        void *docsis128_crc32_sec_ooo;
        void *docsis256_sec_ooo;
        void *docsis256_crc32_sec_ooo;
        void *des_enc_ooo;
        void *des_dec_ooo;
        void *des3_enc_ooo;
        void *des3_dec_ooo;
        void *docsis_des_enc_ooo;
        void *docsis_des_dec_ooo;

        void *hmac_sha_1_ooo;
        void *hmac_sha_224_ooo;
        void *hmac_sha_256_ooo;
        void *hmac_sha_384_ooo;
        void *hmac_sha_512_ooo;
        void *hmac_md5_ooo;
        void *aes_xcbc_ooo;
        void *aes_ccm_ooo;
        void *aes_cmac_ooo;
        void *zuc_eea3_ooo;
        void *zuc_eia3_ooo;
        void *aes128_cbcs_ooo;
        void *zuc256_eea3_ooo;
        void *zuc256_eia3_ooo;
        void *aes256_ccm_ooo;
        void *aes256_cmac_ooo;
        void *snow3g_uea2_ooo;
        void *snow3g_uia2_ooo;
        void *sha_1_ooo;
        void *sha_224_ooo;
        void *sha_256_ooo;
        void *sha_384_ooo;
        void *sha_512_ooo;
        void *end_ooo; /* add new out-of-order managers above this line */
} IMB_MGR;

/**
 * API definitions
 */


/**
 * @brief Get library version in string format
 *
 * @return library version string
 */
IMB_DLL_EXPORT const char *imb_get_version_str(void);

/**
 * @brief Get library version in numerical format
 *
 * Use IMB_VERSION() macro to compare this
 * numerical version against known library version.
 *
 * @return library version number
 */
IMB_DLL_EXPORT unsigned imb_get_version(void);


/**
 * @brief API to get error status
 *
 * @param mb_mgr Pointer to multi-buffer manager
 *
 * @retval Integer error type
 */
IMB_DLL_EXPORT int imb_get_errno(IMB_MGR *mb_mgr);

/**
 * @brief API to get description for \a errnum
 *
 * @param errnum error type
 *
 * @retval String description of \a errnum
 */
IMB_DLL_EXPORT const char *imb_get_strerror(int errnum);

/**
 * get_next_job returns a job object. This must be filled in and returned
 * via submit_job before get_next_job is called again.
 * After submit_job is called, one should call get_completed_job() at least
 * once (and preferably until it returns NULL).
 * get_completed_job and flush_job returns a job object. This job object ceases
 * to be usable at the next call to get_next_job
 */

/**
 * @brief Allocates memory for multi-buffer manager instance
 *
 * For binary compatibility between library versions
 * it is recommended to use this API.
 *
 * @param flags multi-buffer manager flags
 *     IMB_FLAG_SHANI_OFF - disable use (and detection) of SHA extensions,
 *                          currently SHANI is only available for SSE
 *     IMB_FLAG_AESNI_OFF - disable use (and detection) of AES extensions.
 *     IMB_FLAG_GFNI_OFF - disable use (and detection) of
 *                         Galois Field extensions.
 *
 * @return Pointer to allocated memory for IMB_MGR structure
 * @retval NULL on allocation error
 */
IMB_DLL_EXPORT IMB_MGR *alloc_mb_mgr(uint64_t flags);

/**
 * @brief Frees memory allocated previously by alloc_mb_mgr()
 *
 * @param [in] ptr Pointer to allocated MB_MGR structure
 *
 */
IMB_DLL_EXPORT void free_mb_mgr(IMB_MGR *ptr);

/**
 * @brief Calculates necessary memory size for IMB_MGR.
 *
 * @return Size for IMB_MGR (aligned to 64 bytes)
 */
IMB_DLL_EXPORT size_t imb_get_mb_mgr_size(void);

/**
 * @brief Initializes IMB_MGR pointers to out-of-order managers with
 *        use of externally allocated memory.
 *
 * imb_get_mb_mgr_size() should be called to know how much memory
 * should be allocated externally.
 *
 * init_mb_mgr_XXX() must be called after this function call,
 * whereas XXX is the desired architecture.
 *
 * @param [in] ptr a pointer to allocated memory
 * @param [in] flags multi-buffer manager flags
 *     IMB_FLAG_SHANI_OFF - disable use (and detection) of SHA extensions,
 *                          currently SHANI is only available for SSE
 *     IMB_FLAG_AESNI_OFF - disable use (and detection) of AES extensions.
 *     IMB_FLAG_GFNI_OFF - disable use (and detection)
 *                         of Galois Field extensions.
 *
 * @param [in] reset_mgr if 0, IMB_MGR structure is not cleared, else it is.
 *
 * @return Pointer to IMB_MGR structure
 */
IMB_DLL_EXPORT IMB_MGR *imb_set_pointers_mb_mgr(void *ptr, const uint64_t flags,
                                                const unsigned reset_mgr);

/**
 * @brief Retrieves the bitmask with the features supported by the library,
 *        without having to allocate/initialize IMB_MGR;
 *
 * @return Bitmask containing feature flags
 */
IMB_DLL_EXPORT uint64_t imb_get_feature_flags(void);

/**
 * @brief Initialize Multi-Buffer Manager structure.
 *
 * Must be called before calling JOB/BURST API.
 *
 * @param [in,out] state Pointer to IMB_MGR structure
 *                       For binary compatibility between library versions, it
 *                       is recommended to allocate the IMB_MGR structure using
 *                       the alloc_mb_mgr() API
 */
IMB_DLL_EXPORT void init_mb_mgr_avx(IMB_MGR *state);
/**
 * @copydoc init_mb_mgr_avx
 */
IMB_DLL_EXPORT void init_mb_mgr_avx2(IMB_MGR *state);
/**
 * @copydoc init_mb_mgr_avx
 */
IMB_DLL_EXPORT void init_mb_mgr_avx512(IMB_MGR *state);
/**
 * @copydoc init_mb_mgr_avx
 */
IMB_DLL_EXPORT void init_mb_mgr_sse(IMB_MGR *state);


/**
 * @brief Submit job for processing after validating.
 *
 * @param [in,out] state Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if no job completed
 *         If NULL, imb_get_errno() can be used to check for potential
 *         error conditions
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_avx(IMB_MGR *state);
/**
 * @copydoc submit_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_avx2(IMB_MGR *state);
/**
 * @copydoc submit_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_avx512(IMB_MGR *state);
/**
 * @copydoc submit_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_sse(IMB_MGR *state);

/**
 * @brief Submit job for processing without validating.
 *
 * This is more performant but less secure than submit_job_xxx()
 *
 * @param [in,out] state Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if no job completed
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_nocheck_avx(IMB_MGR *state);
/**
 * @copydoc submit_job_nocheck_avx
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_nocheck_avx2(IMB_MGR *state);
/**
 * @copydoc submit_job_nocheck_avx
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_nocheck_avx512(IMB_MGR *state);
/**
 * @copydoc submit_job_nocheck_avx
 */
IMB_DLL_EXPORT IMB_JOB *submit_job_nocheck_sse(IMB_MGR *state);

/**
 * @brief Force processing until next job in queue is completed.
 *
 * @param [in,out] state Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if no more jobs to process
 */
IMB_DLL_EXPORT IMB_JOB *flush_job_avx(IMB_MGR *state);
/**
 * @copydoc flush_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *flush_job_avx2(IMB_MGR *state);
/**
 * @copydoc flush_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *flush_job_avx512(IMB_MGR *state);
/**
 * @copydoc flush_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *flush_job_sse(IMB_MGR *state);

/**
 * @brief Get number of jobs queued to be processed.
 *
 * @param [in,out] state Pointer to initialized IMB_MGR structure
 *
 * @return Number of jobs in the queue
 */
IMB_DLL_EXPORT uint32_t queue_size_avx(IMB_MGR *state);
/**
 * @copydoc queue_size_avx
 */
IMB_DLL_EXPORT uint32_t queue_size_avx2(IMB_MGR *state);
/**
 * @copydoc queue_size_avx
 */
IMB_DLL_EXPORT uint32_t queue_size_avx512(IMB_MGR *state);
/**
 * @copydoc queue_size_avx
 */
IMB_DLL_EXPORT uint32_t queue_size_sse(IMB_MGR *state);

/**
 * @brief Get next completed job.
 *
 * @param [in,out] state Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if next job not complete
 */
IMB_DLL_EXPORT IMB_JOB *get_completed_job_avx(IMB_MGR *state);
/**
 * @copydoc get_completed_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *get_completed_job_avx2(IMB_MGR *state);
/**
 * @copydoc get_completed_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *get_completed_job_avx512(IMB_MGR *state);
/**
 * @copydoc get_completed_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *get_completed_job_sse(IMB_MGR *state);

/**
 * @brief Get next available job.
 *
 * @param [in,out] state Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to next free IMB_JOB in the queue
 */
IMB_DLL_EXPORT IMB_JOB *get_next_job_avx(IMB_MGR *state);
/**
 * @copydoc get_next_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *get_next_job_avx2(IMB_MGR *state);
/**
 * @copydoc get_next_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *get_next_job_avx512(IMB_MGR *state);
/**
 * @copydoc get_next_job_avx
 */
IMB_DLL_EXPORT IMB_JOB *get_next_job_sse(IMB_MGR *state);

/**
 * @brief Automatically initialize most performant
 *        Multi-buffer manager based on CPU features
 *
 * @param [in]  state Pointer to MB_MGR struct
 * @param [out] arch Pointer to arch enum to be set (can be NULL)
 *
 */
IMB_DLL_EXPORT void init_mb_mgr_auto(IMB_MGR *state, IMB_ARCH *arch);

/*
 * Wrapper macros to call arch API's set up
 * at init phase of multi-buffer manager.
 *
 * For example, after calling init_mb_mgr_sse(&mgr)
 * The 'mgr' structure be set up so that:
 *   mgr.get_next_job will point to get_next_job_sse(),
 *   mgr.submit_job will point to submit_job_sse(),
 *   mgr.submit_job_nocheck will point to submit_job_nocheck_sse(),
 *   mgr.get_completed_job will point to get_completed_job_sse(),
 *   mgr.flush_job will point to flush_job_sse(),
 *   mgr.queue_size will point to queue_size_sse()
 *   mgr.keyexp_128 will point to aes_keyexp_128_sse()
 *   mgr.keyexp_192 will point to aes_keyexp_192_sse()
 *   mgr.keyexp_256 will point to aes_keyexp_256_sse()
 *   etc.
 *
 * Direct use of arch API's may result in better performance.
 * Using below indirect interface may produce slightly worse performance but
 * it can simplify application implementation.
 * The test app provides example of using the indirect interface.
 */

/**
 * @brief Get next available job.
 *
 * @param [in,out] _mgr Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to next free IMB_JOB in the queue
 */
#define IMB_GET_NEXT_JOB(_mgr) ((_mgr)->get_next_job((_mgr)))

/**
 * @brief Submit job for processing after validating.
 *
 * @param [in,out] _mgr Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if no job completed
 *         If NULL, imb_get_errno() can be used to check for potential
 *         error conditions
 */
#define IMB_SUBMIT_JOB(_mgr) ((_mgr)->submit_job((_mgr)))

/**
 * @brief Submit job for processing without validating.
 *
 * This is more performant but less secure than submit_job_xxx()
 *
 * @param [in,out] _mgr Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if no job completed
 */
#define IMB_SUBMIT_JOB_NOCHECK(_mgr) ((_mgr)->submit_job_nocheck((_mgr)))

/**
 * @brief Get next completed job.
 *
 * @param [in,out] _mgr Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if next job not complete
 */
#define IMB_GET_COMPLETED_JOB(_mgr)  ((_mgr)->get_completed_job((_mgr)))

/**
 * @brief Force processing until next job in queue is completed.
 *
 * @param [in,out] _mgr Pointer to initialized IMB_MGR structure
 *
 * @return Pointer to completed IMB_JOB or NULL if no more jobs to process
 */
#define IMB_FLUSH_JOB(_mgr)          ((_mgr)->flush_job((_mgr)))

/**
 * @brief Get number of jobs queued to be processed.
 *
 * @param [in,out] _mgr Pointer to initialized IMB_MGR structure
 *
 * @return Number of jobs in the queue
 */
#define IMB_QUEUE_SIZE(_mgr)         ((_mgr)->queue_size((_mgr)))

/**
 * @brief Get next available burst
 *        (list of pointers to available IMB_JOB structures).
 *
 * @param [in,out] _mgr     Pointer to initialized IMB_MGR structure
 * @param [in] _n_jobs      Requested number of burst jobs
 * @param [out] _jobs       List of pointers to returned jobs
 *
 * @return Number of returned jobs.
 *         May be less than number of requested jobs if not enough space in
 *         queue. IMB_FLUSH_BURST() can be used to free up space.
 */
#define IMB_GET_NEXT_BURST(_mgr, _n_jobs, _jobs)   \
        ((_mgr)->get_next_burst((_mgr), (_n_jobs), (_jobs)))

/**
 * Submit multiple jobs to be processed after validating.
 *
 * @param [in,out] _mgr         Pointer to initialized IMB_MGR structure
 * @param [in] _n_jobs          Number of jobs to submit for processing
 * @param [in,out] _jobs        In:  List of pointers to jobs for submission
 *                              Out: List of pointers to completed jobs
 *
 * @return Number of completed jobs or zero on error.
 *         If zero, imb_get_errno() can be used to check for potential
 *         error conditions and _jobs[0] contains pointer to invalid job
 */
#define IMB_SUBMIT_BURST(_mgr, _n_jobs, _jobs)   \
        ((_mgr)->submit_burst((_mgr), (_n_jobs), (_jobs)))

/**
 * Submit multiple jobs to be processed without validating.
 *
 * @param [in,out] _mgr         Pointer to initialized IMB_MGR structure
 * @param [in] _n_jobs          Number of jobs to submit for processing
 * @param [in,out] _jobs        In:  List of pointers to jobs for submission
 *                              Out: List of pointers to completed jobs
 *
 * @return Number of completed jobs or zero on error
 */
#define IMB_SUBMIT_BURST_NOCHECK(_mgr, _n_jobs, _jobs)  \
        ((_mgr)->submit_burst_nocheck((_mgr), (_n_jobs), (_jobs)))

/**
 * @brief Force up to \a max_jobs outstanding jobs to completion.
 *
 * @param [in,out] _mgr         Pointer to initialized IMB_MGR structure
 * @param [in] _max_jobs        Maximum number of jobs to flush
 * @param [out] _jobs           List of pointers to completed jobs
 *
 * @return Number of completed jobs
 */
#define IMB_FLUSH_BURST(_mgr, _max_jobs, _jobs)  \
        ((_mgr)->flush_burst((_mgr), (_max_jobs), (_jobs)))

/**
 * Submit multiple cipher jobs to be processed synchronously after validating.
 *
 * @param [in] _mgr        Pointer to initialized IMB_MGR structure
 * @param [in,out] _jobs   Pointer to array of IMB_JOB structures
 * @param [in] _n_jobs     Number of jobs to process
 * @param [in] _cipher     Cipher algorithm of type #IMB_CIPHER_MODE
 * @param [in] _dir        Cipher direction of type #IMB_CIPHER_DIRECTION
 * @param [in] _key_size   Key size in bytes of type #IMB_KEY_SIZE_BYTES
 *
 * @return Number of completed jobs
 */
#define IMB_SUBMIT_CIPHER_BURST(_mgr, _jobs, _n_jobs, _cipher,          \
                                _dir, _key_size)                        \
        ((_mgr)->submit_cipher_burst((_mgr), (_jobs), (_n_jobs),        \
                                     (_cipher), (_dir), (_key_size)))
/**
 * Submit multiple cipher jobs to be processed synchronously without validating.
 *
 * This is more performant but less secure than IMB_SUBMIT_CIPHER_BURST().
 *
 * @param [in] _mgr        Pointer to initialized IMB_MGR structure
 * @param [in,out] _jobs   Pointer to array of IMB_JOB structures
 * @param [in] _n_jobs     Number of jobs to process
 * @param [in] _cipher     Cipher algorithm of type #IMB_CIPHER_MODE
 * @param [in] _dir        Cipher direction of type #IMB_CIPHER_DIRECTION
 * @param [in] _key_size   Key size in bytes of type #IMB_KEY_SIZE_BYTES
 *
 * @return Number of completed jobs
 */
#define IMB_SUBMIT_CIPHER_BURST_NOCHECK(_mgr, _jobs, _n_jobs, _cipher,  \
                                        _dir, _key_size)                \
        ((_mgr)->submit_cipher_burst_nocheck((_mgr), (_jobs), (_n_jobs),\
                                             (_cipher), (_dir), (_key_size)))
/**
 * Submit multiple hash jobs to be processed synchronously after validating.
 *
 * @param [in] _mgr        Pointer to initialized IMB_MGR structure
 * @param [in,out] _jobs   Pointer to array of IMB_JOB structures
 * @param [in] _n_jobs     Number of jobs to process
 * @param [in] _hash       Hash algorithm of type #IMB_HASH_ALG
 *
 * @return Number of completed jobs
 */
#define IMB_SUBMIT_HASH_BURST(_mgr, _jobs, _n_jobs, _hash)              \
        ((_mgr)->submit_hash_burst((_mgr), (_jobs), (_n_jobs), (_hash)))

/**
 * Submit multiple hash jobs to be processed synchronously without validating.
 *
 * This is more performant but less secure than IMB_SUBMIT_HASH_BURST().
 *
 * @param [in] _mgr        Pointer to initialized IMB_MGR structure
 * @param [in,out] _jobs   Pointer to array of IMB_JOB structures
 * @param [in] _n_jobs     Number of jobs to process
 * @param [in] _hash       Hash algorithm of type #IMB_HASH_ALG
 *
 * @return Number of completed jobs
 */
#define IMB_SUBMIT_HASH_BURST_NOCHECK(_mgr, _jobs, _n_jobs, _hash)      \
        ((_mgr)->submit_hash_burst_nocheck((_mgr), (_jobs), (_n_jobs), (_hash)))

/* Key expansion and generation API's */

/**
 * Generate encryption/decryption AES-128 expansion keys.
 *
 * @param[in] _mgr          Pointer to multi-buffer structure
 * @param[in] _key          AES-128 key
 * @param[out] _enc_exp_key AES-128 encryption expansion key
 * @param[out] _dec_exp_key AES-128 decryption expansion key
 */
#define IMB_AES_KEYEXP_128(_mgr, _key, _enc_exp_key, _dec_exp_key)      \
        ((_mgr)->keyexp_128((_key), (_enc_exp_key), (_dec_exp_key)))
/**
 * Generate encryption/decryption AES-192 expansion keys.
 *
 * @param[in] _mgr          Pointer to multi-buffer structure
 * @param[in] _key          AES-192 key
 * @param[out] _enc_exp_key AES-192 encryption expansion key
 * @param[out] _dec_exp_key AES-192 decryption expansion key
 */
#define IMB_AES_KEYEXP_192(_mgr, _key, _enc_exp_key, _dec_exp_key)      \
        ((_mgr)->keyexp_192((_key), (_enc_exp_key), (_dec_exp_key)))
/**
 * Generate encryption/decryption AES-256 expansion keys.
 *
 * @param[in] _mgr          Pointer to multi-buffer structure
 * @param[in] _key          AES-256 key
 * @param[out] _enc_exp_key AES-256 encryption expansion key
 * @param[out] _dec_exp_key AES-256 decryption expansion key
 */
#define IMB_AES_KEYEXP_256(_mgr, _key, _enc_exp_key, _dec_exp_key)      \
        ((_mgr)->keyexp_256((_key), (_enc_exp_key), (_dec_exp_key)))

/**
 * Generate AES-128-CMAC subkeys.
 *
 * @param[in] _mgr         Pointer to multi-buffer structure
 * @param[in] _exp_key     Input expanded AES-128-CMAC key
 * @param[out] _key1       Subkey 1
 * @param[out] _key2       Subkey 2
 */
#define IMB_AES_CMAC_SUBKEY_GEN_128(_mgr, _exp_key, _key1, _key2)   \
        ((_mgr)->cmac_subkey_gen_128((_exp_key), (_key1), (_key2)))

/**
 * Generate AES-256-CMAC subkeys.
 *
 * @param[in] _mgr         Pointer to multi-buffer structure
 * @param[in] _exp_key     Input expanded AES-256-CMAC key
 * @param[out] _key1       Subkey 1
 * @param[out] _key2       Subkey 2
 */
#define IMB_AES_CMAC_SUBKEY_GEN_256(_mgr, _exp_key, _key1, _key2)   \
        ((_mgr)->cmac_subkey_gen_256((_exp_key), (_key1), (_key2)))

/**
 * Generate AES-128-XCBC expansion keys.
 *
 * @param[in] _mgr         Pointer to multi-buffer structure
 * @param[in] _key         AES-128-XCBC key
 * @param[out] _exp_key    k1 expansion key
 * @param[out] _exp_key2   k2 expansion key
 * @param[out] _exp_key3   k3 expansion key
 */
#define IMB_AES_XCBC_KEYEXP(_mgr, _key, _exp_key, _exp_key2, _exp_key3) \
        ((_mgr)->xcbc_keyexp((_key), (_exp_key), (_exp_key2), (_exp_key3)))

#define IMB_DES_KEYSCHED(_mgr, _exp_key, _key)       \
        ((_mgr)->des_key_sched((_exp_key), (_key)))

/* Hash API's */

/**
 * Authenticate 64-byte data buffer with SHA1.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    64-byte data buffer
 * @param[out] _tag   Digest output (20 bytes)
 */
#define IMB_SHA1_ONE_BLOCK(_mgr, _src, _tag)        \
        ((_mgr)->sha1_one_block((_src), (_tag)))

/**
 * Authenticate variable sized data with SHA1.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    Data buffer
 * @param[in] _length Length of data in bytes for authentication.
 * @param[out] _tag   Digest output (20 bytes)
 */
#define IMB_SHA1(_mgr, _src, _length, _tag)         \
        ((_mgr)->sha1((_src), (_length), (_tag)))
/**
 * Authenticate 64-byte data buffer with SHA224.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    64-byte data buffer
 * @param[out] _tag   Digest output (28 bytes)
 */
#define IMB_SHA224_ONE_BLOCK(_mgr, _src, _tag)      \
        ((_mgr)->sha224_one_block((_src), (_tag)))

/**
 * Authenticate variable sized data with SHA224.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    Data buffer
 * @param[in] _length Length of data in bytes for authentication.
 * @param[out] _tag   Digest output (28 bytes)
 */
#define IMB_SHA224(_mgr, _src, _length, _tag)       \
        ((_mgr)->sha224((_src), (_length), (_tag)))
/**
 * Authenticate 64-byte data buffer with SHA256.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    64-byte data buffer
 * @param[out] _tag   Digest output (32 bytes)
 */
#define IMB_SHA256_ONE_BLOCK(_mgr, _src, _tag)      \
        ((_mgr)->sha256_one_block((_src), (_tag)))
/**
 * Authenticate variable sized data with SHA256.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    Data buffer
 * @param[in] _length Length of data in bytes for authentication.
 * @param[out] _tag   Digest output (32 bytes)
 */
#define IMB_SHA256(_mgr, _src, _length, _tag)       \
        ((_mgr)->sha256((_src), (_length), (_tag)))
/**
 * Authenticate 128-byte data buffer with SHA384.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    128-byte data buffer
 * @param[out] _tag   Digest output (48 bytes)
 */
#define IMB_SHA384_ONE_BLOCK(_mgr, _src, _tag)      \
        ((_mgr)->sha384_one_block((_src), (_tag)))
/**
 * Authenticate variable sized data with SHA384.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    Data buffer
 * @param[in] _length Length of data in bytes for authentication.
 * @param[out] _tag   Digest output (48 bytes)
 */
#define IMB_SHA384(_mgr, _src, _length, _tag)       \
        ((_mgr)->sha384((_src), (_length), (_tag)))
/**
 * Authenticate 128-byte data buffer with SHA512.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    128-byte data buffer
 * @param[out] _tag   Digest output (64 bytes)
 */
#define IMB_SHA512_ONE_BLOCK(_mgr, _src, _tag)      \
        ((_mgr)->sha512_one_block((_src), (_tag)))
/**
 * Authenticate variable sized data with SHA512.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    Data buffer
 * @param[in] _length Length of data in bytes for authentication.
 * @param[out] _tag   Digest output (20 bytes)
 */
#define IMB_SHA512(_mgr, _src, _length, _tag)       \
        ((_mgr)->sha512((_src), (_length), (_tag)))
/**
 * Authenticate 64-byte data buffer with MD5.
 *
 * @param[in] _mgr    Pointer to multi-buffer structure
 * @param[in] _src    64-byte data buffer
 * @param[out] _tag   Digest output (16 bytes)
 */
#define IMB_MD5_ONE_BLOCK(_mgr, _src, _tag)         \
        ((_mgr)->md5_one_block((_src), (_tag)))

/**
 * @brief AES-CFB-128 Encrypt/Decrypt up to one block.
 *
 * Processes only one buffer at a time.
 * Designed to manage partial blocks of DOCSIS 3.1 SEC BPI.
 *
 * @param [in] _mgr     Pointer to multi-buffer structure
 * @param [out] _dst    Plaintext/Ciphertext output
 * @param [in] _src     Plaintext/Ciphertext input
 * @param [in] _iv      Pointer to 16 byte IV
 * @param [in] _exp_key Pointer to expanded AES keys
 * @param [in] _len     Length of data in bytes
 */
#define IMB_AES128_CFB_ONE(_mgr, _dst, _src, _iv, _exp_key, _len)       \
        ((_mgr)->aes128_cfb_one((_dst), (_src), (_iv), (_exp_key), (_len)))

/* AES-GCM API's */
#define IMB_AES128_GCM_ENC(_mgr, _exp_key, _ctx, _dst, _src, _len, _iv, _aad,  \
                           _aadl, _tag, _tagl)                                 \
        ((_mgr)->gcm128_enc((_exp_key), (_ctx), (_dst), (_src), (_len), (_iv), \
                            (_aad), (_aadl), (_tag), (_tagl)))
#define IMB_AES192_GCM_ENC(_mgr, _exp_key, _ctx, _dst, _src, _len, _iv, _aad,  \
                           _aadl, _tag, _tagl)                                 \
        ((_mgr)->gcm192_enc((_exp_key), (_ctx), (_dst), (_src), (_len), (_iv), \
                            (_aad), (_aadl), (_tag), (_tagl)))
#define IMB_AES256_GCM_ENC(_mgr, _exp_key, _ctx, _dst, _src, _len, _iv, _aad,  \
                           _aadl, _tag, _tagl)                                 \
        ((_mgr)->gcm256_enc((_exp_key), (_ctx), (_dst), (_src), (_len), (_iv), \
                            (_aad), (_aadl), (_tag), (_tagl)))

#define IMB_AES128_GCM_DEC(_mgr, _exp_key, _ctx, _dst, _src, _len, _iv, _aad,  \
                           _aadl, _tag, _tagl)                                 \
        ((_mgr)->gcm128_dec((_exp_key), (_ctx), (_dst), (_src), (_len), (_iv), \
                            (_aad), (_aadl), (_tag), (_tagl)))
#define IMB_AES192_GCM_DEC(_mgr, _exp_key, _ctx, _dst, _src, _len, _iv, \
                           _aad, _aadl, _tag, _tagl)                    \
        ((_mgr)->gcm192_dec((_exp_key), (_ctx), (_dst), (_src), (_len), \
                            (_iv), (_aad), (_aadl), (_tag), (_tagl)))
#define IMB_AES256_GCM_DEC(_mgr, _exp_key, _ctx, _dst, _src, _len, _iv, \
                           _aad, _aadl, _tag, _tagl)                    \
        ((_mgr)->gcm256_dec((_exp_key), (_ctx), (_dst), (_src), (_len), \
                            (_iv), (_aad), (_aadl), (_tag), (_tagl)))

#define IMB_AES128_GCM_INIT(_mgr, _exp_key, _ctx, _iv, _aad, _aadl)       \
        ((_mgr)->gcm128_init((_exp_key), (_ctx), (_iv), (_aad), (_aadl)))
#define IMB_AES192_GCM_INIT(_mgr, _exp_key, _ctx, _iv, _aad, _aadl)       \
        ((_mgr)->gcm192_init((_exp_key), (_ctx), (_iv), (_aad), (_aadl)))
#define IMB_AES256_GCM_INIT(_mgr, _exp_key, _ctx, _iv, _aad, _aadl)       \
        ((_mgr)->gcm256_init((_exp_key), (_ctx), (_iv), (_aad), (_aadl)))

#define IMB_AES128_GCM_INIT_VAR_IV(_mgr, _exp_key, _ctx, _iv, _ivl, _aad, \
                                   _aadl)                                 \
        ((_mgr)->gcm128_init_var_iv((_exp_key), (_ctx), (_iv), (_ivl),    \
                                    (_aad), (_aadl)))
#define IMB_AES192_GCM_INIT_VAR_IV(_mgr, _exp_key, _ctx, _iv, _ivl, _aad, \
                                   _aadl)                                 \
        ((_mgr)->gcm192_init_var_iv((_exp_key), (_ctx), (_iv), (_ivl),    \
                                    (_aad), (_aadl)))
#define IMB_AES256_GCM_INIT_VAR_IV(_mgr, _exp_key, _ctx, _iv, _ivl, _aad, \
                                    _aadl)                                \
        ((_mgr)->gcm256_init_var_iv((_exp_key), (_ctx), (_iv), (_ivl),    \
                                    (_aad), (_aadl)))

#define IMB_AES128_GCM_ENC_UPDATE(_mgr, _exp_key, _ctx, _dst, _src, _len)    \
        ((_mgr)->gcm128_enc_update((_exp_key), (_ctx), (_dst), (_src), (_len)))
#define IMB_AES192_GCM_ENC_UPDATE(_mgr, _exp_key, _ctx, _dst, _src, _len)    \
        ((_mgr)->gcm192_enc_update((_exp_key), (_ctx), (_dst), (_src), (_len)))
#define IMB_AES256_GCM_ENC_UPDATE(_mgr, _exp_key, _ctx, _dst, _src, _len)    \
        ((_mgr)->gcm256_enc_update((_exp_key), (_ctx), (_dst), (_src), (_len)))

#define IMB_AES128_GCM_DEC_UPDATE(_mgr, _exp_key, _ctx, _dst, _src, _len)    \
        ((_mgr)->gcm128_dec_update((_exp_key), (_ctx), (_dst), (_src), (_len)))
#define IMB_AES192_GCM_DEC_UPDATE(_mgr, _exp_key, _ctx, _dst, _src, _len)    \
        ((_mgr)->gcm192_dec_update((_exp_key), (_ctx), (_dst), (_src), (_len)))
#define IMB_AES256_GCM_DEC_UPDATE(_mgr, _exp_key, _ctx, _dst, _src, _len)    \
        ((_mgr)->gcm256_dec_update((_exp_key), (_ctx), (_dst), (_src), (_len)))

#define IMB_AES128_GCM_ENC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gcm128_enc_finalize((_exp_key), (_ctx), (_tag), (_tagl)))
#define IMB_AES192_GCM_ENC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gcm192_enc_finalize((_exp_key), (_ctx), (_tag), (_tagl)))
#define IMB_AES256_GCM_ENC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gcm256_enc_finalize((_exp_key), (_ctx), (_tag), (_tagl)))

#define IMB_AES128_GCM_DEC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gcm128_dec_finalize((_exp_key), (_ctx), (_tag), (_tagl)))
#define IMB_AES192_GCM_DEC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gcm192_dec_finalize((_exp_key), (_ctx), (_tag), (_tagl)))
#define IMB_AES256_GCM_DEC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gcm256_dec_finalize((_exp_key), (_ctx), (_tag), (_tagl)))

#define IMB_AES128_GMAC_INIT(_mgr, _exp_key, _ctx, _iv, _ivl) \
        ((_mgr)->gmac128_init((_exp_key), (_ctx), (_iv), (_ivl)))
#define IMB_AES192_GMAC_INIT(_mgr, _exp_key, _ctx, _iv, _ivl) \
        ((_mgr)->gmac192_init((_exp_key), (_ctx), (_iv), (_ivl)))
#define IMB_AES256_GMAC_INIT(_mgr, _exp_key, _ctx, _iv, _ivl) \
        ((_mgr)->gmac256_init((_exp_key), (_ctx), (_iv), (_ivl)))

#define IMB_AES128_GMAC_UPDATE(_mgr, _exp_key, _ctx, _src, _len) \
        ((_mgr)->gmac128_update((_exp_key), (_ctx), (_src), (_len)))
#define IMB_AES192_GMAC_UPDATE(_mgr, _exp_key, _ctx, _src, _len) \
        ((_mgr)->gmac192_update((_exp_key), (_ctx), (_src), (_len)))
#define IMB_AES256_GMAC_UPDATE(_mgr, _exp_key, _ctx, _src, _len) \
        ((_mgr)->gmac256_update((_exp_key), (_ctx), (_src), (_len)))

#define IMB_AES128_GMAC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gmac128_finalize((_exp_key), (_ctx), (_tag), (_tagl)))
#define IMB_AES192_GMAC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gmac192_finalize((_exp_key), (_ctx), (_tag), (_tagl)))
#define IMB_AES256_GMAC_FINALIZE(_mgr, _exp_key, _ctx, _tag, _tagl)      \
        ((_mgr)->gmac256_finalize((_exp_key), (_ctx), (_tag), (_tagl)))

#define IMB_AES128_GCM_PRECOMP(_mgr, _key) \
        ((_mgr)->gcm128_precomp((_key)))
#define IMB_AES192_GCM_PRECOMP(_mgr, _key) \
        ((_mgr)->gcm192_precomp((_key)))
#define IMB_AES256_GCM_PRECOMP(_mgr, _key) \
        ((_mgr)->gcm256_precomp((_key)))

#define IMB_AES128_GCM_PRE(_mgr, _key, _exp_key)     \
        ((_mgr)->gcm128_pre((_key), (_exp_key)))
#define IMB_AES192_GCM_PRE(_mgr, _key, _exp_key)     \
        ((_mgr)->gcm192_pre((_key), (_exp_key)))
#define IMB_AES256_GCM_PRE(_mgr, _key, _exp_key)     \
        ((_mgr)->gcm256_pre((_key), (_exp_key)))

#define IMB_GHASH_PRE(_mgr, _key, _exp_key)          \
        ((_mgr)->ghash_pre((_key), (_exp_key)))
#define IMB_GHASH(_mgr, _exp_key, _src, _len, _tag, _tagl) \
        ((_mgr)->ghash((_exp_key), (_src), (_len), (_tag), (_tagl)))

/* Chacha20-Poly1305 direct API's */
#define IMB_CHACHA20_POLY1305_INIT(_mgr, _key, _ctx, _iv, _aad, _aadl)        \
        ((_mgr)->chacha20_poly1305_init((_key), (_ctx), (_iv), (_aad),        \
                                        (_aadl)))

#define IMB_CHACHA20_POLY1305_ENC_UPDATE(_mgr, _key, _ctx, _dst, _src, _len)  \
        ((_mgr)->chacha20_poly1305_enc_update((_key), (_ctx), (_dst), (_src), \
                                              (_len)))
#define IMB_CHACHA20_POLY1305_DEC_UPDATE(_mgr, _key, _ctx, _dst, _src, _len)  \
        ((_mgr)->chacha20_poly1305_dec_update((_key), (_ctx), (_dst), (_src), \
                                              (_len)))

#define IMB_CHACHA20_POLY1305_ENC_FINALIZE(_mgr, _ctx, _tag, _tagl)           \
        ((_mgr)->chacha20_poly1305_finalize((_ctx), (_tag), (_tagl)))

#define IMB_CHACHA20_POLY1305_DEC_FINALIZE(_mgr, _ctx, _tag, _tagl)           \
        ((_mgr)->chacha20_poly1305_finalize((_ctx), (_tag), (_tagl)))

/* ZUC EEA3/EIA3 functions */

/**
 * @brief ZUC EEA3 Confidentiality functions
 *
 * @param _mgr   Pointer to multi-buffer structure
 * @param _key   Pointer to key
 * @param _iv    Pointer to 16-byte IV
 * @param _src   Pointer to Plaintext/Ciphertext input.
 * @param _dst   Pointer to Ciphertext/Plaintext output.
 * @param _len   Length of input data in bytes.
 */
#define IMB_ZUC_EEA3_1_BUFFER(_mgr, _key, _iv, _src, _dst, _len)         \
        ((_mgr)->eea3_1_buffer((_key), (_iv), (_src), (_dst), (_len)))
#define IMB_ZUC_EEA3_4_BUFFER(_mgr, _key, _iv, _src, _dst, _len)         \
        ((_mgr)->eea3_4_buffer((_key), (_iv), (_src), (_dst), (_len)))
#define IMB_ZUC_EEA3_N_BUFFER(_mgr, _key, _iv, _src, _dst, _len, _count) \
        ((_mgr)->eea3_n_buffer((_key), (_iv), (_src), (_dst), (_len), (_count)))


/**
 * @brief ZUC EIA3 Integrity function
 *
 * @param _mgr   Pointer to multi-buffer structure
 * @param _key   Pointer to key
 * @param _iv    Pointer to 16-byte IV
 * @param _src   Pointer to Plaintext/Ciphertext input.
 * @param _len   Length of input data in bits.
 * @param _tag   Pointer to Authenticated Tag output (4 bytes)
 */
#define IMB_ZUC_EIA3_1_BUFFER(_mgr, _key, _iv, _src, _len, _tag)         \
        ((_mgr)->eia3_1_buffer((_key), (_iv), (_src), (_len), (_tag)))
#define IMB_ZUC_EIA3_N_BUFFER(_mgr, _key, _iv, _src, _len, _tag, _count) \
        ((_mgr)->eia3_n_buffer((_key), (_iv), (_src), (_len), (_tag), (_count)))


/* KASUMI F8/F9 functions */

/**
 * @brief Kasumi byte-level f8 operation on a single buffer
 *
 * This function performs kasumi f8 operation on a single buffer. The key has
 * already been scheduled with kasumi_init_f8_key_sched().
 * No extra bits are modified.
 *
 * @param [in]  _mgr      Pointer to multi-buffer structure
 * @param [in]  _exp_key  Context where the scheduled keys are stored
 * @param [in]  _iv       Initialization vector
 * @param [in]  _src      Input buffer
 * @param [out] _dst      Output buffer
 * @param [in]  _len      Length in BYTES
 *
 ******************************************************************************/
#define IMB_KASUMI_F8_1_BUFFER(_mgr, _exp_key, _iv, _src, _dst, _len) \
        ((_mgr)->f8_1_buffer((_exp_key), (_iv), (_src), (_dst), (_len)))

/**
 * @brief Kasumi bit-level f8 operation on a single buffer
 *
 * This function performs kasumi f8 operation on a single buffer. The key has
 * already been scheduled with kasumi_init_f8_key_sched().
 * No extra bits are modified.
 *
 * @param [in]  _mgr      Pointer to multi-buffer structure
 * @param [in]  _exp_key  Context where the scheduled keys are stored
 * @param [in]  _iv       Initialization vector
 * @param [in]  _src      Input buffer
 * @param [out] _dst      Output buffer
 * @param [in]  _len      Length in BITS
 * @param [in]  _offset   Offset in BITS from begin of input buffer
 *
 ******************************************************************************/
#define IMB_KASUMI_F8_1_BUFFER_BIT(_mgr, _exp_key, _iv, _src, _dst, _len,  \
                                   _offset)                                 \
        ((_mgr)->f8_1_buffer_bit((_exp_key), (_iv), (_src), (_dst), (_len), \
                                 (_offset)))

/**
 * @brief Kasumi byte-level f8 operation in parallel on two buffers
 *
 * This function performs kasumi f8 operation on a two buffers.
 * They will be processed with the same key, which has already been scheduled
 * with kasumi_init_f8_key_sched().
 *
 * @param [in]  _mgr      Pointer to multi-buffer structure
 * @param [in]  _exp_key  Context where the scheduled keys are stored
 * @param [in]  _iv1      Initialization vector for buffer in1
 * @param [in]  _iv2      Initialization vector for buffer in2
 * @param [in]  _src1     Input buffer 1
 * @param [out] _dst1     Output buffer 1
 * @param [in]  _len1     Length in BYTES of input buffer 1
 * @param [in]  _src2     Input buffer 2
 * @param [out] _dst2     Output buffer 2
 * @param [in]  _len2     Length in BYTES of input buffer 2
 *
 ******************************************************************************/
#define IMB_KASUMI_F8_2_BUFFER(_mgr, _exp_key, _iv1, _iv2, _src1, _dst1,   \
                               _len1, _src2, _dst2, _len2)                 \
        ((_mgr)->f8_2_buffer((_exp_key), (_iv1), (_iv2), (_src1), (_dst1), \
                             (_len1), (_src2), (_dst2), (_len2)))
/**
 * @brief kasumi byte-level f8 operation in parallel on three buffers
 *
 * This function performs kasumi f8 operation on a three buffers.
 * They must all have the same length and they will be processed with the same
 * key, which has already been scheduled with kasumi_init_f8_key_sched().
 *
 * @param [in]  _mgr      Pointer to multi-buffer structure
 * @param [in]  _exp_key  Context where the scheduled keys are stored
 * @param [in]  _iv1      Initialization vector for buffer in1
 * @param [in]  _iv2      Initialization vector for buffer in2
 * @param [in]  _iv3      Initialization vector for buffer in3
 * @param [in]  _src1     Input buffer 1
 * @param [out] _dst1     Output buffer 1
 * @param [in]  _src2     Input buffer 2
 * @param [out] _dst2     Output buffer 2
 * @param [in]  _src3     Input buffer 3
 * @param [out] _dst3     Output buffer 3
 * @param [in]  _len      Common length in bytes for all buffers
 *
 ******************************************************************************/
#define IMB_KASUMI_F8_3_BUFFER(_mgr, _exp_key, _iv1, _iv2, _iv3, _src1, _dst1, \
                               _src2, _dst2, _src3, _dst3, _len)               \
        ((_mgr)->f8_3_buffer((_exp_key), (_iv1), (_iv2), (_iv3), (_src1),      \
                             (_dst1), (_src2), (_dst2), (_src3), (_dst3),      \
                             (_len)))
/**
 * @brief kasumi byte-level f8 operation in parallel on four buffers
 *
 * This function performs kasumi f8 operation on four buffers.
 * They must all have the same length and they will be processed with the same
 * key, which has already been scheduled with kasumi_init_f8_key_sched().
 *
 * @param [in]  _mgr      Pointer to multi-buffer structure
 * @param [in]  _exp_key  Context where the scheduled keys are stored
 * @param [in]  _iv1      Initialization vector for buffer in1
 * @param [in]  _iv2      Initialization vector for buffer in2
 * @param [in]  _iv3      Initialization vector for buffer in3
 * @param [in]  _iv4      Initialization vector for buffer in4
 * @param [in]  _src1     Input buffer 1
 * @param [out] _dst1     Output buffer 1
 * @param [in]  _src2     Input buffer 2
 * @param [out] _dst2     Output buffer 2
 * @param [in]  _src3     Input buffer 3
 * @param [out] _dst3     Output buffer 3
 * @param [in]  _src4     Input buffer 4
 * @param [out] _dst4     Output buffer 4
 * @param [in]  _len      Common length in bytes for all buffers
 *
 ******************************************************************************/
#define IMB_KASUMI_F8_4_BUFFER(_mgr, _exp_key, _iv1, _iv2, _iv3, _iv4,   \
                               _src1, _dst1, _src2, _dst2, _src3, _dst3, \
                               _src4, _dst4, _len)                       \
        ((_mgr)->f8_4_buffer((_exp_key), (_iv1), (_iv2), (_iv3), (_iv4), \
                             (_src1), (_dst1), (_src2), (_dst2),         \
                             (_src3), (_dst3), (_src4), (_dst4), (_len)))
/**
 * @brief Kasumi f8 operation on N buffers
 *
 * All input buffers can have different lengths and they will be processed
 * with the same key, which has already been scheduled
 * with kasumi_init_f8_key_sched().
 *
 * @param [in]  _mgr     Pointer to multi-buffer structure
 * @param [in]  _exp_key Context where the scheduled keys are stored
 * @param [in]  _iv      Array of IV values
 * @param [in]  _src     Array of input buffers
 * @param [out] _dst     Array of output buffers
 * @param [in]  _len     Array of corresponding input buffer lengths in BITS
 * @param [in]  _count   Number of input buffers
 */
#define IMB_KASUMI_F8_N_BUFFER(_mgr, _exp_key, _iv, _src, _dst, _len, _count) \
        ((_mgr)->f8_n_buffer((_exp_key), (_iv), (_src), (_dst), (_len),       \
                             (_count)))
/**
 * @brief Kasumi bit-level f9 operation on a single buffer.
 *
 * The first QWORD of in represents the COUNT and FRESH, the last QWORD
 * represents the DIRECTION and PADDING. (See 3GPP TS 35.201 v10.0 section 4)
 *
 * The key has already been scheduled with kasumi_init_f9_key_sched().
 *
 * @param [in]  _mgr     Pointer to multi-buffer structure
 * @param [in]  _exp_key Context where the scheduled keys are stored
 * @param [in]  _src     Input buffer
 * @param [in]  _len     Length in BYTES of the data to be hashed
 * @param [out] _tag     Computed digest
 *
 */
#define IMB_KASUMI_F9_1_BUFFER(_mgr, _exp_key,  _src, _len, _tag) \
        ((_mgr)->f9_1_buffer((_exp_key), (_src), (_len), (_tag)))

/**
 * @brief Kasumi bit-level f9 operation on a single buffer.
 *
 * The key has already been scheduled with kasumi_init_f9_key_sched().
 *
 * @param [in]  _mgr     Pointer to multi-buffer structure
 * @param [in]  _exp_key Context where the scheduled keys are stored
 * @param [in]  _iv      Initialization vector
 * @param [in]  _src     Input buffer
 * @param [in]  _len     Length in BITS of the data to be hashed
 * @param [out] _tag     Computed digest
 * @param [in]  _dir     Direction bit
 *
 */
#define IMB_KASUMI_F9_1_BUFFER_USER(_mgr, _exp_key, _iv, _src, _len, _tag,    \
                                    _dir)                                     \
        ((_mgr)->f9_1_buffer_user((_exp_key), (_iv), (_src), (_len),          \
                                  (_tag), (_dir)))

/**
 * KASUMI F8 key schedule init function.
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @param[in]  _key      Confidentiality key (expected in LE format)
 * @param[out] _exp_key  Key schedule context to be initialised
 * @return 0 on success, -1 on failure
 *
 ******************************************************************************/
#define IMB_KASUMI_INIT_F8_KEY_SCHED(_mgr, _key, _exp_key)     \
        ((_mgr)->kasumi_init_f8_key_sched((_key), (_exp_key)))

/**
 * KASUMI F9 key schedule init function.
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @param[in]  _key      Integrity key (expected in LE format)
 * @param[out] _exp_key  Key schedule context to be initialised
 * @return 0 on success, -1 on failure
 *
 ******************************************************************************/
#define IMB_KASUMI_INIT_F9_KEY_SCHED(_mgr, _key, _exp_key)     \
        ((_mgr)->kasumi_init_f9_key_sched((_key), (_exp_key)))

/**
 *******************************************************************************
 * This function returns the size of the kasumi_key_sched_t, used
 * to store the key schedule.
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @return size of kasumi_key_sched_t type success
 *
 ******************************************************************************/
#define IMB_KASUMI_KEY_SCHED_SIZE(_mgr)((_mgr)->kasumi_key_sched_size())


/* SNOW3G F8/F9 functions */

/**
 * This function performs snow3g f8 operation on a single buffer. The key has
 * already been scheduled with snow3g_init_key_sched().
 *
 * @param[in]  _mgr          Pointer to multi-buffer structure
 * @param[in]  _exp_key      Context where the scheduled keys are stored
 * @param[in]  _iv           iv[3] = count
 *                           iv[2] = (bearer << 27) | ((dir & 0x1) << 26)
 *                           iv[1] = pIV[3]
 *                           iv[0] = pIV[2]
 * @param[in]  _src          Input buffer
 * @param[out] _dst          Output buffer
 * @param[in]  _len          Length in bits of input buffer
 * @param[in]  _offset       Offset in input/output buffer (in bits)
 */
#define IMB_SNOW3G_F8_1_BUFFER_BIT(_mgr, _exp_key, _iv, _src, _dst,     \
                                   _len, _offset)                       \
        ((_mgr)->snow3g_f8_1_buffer_bit((_exp_key), (_iv), (_src),      \
                                        (_dst), (_len), (_offset)))

/**
 * This function performs snow3g f8 operation on a single buffer. The key has
 * already been scheduled with snow3g_init_key_sched().
 *
 * @param[in]  _mgr          Pointer to multi-buffer structure
 * @param[in]  _exp_key      Context where the scheduled keys are stored
 * @param[in]  _iv           iv[3] = count
 *                           iv[2] = (bearer << 27) | ((dir & 0x1) << 26)
 *                           iv[1] = pIV[3]
 *                           iv[0] = pIV[2]
 * @param[in]  _src          Input buffer
 * @param[out] _dst          Output buffer
 * @param[in]  _len          Length in bits of input buffer
 */
#define IMB_SNOW3G_F8_1_BUFFER(_mgr, _exp_key, _iv, _src, _dst, _len)        \
        ((_mgr)->snow3g_f8_1_buffer((_exp_key), (_iv), (_src), (_dst), (_len)))

/**
 * This function performs snow3g f8 operation on two buffers. They will
 * be processed with the same key, which has already been scheduled with
 * snow3g_init_key_sched().
 *
 * @param[in]  _mgr           Pointer to multi-buffer structure
 * @param[in]  _exp_key       Context where the scheduled keys are stored
 * @param[in]  _iv1           IV to use for buffer pBufferIn1
 * @param[in]  _iv2           IV to use for buffer pBufferIn2
 * @param[in]  _src1          Input buffer 1
 * @param[out] _dst1          Output buffer 1
 * @param[in]  _len1          Length in bytes of input buffer 1
 * @param[in]  _src2          Input buffer 2
 * @param[out] _dst2          Output buffer 2
 * @param[in]  _len2          Length in bytes of input buffer 2
 */
#define IMB_SNOW3G_F8_2_BUFFER(_mgr, _exp_key, _iv1, _iv2,              \
                               _src1, _dst1, _len1,                     \
                               _src2, _dst2, _len2)                     \
        ((_mgr)->snow3g_f8_2_buffer((_exp_key), (_iv1), (_iv2),         \
                                    (_src1), (_dst1), (_len1),          \
                                    (_src2), (_dst2), (_len2)))

/**
 *******************************************************************************
 * This function performs snow3g f8 operation on four buffers. They will
 * be processed with the same key, which has already been scheduled with
 * snow3g_init_key_sched().
 *
 * @param[in]  _mgr           Pointer to multi-buffer structure
 * @param[in]  _exp_key       Context where the scheduled keys are stored
 * @param[in]  _iv1           IV to use for buffer pBufferIn1
 * @param[in]  _iv2           IV to use for buffer pBufferIn2
 * @param[in]  _iv3           IV to use for buffer pBufferIn3
 * @param[in]  _iv4           IV to use for buffer pBufferIn4
 * @param[in]  _src1          Input buffer 1
 * @param[out] _dst1          Output buffer 1
 * @param[in]  _len1          Length in bytes of input buffer 1
 * @param[in]  _src2          Input buffer 2
 * @param[out] _dst2          Output buffer 2
 * @param[in]  _len2          Length in bytes of input buffer 2
 * @param[in]  _src3          Input buffer 3
 * @param[out] _dst3          Output buffer 3
 * @param[in]  _len3          Length in bytes of input buffer 3
 * @param[in]  _src4          Input buffer 4
 * @param[out] _dst4          Output buffer 4
 * @param[in]  _len4          Length in bytes of input buffer 4
 */
#define IMB_SNOW3G_F8_4_BUFFER(_mgr, _exp_key, _iv1, _iv2, _iv3, _iv4,   \
                               _src1, _dst1, _len1,                      \
                               _src2, _dst2, _len2,                      \
                               _src3, _dst3, _len3,                      \
                               _src4, _dst4, _len4)                      \
        ((_mgr)->snow3g_f8_4_buffer((_exp_key), (_iv1), (_iv2), (_iv3),  \
                                    (_iv4), (_src1), (_dst1), (_len1),   \
                                    (_src2), (_dst2), (_len2),           \
                                    (_src3), (_dst3), (_len3),           \
                                    (_src4), (_dst4), (_len4)))

/**
 *******************************************************************************
 * This function performs snow3g f8 operation on eight buffers. They will
 * be processed with the same key, which has already been scheduled with
 * snow3g_init_key_sched().
 *
 * @param[in]  _mgr           Pointer to multi-buffer structure
 * @param[in]  _exp_key       Context where the scheduled keys are stored
 * @param[in]  _iv1           IV to use for buffer pBufferIn1
 * @param[in]  _iv2           IV to use for buffer pBufferIn2
 * @param[in]  _iv3           IV to use for buffer pBufferIn3
 * @param[in]  _iv4           IV to use for buffer pBufferIn4
 * @param[in]  _iv5           IV to use for buffer pBufferIn5
 * @param[in]  _iv6           IV to use for buffer pBufferIn6
 * @param[in]  _iv7           IV to use for buffer pBufferIn7
 * @param[in]  _iv8           IV to use for buffer pBufferIn8
 * @param[in]  _src1          Input buffer 1
 * @param[out] _dst1          Output buffer 1
 * @param[in]  _len1          Length in bytes of input buffer 1
 * @param[in]  _src2          Input buffer 2
 * @param[out] _dst2          Output buffer 2
 * @param[in]  _len2          Length in bytes of input buffer 2
 * @param[in]  _src3          Input buffer 3
 * @param[out] _dst3          Output buffer 3
 * @param[in]  _len3          Length in bytes of input buffer 3
 * @param[in]  _src4          Input buffer 4
 * @param[out] _dst4          Output buffer 4
 * @param[in]  _len4          Length in bytes of input buffer 4
 * @param[in]  _src5          Input buffer 5
 * @param[out] _dst5          Output buffer 5
 * @param[in]  _len5          Length in bytes of input buffer 5
 * @param[in]  _src6          Input buffer 6
 * @param[out] _dst6          Output buffer 6
 * @param[in]  _len6          Length in bytes of input buffer 6
 * @param[in]  _src7          Input buffer 7
 * @param[out] _dst7          Output buffer 7
 * @param[in]  _len7          Length in bytes of input buffer 7
 * @param[in]  _src8          Input buffer 8
 * @param[out] _dst8          Output buffer 8
 * @param[in]  _len8          Length in bytes of input buffer 8
 */
#define IMB_SNOW3G_F8_8_BUFFER(_mgr, _exp_key, _iv1, _iv2, _iv3, _iv4,   \
                               _iv5, _iv6, _iv7, _iv8,                   \
                               _src1, _dst1, _len1,                      \
                               _src2, _dst2, _len2,                      \
                               _src3, _dst3, _len3,                      \
                               _src4, _dst4, _len4,                      \
                               _src5, _dst5, _len5,                      \
                               _src6, _dst6, _len6,                      \
                               _src7, _dst7, _len7,                      \
                               _src8, _dst8, _len8)                      \
        ((_mgr)->snow3g_f8_8_buffer((_exp_key), (_iv1), (_iv2), (_iv3),  \
                                    (_iv4), (_iv5), (_iv6), (_iv7),      \
                                    (_iv8), (_src1), (_dst1), (_len1),   \
                                    (_src2), (_dst2), (_len2),           \
                                    (_src3), (_dst3), (_len3),           \
                                    (_src4), (_dst4), (_len4),           \
                                    (_src5), (_dst5), (_len5),           \
                                    (_src6), (_dst6), (_len6),           \
                                    (_src7), (_dst7), (_len7),           \
                                    (_src8), (_dst8), (_len8)))
/**
 * This function performs snow3g f8 operation on eight buffers. They will
 * be processed with individual keys, which have already been scheduled
 * with snow3g_init_key_sched().
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @param[in]  _exp_key  Array of 8 Contexts, where the scheduled keys
 * are stored
 * @param[in]  _iv       Array of 8 IV values
 * @param[in]  _src      Array of 8 input buffers
 * @param[out] _dst      Array of 8 output buffers
 * @param[in]  _len      Array of 8 corresponding input buffer lengths
 */
#define IMB_SNOW3G_F8_8_BUFFER_MULTIKEY(_mgr, _exp_key, _iv, _src, _dst, _len) \
        ((_mgr)->snow3g_f8_8_buffer_multikey((_exp_key), (_iv), (_src), (_dst),\
                                             (_len)))

/**
 * This function performs snow3g f8 operation in parallel on N buffers. All
 * input buffers can have different lengths and they will be processed with the
 * same key, which has already been scheduled with snow3g_init_key_sched().
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @param[in]  _exp_key  Context where the scheduled keys are stored
 * @param[in]  _iv       Array of IV values
 * @param[in]  _src      Array of input buffers
 * @param[out] _dst      Array of output buffers - out[0] set to NULL on failure
 * @param[in]  _len      Array of corresponding input buffer lengths
 * @param[in]  _count    Number of input buffers
 *
 ******************************************************************************/
#define IMB_SNOW3G_F8_N_BUFFER(_mgr, _exp_key, _iv, _src, _dst, _len, _count) \
        ((_mgr)->snow3g_f8_n_buffer((_exp_key), (_iv), (_src), \
                                    (_dst), (_len), (_count)))

/**
 * This function performs snow3g f8 operation in parallel on N buffers. All
 * input buffers can have different lengths. Confidentiallity keys can vary,
 * schedules with snow3g_init_key_sched_multi().
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @param[in]  _exp_key  Array of Contexts, where the scheduled keys are stored
 * @param[in]  _iv       Array of IV values
 * @param[in]  _src      Array of input buffers
 * @param[out] _dst      Array of output buffers
 *                       - out[0] set to NULL on failure
 * @param[in]  _len      Array of corresponding input buffer lengths
 * @param[in]  _count    Number of input buffers
 */
#define IMB_SNOW3G_F8_N_BUFFER_MULTIKEY(_mgr, _exp_key, _iv, _src,           \
                                        _dst, _len, _count)             \
        ((_mgr)->snow3g_f8_n_buffer_multikey((_exp_key), (_iv), (_src),      \
                                             (_dst), (_len), (_count)))

/**
 * This function performs a snow3g f9 operation on a single block of data. The
 * key has already been scheduled with snow3g_init_f8_key_sched().
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @param[in]  _exp_key  Context where the scheduled keys are stored
 * @param[in]  _iv       iv[3] = _BSWAP32(fresh^(dir<<15))
 *                       iv[2] = _BSWAP32(count^(dir<<31))
 *                       iv[1] = _BSWAP32(fresh)
 *                       iv[0] = _BSWAP32(count)
 *
 * @param[in]  _src      Input buffer
 * @param[in]  _len      Length in bits of the data to be hashed
 * @param[out] _tag      Computed digest
 */
#define IMB_SNOW3G_F9_1_BUFFER(_mgr, _exp_key, _iv, _src, _len, _tag)     \
        ((_mgr)->snow3g_f9_1_buffer((_exp_key), (_iv), (_src), (_len), (_tag)))

/**
 * Snow3g key schedule init function.
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @param[in]  _key      Confidentiality/Integrity key (expected in LE format)
 * @param[out] _exp_key  Key schedule context to be initialised
 * @return 0 on success
 * @return -1 on error
 *
 ******************************************************************************/
#define IMB_SNOW3G_INIT_KEY_SCHED(_mgr, _key, _exp_key)     \
        ((_mgr)->snow3g_init_key_sched((_key), (_exp_key)))

/**
 *******************************************************************************
 * This function returns the size of the snow3g_key_schedule_t, used
 * to store the key schedule.
 *
 * @param[in]  _mgr      Pointer to multi-buffer structure
 * @return size of snow3g_key_schedule_t type
 *
 ******************************************************************************/
#define IMB_SNOW3G_KEY_SCHED_SIZE(_mgr)((_mgr)->snow3g_key_sched_size())

/**
 *  HEC compute functions
 */
#define IMB_HEC_32(_mgr, _src)((_mgr)->hec_32(_src))
#define IMB_HEC_64(_mgr, _src)((_mgr)->hec_64(_src))

/**
 * CRC32 Ethernet FCS function
 */
#define IMB_CRC32_ETHERNET_FCS(_mgr, _src, _len) \
        (_mgr)->crc32_ethernet_fcs(_src, _len)

/**
 *  CRC16 X25 function
 */
#define IMB_CRC16_X25(_mgr, _src, _len) \
        (_mgr)->crc16_x25(_src, _len)

/**
 *  CRC32 SCTP function
 */
#define IMB_CRC32_SCTP(_mgr, _src, _len) \
        (_mgr)->crc32_sctp(_src, _len)

/**
 *  LTE CRC24A function
 */
#define IMB_CRC24_LTE_A(_mgr, _src, _len) \
        (_mgr)->crc24_lte_a(_src, _len)

/**
 *  LTE CRC24B function
 */
#define IMB_CRC24_LTE_B(_mgr, _src, _len) \
        (_mgr)->crc24_lte_b(_src, _len)

/**
 *  Framing Protocol CRC16 function (3GPP TS 25.435, 3GPP TS 25.427)
 */
#define IMB_CRC16_FP_DATA(_mgr, _src, _len) \
        (_mgr)->crc16_fp_data(_src, _len)

/**
 *  Framing Protocol CRC11 function (3GPP TS 25.435, 3GPP TS 25.427)
 */
#define IMB_CRC11_FP_HEADER(_mgr, _src, _len) \
        (_mgr)->crc11_fp_header(_src, _len)

/**
 * Framing Protocol CRC7 function (3GPP TS 25.435, 3GPP TS 25.427)
 */
#define IMB_CRC7_FP_HEADER(_mgr, _src, _len) \
        (_mgr)->crc7_fp_header(_src, _len)

/**
 *  IUUP CRC10 function (3GPP TS 25.415)
 */
#define IMB_CRC10_IUUP_DATA(_mgr, _src, _len) \
        (_mgr)->crc10_iuup_data(_src, _len)

/**
 *  IUUP CRC6 function (3GPP TS 25.415)
 */
#define IMB_CRC6_IUUP_HEADER(_mgr, _src, _len) \
        (_mgr)->crc6_iuup_header(_src, _len)

/**
 *  WIMAX OFDMA DATA CRC32 function (IEEE 802.16)
 */
#define IMB_CRC32_WIMAX_OFDMA_DATA(_mgr, _src, _len) \
        (_mgr)->crc32_wimax_ofdma_data(_src, _len)

/**
 *  WIMAX OFDMA HCS CRC8 function (IEEE 802.16)
 */
#define IMB_CRC8_WIMAX_OFDMA_HCS(_mgr, _src, _len) \
        (_mgr)->crc8_wimax_ofdma_hcs(_src, _len)

/* Auxiliary functions */

/**
 * @brief DES key schedule set up.
 *
 * \a ks buffer needs to accommodate \a DES_KEY_SCHED_SIZE (128) bytes of data.
 *
 * @param[out] ks Destination buffer to accommodate DES key schedule
 * @param[in] key Pointer to an 8 byte DES key
 *
 * @return Operation status
 * @retval 0 success
 * @retval !0 error
 */
IMB_DLL_EXPORT int
des_key_schedule(uint64_t *ks, const void *key);

/**
 * Authenticate variable sized data with SHA1.
 *
 * @param[in] data    Data buffer
 * @param[in] length  Length of data in bytes for authentication.
 * @param[out] digest Digest output (20 bytes)
 */
IMB_DLL_EXPORT void sha1_sse(const void *data, const uint64_t length,
                             void *digest);

/**
 * @copydoc sha1_sse
 */
IMB_DLL_EXPORT void sha1_avx(const void *data, const uint64_t length,
                             void *digest);
/**
 * @copydoc sha1_sse
 */
IMB_DLL_EXPORT void sha1_avx2(const void *data, const uint64_t length,
                              void *digest);
/**
 * @copydoc sha1_sse
 */
IMB_DLL_EXPORT void sha1_avx512(const void *data, const uint64_t length,
                                 void *digest);

/**
 * Authenticate 64-byte data buffer with SHA1.
 *
 * @param[in] data    64-byte data buffer
 * @param[out] digest Digest output (20 bytes)
 */
IMB_DLL_EXPORT void sha1_one_block_sse(const void *data, void *digest);
/**
 * @copydoc sha1_one_block_sse
 */
IMB_DLL_EXPORT void sha1_one_block_avx(const void *data, void *digest);
/**
 * @copydoc sha1_one_block_sse
 */
IMB_DLL_EXPORT void sha1_one_block_avx2(const void *data, void *digest);
/**
 * @copydoc sha1_one_block_sse
 */
IMB_DLL_EXPORT void sha1_one_block_avx512(const void *data, void *digest);

/**
 * Authenticate variable sized data with SHA224.
 *
 * @param[in] data    Data buffer
 * @param[in] length  Length of data in bytes for authentication.
 * @param[out] digest Digest output (28 bytes)
 */
IMB_DLL_EXPORT void sha224_sse(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha224_sse
 */
IMB_DLL_EXPORT void sha224_avx(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha224_sse
 */
IMB_DLL_EXPORT void sha224_avx2(const void *data, const uint64_t length,
                                void *digest);
/**
 * @copydoc sha224_sse
 */
IMB_DLL_EXPORT void sha224_avx512(const void *data, const uint64_t length,
                                  void *digest);

/**
 * Authenticate 64-byte data buffer with SHA224.
 *
 * @param[in] data    64-byte data buffer
 * @param[out] digest Digest output (28 bytes)
 */
IMB_DLL_EXPORT void sha224_one_block_sse(const void *data, void *digest);
/**
 * @copydoc sha224_one_block_sse
 */
IMB_DLL_EXPORT void sha224_one_block_avx(const void *data, void *digest);
/**
 * @copydoc sha224_one_block_sse
 */
IMB_DLL_EXPORT void sha224_one_block_avx2(const void *data, void *digest);
/**
 * @copydoc sha224_one_block_sse
 */
IMB_DLL_EXPORT void sha224_one_block_avx512(const void *data, void *digest);

/**
 * Authenticate variable sized data with SHA256.
 *
 * @param[in] data    Data buffer
 * @param[in] length  Length of data in bytes for authentication.
 * @param[out] digest Digest output (32 bytes)
 */
IMB_DLL_EXPORT void sha256_sse(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha256_sse
 */
IMB_DLL_EXPORT void sha256_avx(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha256_sse
 */
IMB_DLL_EXPORT void sha256_avx2(const void *data, const uint64_t length,
                                void *digest);
/**
 * @copydoc sha256_sse
 */
IMB_DLL_EXPORT void sha256_avx512(const void *data, const uint64_t length,
                                  void *digest);

/**
 * Authenticate 64-byte data buffer with SHA256.
 *
 * @param[in] data    64-byte data buffer
 * @param[out] digest Digest output (32 bytes)
 */
IMB_DLL_EXPORT void sha256_one_block_sse(const void *data, void *digest);
/**
 * @copydoc sha256_one_block_sse
 */
IMB_DLL_EXPORT void sha256_one_block_avx(const void *data, void *digest);
/**
 * @copydoc sha256_one_block_sse
 */
IMB_DLL_EXPORT void sha256_one_block_avx2(const void *data, void *digest);
/**
 * @copydoc sha256_one_block_sse
 */
IMB_DLL_EXPORT void sha256_one_block_avx512(const void *data, void *digest);

/**
 * Authenticate variable sized data with SHA384.
 *
 * @param[in] data    Data buffer
 * @param[in] length  Length of data in bytes for authentication.
 * @param[out] digest Digest output (48 bytes)
 */
IMB_DLL_EXPORT void sha384_sse(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha384_sse
 */
IMB_DLL_EXPORT void sha384_avx(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha384_sse
 */
IMB_DLL_EXPORT void sha384_avx2(const void *data, const uint64_t length,
                                void *digest);
/**
 * @copydoc sha384_sse
 */
IMB_DLL_EXPORT void sha384_avx512(const void *data, const uint64_t length,
                                  void *digest);

/**
 * Authenticate 128-byte data buffer with SHA384.
 *
 * @param[in] data    64-byte data buffer
 * @param[out] digest Digest output (48 bytes)
 */
IMB_DLL_EXPORT void sha384_one_block_sse(const void *data, void *digest);
/**
 * @copydoc sha384_one_block_sse
 */
IMB_DLL_EXPORT void sha384_one_block_avx(const void *data, void *digest);
/**
 * @copydoc sha384_one_block_sse
 */
IMB_DLL_EXPORT void sha384_one_block_avx2(const void *data, void *digest);
/**
 * @copydoc sha384_one_block_sse
 */
IMB_DLL_EXPORT void sha384_one_block_avx512(const void *data, void *digest);

/**
 * Authenticate variable sized data with SHA512.
 *
 * @param[in] data    Data buffer
 * @param[in] length  Length of data in bytes for authentication.
 * @param[out] digest Digest output (64 bytes)
 */
IMB_DLL_EXPORT void sha512_sse(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha512_sse
 */
IMB_DLL_EXPORT void sha512_avx(const void *data, const uint64_t length,
                               void *digest);
/**
 * @copydoc sha512_sse
 */
IMB_DLL_EXPORT void sha512_avx2(const void *data, const uint64_t length,
                                void *digest);
/**
 * @copydoc sha512_sse
 */
IMB_DLL_EXPORT void sha512_avx512(const void *data, const uint64_t length,
                                  void *digest);

/**
 * Authenticate 64-byte data buffer with SHA512.
 *
 * @param[in] data    128-byte data buffer
 * @param[out] digest Digest output (64 bytes)
 */
IMB_DLL_EXPORT void sha512_one_block_sse(const void *data, void *digest);
/**
 * @copydoc sha512_one_block_sse
 */
IMB_DLL_EXPORT void sha512_one_block_avx(const void *data, void *digest);
/**
 * @copydoc sha512_one_block_sse
 */
IMB_DLL_EXPORT void sha512_one_block_avx2(const void *data, void *digest);
/**
 * @copydoc sha512_one_block_sse
 */
IMB_DLL_EXPORT void sha512_one_block_avx512(const void *data, void *digest);

/**
 * Authenticate 64-byte data buffer with MD5.
 *
 * @param[in] data    64-byte data buffer
 * @param[out] digest Digest output (16 bytes)
 */
IMB_DLL_EXPORT void md5_one_block_sse(const void *data, void *digest);
/**
 * @copydoc md5_one_block_sse
 */
IMB_DLL_EXPORT void md5_one_block_avx(const void *data, void *digest);
/**
 * @copydoc md5_one_block_sse
 */
IMB_DLL_EXPORT void md5_one_block_avx2(const void *data, void *digest);
/**
 * @copydoc md5_one_block_sse
 */
IMB_DLL_EXPORT void md5_one_block_avx512(const void *data, void *digest);


/**
 * Generate encryption/decryption AES-128 expansion keys.
 *
 * @param[in] key           AES-128 key
 * @param[out] enc_exp_keys AES-128 encryption expansion key
 * @param[out] dec_exp_keys AES-128 decryption expansion key
 */
IMB_DLL_EXPORT void aes_keyexp_128_sse(const void *key, void *enc_exp_keys,
                                       void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_128_sse
 */
IMB_DLL_EXPORT void aes_keyexp_128_avx(const void *key, void *enc_exp_keys,
                                       void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_128_sse
 */
IMB_DLL_EXPORT void aes_keyexp_128_avx2(const void *key, void *enc_exp_keys,
                                        void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_128_sse
 */
IMB_DLL_EXPORT void aes_keyexp_128_avx512(const void *key, void *enc_exp_keys,
                                          void *dec_exp_keys);

/**
 * Generate encryption/decryption AES-192 expansion keys.
 *
 * @param[in] key           AES-192 key
 * @param[out] enc_exp_keys AES-192 encryption expansion key
 * @param[out] dec_exp_keys AES-192 decryption expansion key
 */
IMB_DLL_EXPORT void aes_keyexp_192_sse(const void *key, void *enc_exp_keys,
                                       void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_256_sse
 */
IMB_DLL_EXPORT void aes_keyexp_192_avx(const void *key, void *enc_exp_keys,
                                       void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_256_sse
 */
IMB_DLL_EXPORT void aes_keyexp_192_avx2(const void *key, void *enc_exp_keys,
                                        void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_256_sse
 */
IMB_DLL_EXPORT void aes_keyexp_192_avx512(const void *key, void *enc_exp_keys,
                                          void *dec_exp_keys);

/**
 * Generate encryption/decryption AES-256 expansion keys.
 *
 * @param[in] key           AES-256 key
 * @param[out] enc_exp_keys AES-256 encryption expansion key
 * @param[out] dec_exp_keys AES-256 decryption expansion key
 */
IMB_DLL_EXPORT void aes_keyexp_256_sse(const void *key, void *enc_exp_keys,
                                       void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_256_sse
 */
IMB_DLL_EXPORT void aes_keyexp_256_avx(const void *key, void *enc_exp_keys,
                                       void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_256_sse
 */
IMB_DLL_EXPORT void aes_keyexp_256_avx2(const void *key, void *enc_exp_keys,
                                        void *dec_exp_keys);
/**
 * @copydoc aes_keyexp_256_sse
 */
IMB_DLL_EXPORT void aes_keyexp_256_avx512(const void *key, void *enc_exp_keys,
                                          void *dec_exp_keys);

/**
 * Generate encryption AES-128 expansion keys.
 *
 * @param[in] key           AES-128 key
 * @param[out] enc_exp_keys AES-128 encryption expansion key
 */
IMB_DLL_EXPORT void aes_keyexp_128_enc_sse(const void *key,
                                           void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_128_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_128_enc_avx(const void *key,
                                           void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_128_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_128_enc_avx2(const void *key,
                                            void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_128_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_128_enc_avx512(const void *key,
                                              void *enc_exp_keys);

/**
 * Generate encryption AES-192 expansion keys.
 *
 * @param[in] key           AES-192 key
 * @param[out] enc_exp_keys AES-192 encryption expansion key
 */
IMB_DLL_EXPORT void aes_keyexp_192_enc_sse(const void *key,
                                           void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_192_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_192_enc_avx(const void *key,
                                           void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_192_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_192_enc_avx2(const void *key,
                                            void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_192_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_192_enc_avx512(const void *key,
                                              void *enc_exp_keys);

/**
 * Generate encryption AES-256 expansion keys.
 *
 * @param[in] key           AES-256 key
 * @param[out] enc_exp_keys AES-256 encryption expansion key
 */
IMB_DLL_EXPORT void aes_keyexp_256_enc_sse(const void *key,
                                           void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_256_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_256_enc_avx(const void *key,
                                           void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_256_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_256_enc_avx2(const void *key,
                                            void *enc_exp_keys);
/**
 * @copydoc aes_keyexp_256_enc_sse
 */
IMB_DLL_EXPORT void aes_keyexp_256_enc_avx512(const void *key,
                                              void *enc_exp_keys);

/**
 * Generate AES-128-XCBC expansion keys.
 *
 * @param[in] key     Input AES-128-XCBC key
 * @param[out] k1_exp k1 expansion key
 * @param[out] k2     k2 key
 * @param[out] k3     k3 key
 */
IMB_DLL_EXPORT void aes_xcbc_expand_key_sse(const void *key, void *k1_exp,
                                            void *k2, void *k3);
/**
 * @copydoc aes_xcbc_expand_key_sse
 */
IMB_DLL_EXPORT void aes_xcbc_expand_key_avx(const void *key, void *k1_exp,
                                            void *k2, void *k3);
/**
 * @copydoc aes_xcbc_expand_key_sse
 */
IMB_DLL_EXPORT void aes_xcbc_expand_key_avx2(const void *key, void *k1_exp,
                                             void *k2, void *k3);
/**
 * @copydoc aes_xcbc_expand_key_sse
 */
IMB_DLL_EXPORT void aes_xcbc_expand_key_avx512(const void *key, void *k1_exp,
                                               void *k2, void *k3);

/**
 * Generate AES-128-CMAC subkeys.
 *
 * @param[in] key_exp Input expanded AES-128-CMAC key
 * @param[out] key1   Subkey 1
 * @param[out] key2   Subkey 2
 */
IMB_DLL_EXPORT void aes_cmac_subkey_gen_sse(const void *key_exp, void *key1,
                                            void *key2);
/**
 * @copydoc aes_cmac_subkey_gen_sse
 */
IMB_DLL_EXPORT void aes_cmac_subkey_gen_avx(const void *key_exp, void *key1,
                                            void *key2);
/**
 * @copydoc aes_cmac_subkey_gen_sse
 */
IMB_DLL_EXPORT void aes_cmac_subkey_gen_avx2(const void *key_exp, void *key1,
                                             void *key2);
/**
 * @copydoc aes_cmac_subkey_gen_sse
 */
IMB_DLL_EXPORT void aes_cmac_subkey_gen_avx512(const void *key_exp, void *key1,
                                               void *key2);
/**
 * @brief AES-CFB-128 Encrypt/Decrypt up to one block.
 *
 * Processes only one buffer at a time.
 * Designed to manage partial blocks of DOCSIS 3.1 SEC BPI.
 *
 * @param [out] out Plaintext/Ciphertext output
 * @param [in] in   Plaintext/Ciphertext input
 * @param [in] iv   Pointer to 16 byte IV
 * @param [in] keys Pointer to expanded AES keys
 * @param [in] len  Length of data in bytes
 */
IMB_DLL_EXPORT void aes_cfb_128_one_sse(void *out, const void *in,
                                        const void *iv, const void *keys,
                                        uint64_t len);
/**
 * @copydoc aes_cfb_128_one_sse
 */
IMB_DLL_EXPORT void aes_cfb_128_one_avx(void *out, const void *in,
                                        const void *iv, const void *keys,
                                        uint64_t len);
/**
 * @copydoc aes_cfb_128_one_sse
 */
IMB_DLL_EXPORT void aes_cfb_128_one_avx2(void *out, const void *in,
                                         const void *iv, const void *keys,
                                         uint64_t len);
/**
 * @copydoc aes_cfb_128_one_sse
 */
IMB_DLL_EXPORT void aes_cfb_128_one_avx512(void *out, const void *in,
                                           const void *iv, const void *keys,
                                           uint64_t len);

/*
 * Direct GCM API.
 * Note that GCM is also available through job API.
 */

/**
 * @brief AES-GCM-128 Encryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Ciphertext output. Encrypt in-place is allowed
 * @param [in] in               Plaintext input
 * @param [in] len              Length of data in bytes for encryption
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authentication Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are 16
 *                              (most likely), 12 or 8
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_sse(const struct gcm_key_data *key_data,
                    struct gcm_context_data *context_data,
                    uint8_t *out, uint8_t const *in, uint64_t len,
                    const uint8_t *iv, uint8_t const *aad, uint64_t aad_len,
                    uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_128_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_avx_gen2(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_128_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_avx_gen4(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @brief AES-GCM-192 Encryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Ciphertext output. Encrypt in-place is allowed
 * @param [in] in               Plaintext input
 * @param [in] len              Length of data in bytes for encryption
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authentication Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are 16
 *                              (most likely), 12 or 8
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_sse(const struct gcm_key_data *key_data,
                    struct gcm_context_data *context_data,
                    uint8_t *out, uint8_t const *in, uint64_t len,
                    const uint8_t *iv, uint8_t const *aad, uint64_t aad_len,
                    uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_192_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_avx_gen2(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_192_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_avx_gen4(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief AES-GCM-256 Encryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Ciphertext output. Encrypt in-place is allowed
 * @param [in] in               Plaintext input
 * @param [in] len              Length of data in bytes for encryption
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authentication Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are 16
 *                              (most likely), 12 or 8
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_sse(const struct gcm_key_data *key_data,
                    struct gcm_context_data *context_data,
                    uint8_t *out, uint8_t const *in, uint64_t len,
                    const uint8_t *iv,
                    uint8_t const *aad, uint64_t aad_len,
                    uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_256_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_avx_gen2(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_256_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_avx_gen4(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief AES-GCM-128 Decryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Plaintext output. Decrypt in-place is allowed
 * @param [in] in               Ciphertext input
 * @param [in] len              Length of data in bytes for decryption
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authentication Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are 16
 *                              (most likely), 12 or 8
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_sse(const struct gcm_key_data *key_data,
                    struct gcm_context_data *context_data,
                    uint8_t *out, uint8_t const *in, uint64_t len,
                    const uint8_t *iv, uint8_t const *aad, uint64_t aad_len,
                    uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_128_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_avx_gen2(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_128_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_avx_gen4(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief AES-GCM-192 Decryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Plaintext output. Decrypt in-place is allowed
 * @param [in] in               Ciphertext input
 * @param [in] len              Length of data in bytes for decryption
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authentication Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are 16
 *                              (most likely), 12 or 8
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_sse(const struct gcm_key_data *key_data,
                    struct gcm_context_data *context_data,
                    uint8_t *out, uint8_t const *in, uint64_t len,
                    const uint8_t *iv, uint8_t const *aad, uint64_t aad_len,
                    uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_192_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_avx_gen2(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_192_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_avx_gen4(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief AES-GCM-256 Decryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Plaintext output. Decrypt in-place is allowed
 * @param [in] in               Ciphertext input
 * @param [in] len              Length of data in bytes for decryption
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authentication Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are 16
 *                              (most likely), 12 or 8
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_sse(const struct gcm_key_data *key_data,
                    struct gcm_context_data *context_data,
                    uint8_t *out, uint8_t const *in, uint64_t len,
                    const uint8_t *iv, uint8_t const *aad, uint64_t aad_len,
                    uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_256_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_avx_gen2(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_256_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_avx_gen4(const struct gcm_key_data *key_data,
                         struct gcm_context_data *context_data,
                         uint8_t *out, uint8_t const *in, uint64_t len,
                         const uint8_t *iv,
                         uint8_t const *aad, uint64_t aad_len,
                         uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief Initialize a gcm_context_data structure to prepare for
 *        AES-GCM-128 Encryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authenticated Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 */
IMB_DLL_EXPORT void
aes_gcm_init_128_sse(const struct gcm_key_data *key_data,
                     struct gcm_context_data *context_data,
                     const uint8_t *iv, uint8_t const *aad, uint64_t aad_len);
/**
 * @copydoc aes_gcm_init_128_sse
 */
IMB_DLL_EXPORT void
aes_gcm_init_128_avx_gen2(const struct gcm_key_data *key_data,
                          struct gcm_context_data *context_data,
                          const uint8_t *iv,
                          uint8_t const *aad, uint64_t aad_len);
/**
 * @copydoc aes_gcm_init_128_sse
 */
IMB_DLL_EXPORT void
aes_gcm_init_128_avx_gen4(const struct gcm_key_data *key_data,
                          struct gcm_context_data *context_data,
                          const uint8_t *iv,
                          uint8_t const *aad, uint64_t aad_len);
/**
 * @brief Initialize a gcm_context_data structure to prepare for
 *        AES-GCM-192 Encryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authenticated Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 */
IMB_DLL_EXPORT void
aes_gcm_init_192_sse(const struct gcm_key_data *key_data,
                     struct gcm_context_data *context_data,
                     const uint8_t *iv, uint8_t const *aad, uint64_t aad_len);
/**
 * @copydoc aes_gcm_init_192_sse
 */
IMB_DLL_EXPORT void
aes_gcm_init_192_avx_gen2(const struct gcm_key_data *key_data,
                          struct gcm_context_data *context_data,
                          const uint8_t *iv,
                          uint8_t const *aad, uint64_t aad_len);
/**
 * @copydoc aes_gcm_init_192_sse
 */
IMB_DLL_EXPORT void
aes_gcm_init_192_avx_gen4(const struct gcm_key_data *key_data,
                          struct gcm_context_data *context_data,
                          const uint8_t *iv,
                          uint8_t const *aad, uint64_t aad_len);
/**
 * @brief Initialize a gcm_context_data structure to prepare for
 *        AES-GCM-256 Encryption.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [in] iv               Pointer to 12 byte IV structure
 *                              Internally, the library concatenates 0x00000001
 *                              to the IV
 * @param [in] aad              Additional Authenticated Data (AAD)
 * @param [in] aad_len          Length of AAD in bytes
 */
IMB_DLL_EXPORT void
aes_gcm_init_256_sse(const struct gcm_key_data *key_data,
                     struct gcm_context_data *context_data,
                     const uint8_t *iv, uint8_t const *aad, uint64_t aad_len);
/**
 * @copydoc aes_gcm_init_256_sse
 */
IMB_DLL_EXPORT void
aes_gcm_init_256_avx_gen2(const struct gcm_key_data *key_data,
                          struct gcm_context_data *context_data,
                          const uint8_t *iv,
                          uint8_t const *aad, uint64_t aad_len);
/**
 * @copydoc aes_gcm_init_256_sse
 */
IMB_DLL_EXPORT void
aes_gcm_init_256_avx_gen4(const struct gcm_key_data *key_data,
                          struct gcm_context_data *context_data,
                          const uint8_t *iv,
                          uint8_t const *aad, uint64_t aad_len);

/**
 * @brief Encrypt a block of a AES-GCM-128 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Ciphertext output. Encrypt in-place is allowed
 * @param [in] in               Plaintext input
 * @param [in] len              Length of data in bytes for encryption
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_update_sse(const struct gcm_key_data *key_data,
                           struct gcm_context_data *context_data,
                           uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_enc_128_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_update_avx_gen2(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_enc_128_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_update_avx_gen4(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);

/**
 * @brief Encrypt a block of a AES-GCM-192 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Ciphertext output. Encrypt in-place is allowed
 * @param [in] in               Plaintext input
 * @param [in] len              Length of data in bytes for encryption
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_update_sse(const struct gcm_key_data *key_data,
                           struct gcm_context_data *context_data,
                           uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_enc_192_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_update_avx_gen2(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_enc_192_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_update_avx_gen4(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);

/**
 * @brief Encrypt a block of a AES-GCM-256 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Ciphertext output. Encrypt in-place is allowed
 * @param [in] in               Plaintext input
 * @param [in] len              Length of data in bytes for encryption
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_update_sse(const struct gcm_key_data *key_data,
                           struct gcm_context_data *context_data,
                           uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_enc_256_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_update_avx_gen2(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_enc_256_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_update_avx_gen4(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);

/**
 * @brief Decrypt a block of a AES-GCM-128 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Plaintext output. Decrypt in-place is allowed
 * @param [in] in               Ciphertext input
 * @param [in] len              Length of data in bytes for decryption
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_update_sse(const struct gcm_key_data *key_data,
                           struct gcm_context_data *context_data,
                           uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_dec_128_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_update_avx_gen2(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_dec_128_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_update_avx_gen4(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);

/**
 * @brief Decrypt a block of a AES-GCM-192 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Plaintext output. Decrypt in-place is allowed
 * @param [in] in               Ciphertext input
 * @param [in] len              Length of data in bytes for decryption
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_update_sse(const struct gcm_key_data *key_data,
                           struct gcm_context_data *context_data,
                           uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_dec_192_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_update_avx_gen2(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_dec_192_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_update_avx_gen4(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);

/**
 * @brief Decrypt a block of a AES-GCM-256 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] out             Plaintext output. Decrypt in-place is allowed
 * @param [in] in               Ciphertext input
 * @param [in] len              Length of data in bytes for decryption
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_update_sse(const struct gcm_key_data *key_data,
                           struct gcm_context_data *context_data,
                           uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_dec_256_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_update_avx_gen2(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);
/**
 * @copydoc aes_gcm_dec_256_update_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_update_avx_gen4(const struct gcm_key_data *key_data,
                                struct gcm_context_data *context_data,
                                uint8_t *out, const uint8_t *in, uint64_t len);

/**
 * @brief End encryption of a AES-GCM-128 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are
 *                              16 (most likely), 12 or 8.
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_finalize_sse(const struct gcm_key_data *key_data,
                             struct gcm_context_data *context_data,
                             uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_128_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_finalize_avx_gen2(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_128_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_128_finalize_avx_gen4(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief End encryption of a AES-GCM-192 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are
 *                              16 (most likely), 12 or 8.
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_finalize_sse(const struct gcm_key_data *key_data,
                             struct gcm_context_data *context_data,
                             uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_192_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_finalize_avx_gen2(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_192_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_192_finalize_avx_gen4(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief End encryption of a AES-GCM-256 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are
 *                              16 (most likely), 12 or 8.
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_finalize_sse(const struct gcm_key_data *key_data,
                             struct gcm_context_data *context_data,
                             uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_256_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_finalize_avx_gen2(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_enc_256_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_enc_256_finalize_avx_gen4(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief End decryption of a AES-GCM-128 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are
 *                              16 (most likely), 12 or 8.
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_finalize_sse(const struct gcm_key_data *key_data,
                             struct gcm_context_data *context_data,
                             uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_128_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_finalize_avx_gen2(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_128_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_128_finalize_avx_gen4(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief End decryption of a AES-GCM-192 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are
 *                              16 (most likely), 12 or 8.
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_finalize_sse(const struct gcm_key_data *key_data,
                             struct gcm_context_data *context_data,
                             uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_192_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_finalize_avx_gen2(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_192_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_192_finalize_avx_gen4(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief End decryption of a AES-GCM-256 encryption message.
 *
 * @param [in] key_data         GCM expanded key data
 * @param [in,out] context_data GCM operation context data
 * @param [out] auth_tag        Authenticated Tag output
 * @param [in] auth_tag_len     Authenticated Tag Length in bytes (must be
 *                              a multiple of 4 bytes). Valid values are
 *                              16 (most likely), 12 or 8.
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_finalize_sse(const struct gcm_key_data *key_data,
                             struct gcm_context_data *context_data,
                             uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_256_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_finalize_avx_gen2(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);
/**
 * @copydoc aes_gcm_dec_256_finalize_sse
 */
IMB_DLL_EXPORT void
aes_gcm_dec_256_finalize_avx_gen4(const struct gcm_key_data *key_data,
                                  struct gcm_context_data *context_data,
                                  uint8_t *auth_tag, uint64_t auth_tag_len);

/**
 * @brief Precomputation of AES-GCM-128 HashKey constants.
 *
 * Precomputation of HashKey<<1 mod poly constants (shifted_hkey_X and
 * shifted_hkey_X_k).
 *
 * @param [in,out] key_data GCM key data
 */
IMB_DLL_EXPORT void aes_gcm_precomp_128_sse(struct gcm_key_data *key_data);

/**
 * @copydoc aes_gcm_precomp_128_sse
 */
IMB_DLL_EXPORT void aes_gcm_precomp_128_avx_gen2(struct gcm_key_data *key_data);

/**
 * @copydoc aes_gcm_precomp_128_sse
 */
IMB_DLL_EXPORT void aes_gcm_precomp_128_avx_gen4(struct gcm_key_data *key_data);

/**
 * @brief Precomputation of AES-GCM-192 HashKey constants.
 *
 * Precomputation of HashKey<<1 mod poly constants (shifted_hkey_X and
 * shifted_hkey_X_k).
 *
 * @param [in,out] key_data GCM key data
 */
IMB_DLL_EXPORT void aes_gcm_precomp_192_sse(struct gcm_key_data *key_data);

/**
 * @copydoc aes_gcm_precomp_192_sse
 */
IMB_DLL_EXPORT void aes_gcm_precomp_192_avx_gen2(struct gcm_key_data *key_data);

/**
 * @copydoc aes_gcm_precomp_192_sse
 */
IMB_DLL_EXPORT void aes_gcm_precomp_192_avx_gen4(struct gcm_key_data *key_data);

/**
 * @brief Precomputation of AES-GCM-256 HashKey constants.
 *
 * Precomputation of HashKey<<1 mod poly constants (shifted_hkey_X and
 * shifted_hkey_X_k).
 *
 * @param [in,out] key_data GCM key data
 */
IMB_DLL_EXPORT void aes_gcm_precomp_256_sse(struct gcm_key_data *key_data);

/**
 * @copydoc aes_gcm_precomp_256_sse
 */
IMB_DLL_EXPORT void aes_gcm_precomp_256_avx_gen2(struct gcm_key_data *key_data);

/**
 * @copydoc aes_gcm_precomp_256_sse
 */
IMB_DLL_EXPORT void aes_gcm_precomp_256_avx_gen4(struct gcm_key_data *key_data);

/**
 * @brief Pre-processes AES-GCM-128 key data.
 *
 * Prefills the gcm key data with key values for each round and
 * the initial sub hash key for tag encoding
 *
 * @param [in] key       Pointer to key data
 * @param [out] key_data GCM expanded key data
 */
IMB_DLL_EXPORT void aes_gcm_pre_128_sse(const void *key,
                                        struct gcm_key_data *key_data);
/**
 * @copydoc aes_gcm_pre_128_sse
 */
IMB_DLL_EXPORT void aes_gcm_pre_128_avx_gen2(const void *key,
                                             struct gcm_key_data *key_data);
/**
 * @copydoc aes_gcm_pre_128_sse
 */
IMB_DLL_EXPORT void aes_gcm_pre_128_avx_gen4(const void *key,
                                             struct gcm_key_data *key_data);
/**
 * @brief Pre-processes AES-GCM-192 key data.
 *
 * Prefills the gcm key data with key values for each round and
 * the initial sub hash key for tag encoding
 *
 * @param [in] key       Pointer to key data
 * @param [out] key_data GCM expanded key data
 */
IMB_DLL_EXPORT void aes_gcm_pre_192_sse(const void *key,
                                        struct gcm_key_data *key_data);
/**
 * @copydoc aes_gcm_pre_192_sse
 */
IMB_DLL_EXPORT void aes_gcm_pre_192_avx_gen2(const void *key,
                                             struct gcm_key_data *key_data);
/**
 * @copydoc aes_gcm_pre_192_sse
 */
IMB_DLL_EXPORT void aes_gcm_pre_192_avx_gen4(const void *key,
                                             struct gcm_key_data *key_data);
/**
 * @brief Pre-processes AES-GCM-256 key data.
 *
 * Prefills the gcm key data with key values for each round and
 * the initial sub hash key for tag encoding
 *
 * @param [in] key       Pointer to key data
 * @param [out] key_data GCM expanded key data
 */
IMB_DLL_EXPORT void aes_gcm_pre_256_sse(const void *key,
                                        struct gcm_key_data *key_data);
/**
 * @copydoc aes_gcm_pre_256_sse
 */
IMB_DLL_EXPORT void aes_gcm_pre_256_avx_gen2(const void *key,
                                             struct gcm_key_data *key_data);
/**
 * @copydoc aes_gcm_pre_256_sse
 */
IMB_DLL_EXPORT void aes_gcm_pre_256_avx_gen4(const void *key,
                                             struct gcm_key_data *key_data);

/**
 * @brief Generation of ZUC-EEA3 Initialization Vector.
 *
 * @param [in] count   COUNT (4 bytes in Little Endian)
 * @param [in] bearer  BEARER (5 bits)
 * @param [in] dir     DIRECTION (1 bit)
 * @param [out] iv_ptr Pointer to generated IV (16 bytes)
 *
 * @return Operation status
 * @retval 0 success
 * @retval -1 if one or more parameters are invalid
 */
IMB_DLL_EXPORT int zuc_eea3_iv_gen(const uint32_t count,
                                   const uint8_t bearer,
                                   const uint8_t dir,
                                   void *iv_ptr);
/**
 * @brief Generation of ZUC-EIA3 Initialization Vector.
 *
 * @param [in] count   COUNT (4 bytes in Little Endian)
 * @param [in] bearer  BEARER (5 bits)
 * @param [in] dir     DIRECTION (1 bit)
 * @param [out] iv_ptr Pointer to generated IV (16 bytes)
 *
 * @return Operation status
 * @retval 0 success
 * @retval -1 if one or more parameters are invalid
 */
IMB_DLL_EXPORT int zuc_eia3_iv_gen(const uint32_t count,
                                   const uint8_t bearer,
                                   const uint8_t dir,
                                   void *iv_ptr);

/**
 * @brief Generation of KASUMI F8 Initialization Vector.
 *
 * @param [in] count   COUNT (4 bytes in Little Endian)
 * @param [in] bearer  BEARER (5 bits)
 * @param [in] dir     DIRECTION (1 bit)
 * @param [out] iv_ptr Pointer to generated IV (16 bytes)
 *
 * @return Operation status
 * @retval 0 success
 * @retval -1 if one or more parameters are invalid
 */
IMB_DLL_EXPORT int kasumi_f8_iv_gen(const uint32_t count,
                                    const uint8_t bearer,
                                    const uint8_t dir,
                                    void *iv_ptr);
/**
 * @brief Generation of KASUMI F9 Initialization Vector.
 *
 * @param [in] count   COUNT (4 bytes in Little Endian)
 * @param [in] fresh   FRESH (4 bytes in Little Endian)
 * @param [out] iv_ptr Pointer to generated IV (16 bytes)
 *
 * @return Operation status
 * @retval 0 success
 * @retval -1 if one or more parameters are invalid
 */
IMB_DLL_EXPORT int kasumi_f9_iv_gen(const uint32_t count,
                                    const uint32_t fresh,
                                    void *iv_ptr);

/**
 * @brief Generation of SNOW3G F8 Initialization Vector.
 *
 * Parameters are passed in Little Endian format and
 * used to generate the IV in Big Endian format.
 *
 * @param [in] count   COUNT (4 bytes in Little Endian)
 * @param [in] bearer  BEARER (5 bits)
 * @param [in] dir     DIRECTION (1 bit)
 * @param [out] iv_ptr Pointer to generated IV (16 bytes) in Big Endian format
 *
 * @return Operation status
 * @retval 0 success
 * @retval -1 if one or more parameters are invalid
 */
IMB_DLL_EXPORT int snow3g_f8_iv_gen(const uint32_t count,
                                    const uint8_t bearer,
                                    const uint8_t dir,
                                    void *iv_ptr);
/**
 * @brief Generation of SNOW3G F9 Initialization Vector.
 *
 * Parameters are passed in Little Endian format and
 * used to generate the IV in Big Endian format.
 *
 * @param [in] count   COUNT (4 bytes in Little Endian)
 * @param [in] fresh   FRESH (4 bytes in Little Endian)
 * @param [in] dir     DIRECTION (1 bit)
 * @param [out] iv_ptr Pointer to generated IV (16 bytes) in Big Endian format
 *
 * @return Operation status
 * @retval 0 success
 * @retval -1 if one or more parameters are invalid
 */
IMB_DLL_EXPORT int snow3g_f9_iv_gen(const uint32_t count,
                                    const uint32_t fresh,
                                    const uint8_t dir,
                                    void *iv_ptr);
/**
 * @brief Force clearing/zeroing of memory
 *
 * @param [in] mem   Pointer to memory address to clear
 * @param [in] size  Size of memory to clear (in bytes)
 */
IMB_DLL_EXPORT void imb_clear_mem(void *mem, const size_t size);

#ifdef __cplusplus
}
#endif

#endif /* IMB_IPSEC_MB_H */
