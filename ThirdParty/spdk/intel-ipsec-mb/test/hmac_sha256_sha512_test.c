/*****************************************************************************
 Copyright (c) 2018-2022, Intel Corporation

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
*****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <intel-ipsec-mb.h>
#include "gcm_ctr_vectors_test.h"
#include "utils.h"

#define max_burst_jobs 32

int hmac_sha256_sha512_test(struct IMB_MGR *mb_mgr);

/*
 * Test vectors from https://tools.ietf.org/html/rfc4231
 */

/*
 * 4.2.  Test Case 1
 *
 *    Key =          0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b
 *                   0b0b0b0b                          (20 bytes)
 *    Data =         4869205468657265                  ("Hi There")
 *
 *    HMAC-SHA-224 = 896fb1128abbdf196832107cd49df33f
 *                   47b4b1169912ba4f53684b22
 *    HMAC-SHA-256 = b0344c61d8db38535ca8afceaf0bf12b
 *                   881dc200c9833da726e9376c2e32cff7
 *    HMAC-SHA-384 = afd03944d84895626b0825f4ab46907f
 *                   15f9dadbe4101ec682aa034c7cebc59c
 *                   faea9ea9076ede7f4af152e8b2fa9cb6
 *    HMAC-SHA-512 = 87aa7cdea5ef619d4ff0b4241a1d6cb0
 *                   2379f4e2ce4ec2787ad0b30545e17cde
 *                   daa833b7d6b8a702038b274eaea3f4e4
 *                   be9d914eeb61f1702e696c203a126854
 */
static const uint8_t key_1[] = {
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b
};
static const uint8_t data_1[] = {
        0x48, 0x69, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65
};
static const uint8_t hmac_sha_224_1[] = {
        0x89, 0x6f, 0xb1, 0x12, 0x8a, 0xbb, 0xdf, 0x19,
        0x68, 0x32, 0x10, 0x7c, 0xd4, 0x9d, 0xf3, 0x3f,
        0x47, 0xb4, 0xb1, 0x16, 0x99, 0x12, 0xba, 0x4f,
        0x53, 0x68, 0x4b, 0x22
};
static const uint8_t hmac_sha_256_1[] = {
        0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
        0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
        0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7
};
static const uint8_t hmac_sha_384_1[] = {
        0xaf, 0xd0, 0x39, 0x44, 0xd8, 0x48, 0x95, 0x62,
        0x6b, 0x08, 0x25, 0xf4, 0xab, 0x46, 0x90, 0x7f,
        0x15, 0xf9, 0xda, 0xdb, 0xe4, 0x10, 0x1e, 0xc6,
        0x82, 0xaa, 0x03, 0x4c, 0x7c, 0xeb, 0xc5, 0x9c,
        0xfa, 0xea, 0x9e, 0xa9, 0x07, 0x6e, 0xde, 0x7f,
        0x4a, 0xf1, 0x52, 0xe8, 0xb2, 0xfa, 0x9c, 0xb6
};
static const uint8_t hmac_sha_512_1[] = {
        0x87, 0xaa, 0x7c, 0xde, 0xa5, 0xef, 0x61, 0x9d,
        0x4f, 0xf0, 0xb4, 0x24, 0x1a, 0x1d, 0x6c, 0xb0,
        0x23, 0x79, 0xf4, 0xe2, 0xce, 0x4e, 0xc2, 0x78,
        0x7a, 0xd0, 0xb3, 0x05, 0x45, 0xe1, 0x7c, 0xde,
        0xda, 0xa8, 0x33, 0xb7, 0xd6, 0xb8, 0xa7, 0x02,
        0x03, 0x8b, 0x27, 0x4e, 0xae, 0xa3, 0xf4, 0xe4,
        0xbe, 0x9d, 0x91, 0x4e, 0xeb, 0x61, 0xf1, 0x70,
        0x2e, 0x69, 0x6c, 0x20, 0x3a, 0x12, 0x68, 0x54
};

/*
 * 4.3.  Test Case 2
 *
 *    Test with a key shorter than the length of the HMAC output.
 *
 *    Key =          4a656665                          ("Jefe")
 *    Data =         7768617420646f2079612077616e7420  ("what do ya want ")
 *                   666f72206e6f7468696e673f          ("for nothing?")
 *
 *    HMAC-SHA-224 = a30e01098bc6dbbf45690f3a7e9e6d0f
 *                   8bbea2a39e6148008fd05e44
 *    HMAC-SHA-256 = 5bdcc146bf60754e6a042426089575c7
 *                   5a003f089d2739839dec58b964ec3843
 *    HMAC-SHA-384 = af45d2e376484031617f78d2b58a6b1b
 *                   9c7ef464f5a01b47e42ec3736322445e
 *                   8e2240ca5e69e2c78b3239ecfab21649
 *    HMAC-SHA-512 = 164b7a7bfcf819e2e395fbe73b56e0a3
 *                   87bd64222e831fd610270cd7ea250554
 *                   9758bf75c05a994a6d034f65f8f0e6fd
 *                   caeab1a34d4a6b4b636e070a38bce737
 */
static const uint8_t key_2[] = {
        0x4a, 0x65, 0x66, 0x65
};
static const uint8_t data_2[] = {
        0x77, 0x68, 0x61, 0x74, 0x20, 0x64, 0x6f, 0x20,
        0x79, 0x61, 0x20, 0x77, 0x61, 0x6e, 0x74, 0x20,
        0x66, 0x6f, 0x72, 0x20, 0x6e, 0x6f, 0x74, 0x68,
        0x69, 0x6e, 0x67, 0x3f
};
static const uint8_t hmac_sha_224_2[] = {
        0xa3, 0x0e, 0x01, 0x09, 0x8b, 0xc6, 0xdb, 0xbf,
        0x45, 0x69, 0x0f, 0x3a, 0x7e, 0x9e, 0x6d, 0x0f,
        0x8b, 0xbe, 0xa2, 0xa3, 0x9e, 0x61, 0x48, 0x00,
        0x8f, 0xd0, 0x5e, 0x44
};
static const uint8_t hmac_sha_256_2[] = {
        0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e,
        0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7,
        0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83,
        0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43
};
static const uint8_t hmac_sha_384_2[] = {
        0xaf, 0x45, 0xd2, 0xe3, 0x76, 0x48, 0x40, 0x31,
        0x61, 0x7f, 0x78, 0xd2, 0xb5, 0x8a, 0x6b, 0x1b,
        0x9c, 0x7e, 0xf4, 0x64, 0xf5, 0xa0, 0x1b, 0x47,
        0xe4, 0x2e, 0xc3, 0x73, 0x63, 0x22, 0x44, 0x5e,
        0x8e, 0x22, 0x40, 0xca, 0x5e, 0x69, 0xe2, 0xc7,
        0x8b, 0x32, 0x39, 0xec, 0xfa, 0xb2, 0x16, 0x49
};
static const uint8_t hmac_sha_512_2[] = {
        0x16, 0x4b, 0x7a, 0x7b, 0xfc, 0xf8, 0x19, 0xe2,
        0xe3, 0x95, 0xfb, 0xe7, 0x3b, 0x56, 0xe0, 0xa3,
        0x87, 0xbd, 0x64, 0x22, 0x2e, 0x83, 0x1f, 0xd6,
        0x10, 0x27, 0x0c, 0xd7, 0xea, 0x25, 0x05, 0x54,
        0x97, 0x58, 0xbf, 0x75, 0xc0, 0x5a, 0x99, 0x4a,
        0x6d, 0x03, 0x4f, 0x65, 0xf8, 0xf0, 0xe6, 0xfd,
        0xca, 0xea, 0xb1, 0xa3, 0x4d, 0x4a, 0x6b, 0x4b,
        0x63, 0x6e, 0x07, 0x0a, 0x38, 0xbc, 0xe7, 0x37
};

/*
 * 4.4.  Test Case 3
 *
 *    Test with a combined length of key and data that is larger than 64
 *    bytes (= block-size of SHA-224 and SHA-256).
 *
 *    Key            aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaa                          (20 bytes)
 *    Data =         dddddddddddddddddddddddddddddddd
 *                   dddddddddddddddddddddddddddddddd
 *                   dddddddddddddddddddddddddddddddd
 *                   dddd                              (50 bytes)
 *
 *    HMAC-SHA-224 = 7fb3cb3588c6c1f6ffa9694d7d6ad264
 *                   9365b0c1f65d69d1ec8333ea
 *    HMAC-SHA-256 = 773ea91e36800e46854db8ebd09181a7
 *                   2959098b3ef8c122d9635514ced565fe
 *    HMAC-SHA-384 = 88062608d3e6ad8a0aa2ace014c8a86f
 *                   0aa635d947ac9febe83ef4e55966144b
 *                   2a5ab39dc13814b94e3ab6e101a34f27
 *    HMAC-SHA-512 = fa73b0089d56a284efb0f0756c890be9
 *                   b1b5dbdd8ee81a3655f83e33b2279d39
 *                   bf3e848279a722c806b485a47e67c807
 *                   b946a337bee8942674278859e13292fb
 */
static const uint8_t key_3[] = {
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa
};
static const uint8_t data_3[] = {
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
        0xdd, 0xdd
};
static const uint8_t hmac_sha_224_3[] = {
        0x7f, 0xb3, 0xcb, 0x35, 0x88, 0xc6, 0xc1, 0xf6,
        0xff, 0xa9, 0x69, 0x4d, 0x7d, 0x6a, 0xd2, 0x64,
        0x93, 0x65, 0xb0, 0xc1, 0xf6, 0x5d, 0x69, 0xd1,
        0xec, 0x83, 0x33, 0xea
};
static const uint8_t hmac_sha_256_3[] = {
        0x77, 0x3e, 0xa9, 0x1e, 0x36, 0x80, 0x0e, 0x46,
        0x85, 0x4d, 0xb8, 0xeb, 0xd0, 0x91, 0x81, 0xa7,
        0x29, 0x59, 0x09, 0x8b, 0x3e, 0xf8, 0xc1, 0x22,
        0xd9, 0x63, 0x55, 0x14, 0xce, 0xd5, 0x65, 0xfe
};
static const uint8_t hmac_sha_384_3[] = {
        0x88, 0x06, 0x26, 0x08, 0xd3, 0xe6, 0xad, 0x8a,
        0x0a, 0xa2, 0xac, 0xe0, 0x14, 0xc8, 0xa8, 0x6f,
        0x0a, 0xa6, 0x35, 0xd9, 0x47, 0xac, 0x9f, 0xeb,
        0xe8, 0x3e, 0xf4, 0xe5, 0x59, 0x66, 0x14, 0x4b,
        0x2a, 0x5a, 0xb3, 0x9d, 0xc1, 0x38, 0x14, 0xb9,
        0x4e, 0x3a, 0xb6, 0xe1, 0x01, 0xa3, 0x4f, 0x27
};
static const uint8_t hmac_sha_512_3[] = {
        0xfa, 0x73, 0xb0, 0x08, 0x9d, 0x56, 0xa2, 0x84,
        0xef, 0xb0, 0xf0, 0x75, 0x6c, 0x89, 0x0b, 0xe9,
        0xb1, 0xb5, 0xdb, 0xdd, 0x8e, 0xe8, 0x1a, 0x36,
        0x55, 0xf8, 0x3e, 0x33, 0xb2, 0x27, 0x9d, 0x39,
        0xbf, 0x3e, 0x84, 0x82, 0x79, 0xa7, 0x22, 0xc8,
        0x06, 0xb4, 0x85, 0xa4, 0x7e, 0x67, 0xc8, 0x07,
        0xb9, 0x46, 0xa3, 0x37, 0xbe, 0xe8, 0x94, 0x26,
        0x74, 0x27, 0x88, 0x59, 0xe1, 0x32, 0x92, 0xfb
};

/*
 * 4.5.  Test Case 4
 *
 *    Test with a combined length of key and data that is larger than 64
 *    bytes (= block-size of SHA-224 and SHA-256).
 *
 *    Key =          0102030405060708090a0b0c0d0e0f10
 *                   111213141516171819                (25 bytes)
 *    Data =         cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd
 *                   cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd
 *                   cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd
 *                   cdcd                              (50 bytes)
 *
 *    HMAC-SHA-224 = 6c11506874013cac6a2abc1bb382627c
 *                   ec6a90d86efc012de7afec5a
 *    HMAC-SHA-256 = 82558a389a443c0ea4cc819899f2083a
 *                   85f0faa3e578f8077a2e3ff46729665b
 *    HMAC-SHA-384 = 3e8a69b7783c25851933ab6290af6ca7
 *                   7a9981480850009cc5577c6e1f573b4e
 *                   6801dd23c4a7d679ccf8a386c674cffb
 *    HMAC-SHA-512 = b0ba465637458c6990e5a8c5f61d4af7
 *                   e576d97ff94b872de76f8050361ee3db
 *                   a91ca5c11aa25eb4d679275cc5788063
 *                   a5f19741120c4f2de2adebeb10a298dd
 */
static const uint8_t key_4[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19
};
static const uint8_t data_4[] = {
        0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
        0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
        0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
        0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
        0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
        0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
        0xcd, 0xcd
};
static const uint8_t hmac_sha_224_4[] = {
        0x6c, 0x11, 0x50, 0x68, 0x74, 0x01, 0x3c, 0xac,
        0x6a, 0x2a, 0xbc, 0x1b, 0xb3, 0x82, 0x62, 0x7c,
        0xec, 0x6a, 0x90, 0xd8, 0x6e, 0xfc, 0x01, 0x2d,
        0xe7, 0xaf, 0xec, 0x5a
};
static const uint8_t hmac_sha_256_4[] = {
        0x82, 0x55, 0x8a, 0x38, 0x9a, 0x44, 0x3c, 0x0e,
        0xa4, 0xcc, 0x81, 0x98, 0x99, 0xf2, 0x08, 0x3a,
        0x85, 0xf0, 0xfa, 0xa3, 0xe5, 0x78, 0xf8, 0x07,
        0x7a, 0x2e, 0x3f, 0xf4, 0x67, 0x29, 0x66, 0x5b
};
static const uint8_t hmac_sha_384_4[] = {
        0x3e, 0x8a, 0x69, 0xb7, 0x78, 0x3c, 0x25, 0x85,
        0x19, 0x33, 0xab, 0x62, 0x90, 0xaf, 0x6c, 0xa7,
        0x7a, 0x99, 0x81, 0x48, 0x08, 0x50, 0x00, 0x9c,
        0xc5, 0x57, 0x7c, 0x6e, 0x1f, 0x57, 0x3b, 0x4e,
        0x68, 0x01, 0xdd, 0x23, 0xc4, 0xa7, 0xd6, 0x79,
        0xcc, 0xf8, 0xa3, 0x86, 0xc6, 0x74, 0xcf, 0xfb
};
static const uint8_t hmac_sha_512_4[] = {
        0xb0, 0xba, 0x46, 0x56, 0x37, 0x45, 0x8c, 0x69,
        0x90, 0xe5, 0xa8, 0xc5, 0xf6, 0x1d, 0x4a, 0xf7,
        0xe5, 0x76, 0xd9, 0x7f, 0xf9, 0x4b, 0x87, 0x2d,
        0xe7, 0x6f, 0x80, 0x50, 0x36, 0x1e, 0xe3, 0xdb,
        0xa9, 0x1c, 0xa5, 0xc1, 0x1a, 0xa2, 0x5e, 0xb4,
        0xd6, 0x79, 0x27, 0x5c, 0xc5, 0x78, 0x80, 0x63,
        0xa5, 0xf1, 0x97, 0x41, 0x12, 0x0c, 0x4f, 0x2d,
        0xe2, 0xad, 0xeb, 0xeb, 0x10, 0xa2, 0x98, 0xdd
};

/*
 *
 * 4.6.  Test Case 5
 *
 *    Test with a truncation of output to 128 bits.
 *
 *    Key =          0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c
 *                   0c0c0c0c                          (20 bytes)
 *    Data =         546573742057697468205472756e6361  ("Test With Trunca")
 *                   74696f6e                          ("tion")
 *
 *    HMAC-SHA-224 = 0e2aea68a90c8d37c988bcdb9fca6fa8
 *    HMAC-SHA-256 = a3b6167473100ee06e0c796c2955552b
 *    HMAC-SHA-384 = 3abf34c3503b2a23a46efc619baef897
 *    HMAC-SHA-512 = 415fad6271580a531d4179bc891d87a6
 */
/* static const uint8_t key_5[] = { */
/*         0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, */
/*         0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, */
/*         0x0c, 0x0c, 0x0c, 0x0c */
/* }; */
/* static const uint8_t data_5[] = { */
/*         0x54, 0x65, 0x73, 0x74, 0x20, 0x57, 0x69, 0x74, */
/*         0x68, 0x20, 0x54, 0x72, 0x75, 0x6e, 0x63, 0x61, */
/*         0x74, 0x69, 0x6f, 0x6e */
/* }; */
/* static const uint8_t hmac_sha_224_5[] = { */
/*         0x0e, 0x2a, 0xea, 0x68, 0xa9, 0x0c, 0x8d, 0x37, */
/*         0xc9, 0x88, 0xbc, 0xdb, 0x9f, 0xca, 0x6f, 0xa8 */
/* }; */
/* static const uint8_t hmac_sha_256_5[] = { */
/*         0xa3, 0xb6, 0x16, 0x74, 0x73, 0x10, 0x0e, 0xe0, */
/*         0x6e, 0x0c, 0x79, 0x6c, 0x29, 0x55, 0x55, 0x2b */
/* }; */
/* static const uint8_t hmac_sha_384_5[] = { */
/*         0x3a, 0xbf, 0x34, 0xc3, 0x50, 0x3b, 0x2a, 0x23, */
/*         0xa4, 0x6e, 0xfc, 0x61, 0x9b, 0xae, 0xf8, 0x97 */
/* }; */
/* static const uint8_t hmac_sha_512_5[] = { */
/*         0x41, 0x5f, 0xad, 0x62, 0x71, 0x58, 0x0a, 0x53, */
/*         0x1d, 0x41, 0x79, 0xbc, 0x89, 0x1d, 0x87, 0xa6 */
/* }; */

/*
 * 4.7.  Test Case 6
 *
 *    Test with a key larger than 128 bytes (= block-size of SHA-384 and
 *    SHA-512).
 *
 *    Key =          aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaa                            (131 bytes)
 *    Data =         54657374205573696e67204c61726765  ("Test Using Large")
 *                   72205468616e20426c6f636b2d53697a  ("r Than Block-Siz")
 *                   65204b6579202d2048617368204b6579  ("e Key - Hash Key")
 *                   204669727374                      (" First")
 *
 *    HMAC-SHA-224 = 95e9a0db962095adaebe9b2d6f0dbce2
 *                   d499f112f2d2b7273fa6870e
 *    HMAC-SHA-256 = 60e431591ee0b67f0d8a26aacbf5b77f
 *                   8e0bc6213728c5140546040f0ee37f54
 *    HMAC-SHA-384 = 4ece084485813e9088d2c63a041bc5b4
 *                   4f9ef1012a2b588f3cd11f05033ac4c6
 *                   0c2ef6ab4030fe8296248df163f44952
 *    HMAC-SHA-512 = 80b24263c7c1a3ebb71493c1dd7be8b4
 *                   9b46d1f41b4aeec1121b013783f8f352
 *                   6b56d037e05f2598bd0fd2215d6a1e52
 *                   95e64f73f63f0aec8b915a985d786598
 */
static const uint8_t key_6[] = {
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa
};
static const uint8_t data_6[] = {
        0x54, 0x65, 0x73, 0x74, 0x20, 0x55, 0x73, 0x69,
        0x6e, 0x67, 0x20, 0x4c, 0x61, 0x72, 0x67, 0x65,
        0x72, 0x20, 0x54, 0x68, 0x61, 0x6e, 0x20, 0x42,
        0x6c, 0x6f, 0x63, 0x6b, 0x2d, 0x53, 0x69, 0x7a,
        0x65, 0x20, 0x4b, 0x65, 0x79, 0x20, 0x2d, 0x20,
        0x48, 0x61, 0x73, 0x68, 0x20, 0x4b, 0x65, 0x79,
        0x20, 0x46, 0x69, 0x72, 0x73, 0x74
};
static const uint8_t hmac_sha_224_6[] = {
        0x95, 0xe9, 0xa0, 0xdb, 0x96, 0x20, 0x95, 0xad,
        0xae, 0xbe, 0x9b, 0x2d, 0x6f, 0x0d, 0xbc, 0xe2,
        0xd4, 0x99, 0xf1, 0x12, 0xf2, 0xd2, 0xb7, 0x27,
        0x3f, 0xa6, 0x87, 0x0e
};
static const uint8_t hmac_sha_256_6[] = {
        0x60, 0xe4, 0x31, 0x59, 0x1e, 0xe0, 0xb6, 0x7f,
        0x0d, 0x8a, 0x26, 0xaa, 0xcb, 0xf5, 0xb7, 0x7f,
        0x8e, 0x0b, 0xc6, 0x21, 0x37, 0x28, 0xc5, 0x14,
        0x05, 0x46, 0x04, 0x0f, 0x0e, 0xe3, 0x7f, 0x54
};
static const uint8_t hmac_sha_384_6[] = {
        0x4e, 0xce, 0x08, 0x44, 0x85, 0x81, 0x3e, 0x90,
        0x88, 0xd2, 0xc6, 0x3a, 0x04, 0x1b, 0xc5, 0xb4,
        0x4f, 0x9e, 0xf1, 0x01, 0x2a, 0x2b, 0x58, 0x8f,
        0x3c, 0xd1, 0x1f, 0x05, 0x03, 0x3a, 0xc4, 0xc6,
        0x0c, 0x2e, 0xf6, 0xab, 0x40, 0x30, 0xfe, 0x82,
        0x96, 0x24, 0x8d, 0xf1, 0x63, 0xf4, 0x49, 0x52
};
static const uint8_t hmac_sha_512_6[] = {
        0x80, 0xb2, 0x42, 0x63, 0xc7, 0xc1, 0xa3, 0xeb,
        0xb7, 0x14, 0x93, 0xc1, 0xdd, 0x7b, 0xe8, 0xb4,
        0x9b, 0x46, 0xd1, 0xf4, 0x1b, 0x4a, 0xee, 0xc1,
        0x12, 0x1b, 0x01, 0x37, 0x83, 0xf8, 0xf3, 0x52,
        0x6b, 0x56, 0xd0, 0x37, 0xe0, 0x5f, 0x25, 0x98,
        0xbd, 0x0f, 0xd2, 0x21, 0x5d, 0x6a, 0x1e, 0x52,
        0x95, 0xe6, 0x4f, 0x73, 0xf6, 0x3f, 0x0a, 0xec,
        0x8b, 0x91, 0x5a, 0x98, 0x5d, 0x78, 0x65, 0x98
};

/*
 * 4.8.  Test Case 7
 *
 *    Test with a key and data that is larger than 128 bytes (= block-size
 *    of SHA-384 and SHA-512).
 *
 *    Key =          aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 *                   aaaaaa                            (131 bytes)
 *    Data =         54686973206973206120746573742075  ("This is a test u")
 *                   73696e672061206c6172676572207468  ("sing a larger th")
 *                   616e20626c6f636b2d73697a65206b65  ("an block-size ke")
 *                   7920616e642061206c61726765722074  ("y and a larger t")
 *                   68616e20626c6f636b2d73697a652064  ("han block-size d")
 *                   6174612e20546865206b6579206e6565  ("ata. The key nee")
 *                   647320746f2062652068617368656420  ("ds to be hashed ")
 *                   6265666f7265206265696e6720757365  ("before being use")
 *                   642062792074686520484d414320616c  ("d by the HMAC al")
 *                   676f726974686d2e                  ("gorithm.")
 *
 *    HMAC-SHA-224 = 3a854166ac5d9f023f54d517d0b39dbd
 *                   946770db9c2b95c9f6f565d1
 *    HMAC-SHA-256 = 9b09ffa71b942fcb27635fbcd5b0e944
 *                   bfdc63644f0713938a7f51535c3a35e2
 *    HMAC-SHA-384 = 6617178e941f020d351e2f254e8fd32c
 *                   602420feb0b8fb9adccebb82461e99c5
 *                   a678cc31e799176d3860e6110c46523e
 *    HMAC-SHA-512 = e37b6a775dc87dbaa4dfa9f96e5e3ffd
 *                   debd71f8867289865df5a32d20cdc944
 *                   b6022cac3c4982b10d5eeb55c3e4de15
 *                   134676fb6de0446065c97440fa8c6a58
 */
static const uint8_t key_7[] = {
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa
};
static const uint8_t data_7[] = {
        0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20,
        0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x75,
        0x73, 0x69, 0x6e, 0x67, 0x20, 0x61, 0x20, 0x6c,
        0x61, 0x72, 0x67, 0x65, 0x72, 0x20, 0x74, 0x68,
        0x61, 0x6e, 0x20, 0x62, 0x6c, 0x6f, 0x63, 0x6b,
        0x2d, 0x73, 0x69, 0x7a, 0x65, 0x20, 0x6b, 0x65,
        0x79, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x61, 0x20,
        0x6c, 0x61, 0x72, 0x67, 0x65, 0x72, 0x20, 0x74,
        0x68, 0x61, 0x6e, 0x20, 0x62, 0x6c, 0x6f, 0x63,
        0x6b, 0x2d, 0x73, 0x69, 0x7a, 0x65, 0x20, 0x64,
        0x61, 0x74, 0x61, 0x2e, 0x20, 0x54, 0x68, 0x65,
        0x20, 0x6b, 0x65, 0x79, 0x20, 0x6e, 0x65, 0x65,
        0x64, 0x73, 0x20, 0x74, 0x6f, 0x20, 0x62, 0x65,
        0x20, 0x68, 0x61, 0x73, 0x68, 0x65, 0x64, 0x20,
        0x62, 0x65, 0x66, 0x6f, 0x72, 0x65, 0x20, 0x62,
        0x65, 0x69, 0x6e, 0x67, 0x20, 0x75, 0x73, 0x65,
        0x64, 0x20, 0x62, 0x79, 0x20, 0x74, 0x68, 0x65,
        0x20, 0x48, 0x4d, 0x41, 0x43, 0x20, 0x61, 0x6c,
        0x67, 0x6f, 0x72, 0x69, 0x74, 0x68, 0x6d, 0x2e
};
static const uint8_t hmac_sha_224_7[] = {
        0x3a, 0x85, 0x41, 0x66, 0xac, 0x5d, 0x9f, 0x02,
        0x3f, 0x54, 0xd5, 0x17, 0xd0, 0xb3, 0x9d, 0xbd,
        0x94, 0x67, 0x70, 0xdb, 0x9c, 0x2b, 0x95, 0xc9,
        0xf6, 0xf5, 0x65, 0xd1
};
static const uint8_t hmac_sha_256_7[] = {
        0x9b, 0x09, 0xff, 0xa7, 0x1b, 0x94, 0x2f, 0xcb,
        0x27, 0x63, 0x5f, 0xbc, 0xd5, 0xb0, 0xe9, 0x44,
        0xbf, 0xdc, 0x63, 0x64, 0x4f, 0x07, 0x13, 0x93,
        0x8a, 0x7f, 0x51, 0x53, 0x5c, 0x3a, 0x35, 0xe2
};
static const uint8_t hmac_sha_384_7[] = {
        0x66, 0x17, 0x17, 0x8e, 0x94, 0x1f, 0x02, 0x0d,
        0x35, 0x1e, 0x2f, 0x25, 0x4e, 0x8f, 0xd3, 0x2c,
        0x60, 0x24, 0x20, 0xfe, 0xb0, 0xb8, 0xfb, 0x9a,
        0xdc, 0xce, 0xbb, 0x82, 0x46, 0x1e, 0x99, 0xc5,
        0xa6, 0x78, 0xcc, 0x31, 0xe7, 0x99, 0x17, 0x6d,
        0x38, 0x60, 0xe6, 0x11, 0x0c, 0x46, 0x52, 0x3e
};
static const uint8_t hmac_sha_512_7[] = {
        0xe3, 0x7b, 0x6a, 0x77, 0x5d, 0xc8, 0x7d, 0xba,
        0xa4, 0xdf, 0xa9, 0xf9, 0x6e, 0x5e, 0x3f, 0xfd,
        0xde, 0xbd, 0x71, 0xf8, 0x86, 0x72, 0x89, 0x86,
        0x5d, 0xf5, 0xa3, 0x2d, 0x20, 0xcd, 0xc9, 0x44,
        0xb6, 0x02, 0x2c, 0xac, 0x3c, 0x49, 0x82, 0xb1,
        0x0d, 0x5e, 0xeb, 0x55, 0xc3, 0xe4, 0xde, 0x15,
        0x13, 0x46, 0x76, 0xfb, 0x6d, 0xe0, 0x44, 0x60,
        0x65, 0xc9, 0x74, 0x40, 0xfa, 0x8c, 0x6a, 0x58
};

/*
 * Test Case 8
 *
 * Test vector from https://csrc.nist.gov/csrc/media/projects/
 * cryptographic-standards-and-guidelines/documents/examples/hmac_sha224.pdf
 */
static const uint8_t key_8[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};
static const uint8_t data_8[] = {
        0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x6d,
        0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x66,
        0x6f, 0x72, 0x20, 0x6b, 0x65, 0x79, 0x6c, 0x65,
        0x6e, 0x3d, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x6c,
        0x65, 0x6e
};
static const uint8_t hmac_sha_224_8[] = {
        0xc7, 0x40, 0x5e, 0x3a, 0xe0, 0x58, 0xe8, 0xcd,
        0x30, 0xb0, 0x8b, 0x41, 0x40, 0x24, 0x85, 0x81,
        0xed, 0x17, 0x4c, 0xb3, 0x4e, 0x12, 0x24, 0xbc,
        0xc1, 0xef, 0xc8, 0x1b
};

/*
 * Test Case 9
 *
 * Test vector from https://csrc.nist.gov/csrc/media/projects/
 * cryptographic-standards-and-guidelines/documents/examples/hmac_sha256.pdf
 */
static const uint8_t key_9[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};
static const uint8_t data_9[] = {
        0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x6d,
        0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x66,
        0x6f, 0x72, 0x20, 0x6b, 0x65, 0x79, 0x6c, 0x65,
        0x6e, 0x3d, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x6c,
        0x65, 0x6e
};
static const uint8_t hmac_sha_256_9[] = {
        0x8b, 0xb9, 0xa1, 0xdb, 0x98, 0x06, 0xf2, 0x0d,
        0xf7, 0xf7, 0x7b, 0x82, 0x13, 0x8c, 0x79, 0x14,
        0xd1, 0x74, 0xd5, 0x9e, 0x13, 0xdc, 0x4d, 0x01,
        0x69, 0xc9, 0x05, 0x7b, 0x13, 0x3e, 0x1d, 0x62,
};

/*
 * Test Case 10
 *
 * Test vector from https://csrc.nist.gov/csrc/media/projects/
 * cryptographic-standards-and-guidelines/documents/examples/hmac_sha384.pdf
 */
static const uint8_t key_10[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
        0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};
static const uint8_t data_10[] = {
        0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x6d,
        0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x66,
        0x6f, 0x72, 0x20, 0x6b, 0x65, 0x79, 0x6c, 0x65,
        0x6e, 0x3d, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x6c,
        0x65, 0x6e
};
static const uint8_t hmac_sha_384_10[] = {
        0x63, 0xc5, 0xda, 0xa5, 0xe6, 0x51, 0x84, 0x7c,
        0xa8, 0x97, 0xc9, 0x58, 0x14, 0xab, 0x83, 0x0b,
        0xed, 0xed, 0xc7, 0xd2, 0x5e, 0x83, 0xee, 0xf9
};

/*
 * Test Case 11
 *
 * Test vector from https://csrc.nist.gov/csrc/media/projects/
 * cryptographic-standards-and-guidelines/documents/examples/hmac_sha512.pdf
 */
static const uint8_t key_11[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
        0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};
static const uint8_t data_11[] = {
        0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x6d,
        0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x66,
        0x6f, 0x72, 0x20, 0x6b, 0x65, 0x79, 0x6c, 0x65,
        0x6e, 0x3d, 0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x6c,
        0x65, 0x6e
};
static const uint8_t hmac_sha_512_11[] = {
        0xfc, 0x25, 0xe2, 0x40, 0x65, 0x8c, 0xa7, 0x85,
        0xb7, 0xa8, 0x11, 0xa8, 0xd3, 0xf7, 0xb4, 0xca,
        0x48, 0xcf, 0xa2, 0x6a, 0x8a, 0x36, 0x6b, 0xf2,
        0xcd, 0x1f, 0x83, 0x6b, 0x05, 0xfc, 0xb0, 0x24
};

#define HMAC_SHA256_SHA512_TEST_VEC(num)                                \
        { num,                                                          \
                        key_##num, sizeof(key_##num),                   \
                        data_##num, sizeof(data_##num),                 \
                        hmac_sha_224_##num, sizeof(hmac_sha_224_##num), \
                        hmac_sha_256_##num, sizeof(hmac_sha_256_##num), \
                        hmac_sha_384_##num, sizeof(hmac_sha_384_##num), \
                        hmac_sha_512_##num, sizeof(hmac_sha_512_##num) }

#define HMAC_SHA224_TEST_VEC(num)                                       \
        { num,                                                          \
                        key_##num, sizeof(key_##num),                   \
                        data_##num, sizeof(data_##num),                 \
                        hmac_sha_224_##num, sizeof(hmac_sha_224_##num), \
                        NULL, 0,                                        \
                        NULL, 0,                                        \
                        NULL, 0 }

#define HMAC_SHA256_TEST_VEC(num)                                       \
        { num,                                                          \
                        key_##num, sizeof(key_##num),                   \
                        data_##num, sizeof(data_##num),                 \
                        NULL, 0,                                        \
                        hmac_sha_256_##num, sizeof(hmac_sha_256_##num), \
                        NULL, 0,                                        \
                        NULL, 0 }

#define HMAC_SHA384_TEST_VEC(num)                                       \
        { num,                                                          \
                        key_##num, sizeof(key_##num),                   \
                        data_##num, sizeof(data_##num),                 \
                        NULL, 0,                                        \
                        NULL, 0,                                        \
                        hmac_sha_384_##num, sizeof(hmac_sha_384_##num), \
                        NULL, 0 }

#define HMAC_SHA512_TEST_VEC(num)                                       \
        { num,                                                          \
                        key_##num, sizeof(key_##num),                   \
                        data_##num, sizeof(data_##num),                 \
                        NULL, 0,                                        \
                        NULL, 0,                                        \
                        NULL, 0,                                        \
                        hmac_sha_512_##num, sizeof(hmac_sha_512_##num) }

static const struct hmac_rfc4231_vector {
        int test_case_num;
        const uint8_t *key;
        size_t key_len;
        const uint8_t *data;
        size_t data_len;
        const uint8_t *hmac_sha224;
        size_t hmac_sha224_len;
        const uint8_t *hmac_sha256;
        size_t hmac_sha256_len;
        const uint8_t *hmac_sha384;
        size_t hmac_sha384_len;
        const uint8_t *hmac_sha512;
        size_t hmac_sha512_len;
} hmac_sha256_sha512_vectors[] = {
        HMAC_SHA256_SHA512_TEST_VEC(1),
        HMAC_SHA256_SHA512_TEST_VEC(2),
        HMAC_SHA256_SHA512_TEST_VEC(3),
        HMAC_SHA256_SHA512_TEST_VEC(4),
        /* HMAC_SHA256_SHA512_TEST_VEC(5), */
        HMAC_SHA256_SHA512_TEST_VEC(6),
        HMAC_SHA256_SHA512_TEST_VEC(7),
        HMAC_SHA224_TEST_VEC(8),
        HMAC_SHA256_TEST_VEC(9),
        HMAC_SHA384_TEST_VEC(10),
        HMAC_SHA512_TEST_VEC(11),
};

static int
hmac_shax_job_ok(const struct hmac_rfc4231_vector *vec,
                 const struct IMB_JOB *job,
                 const int sha_type,
                 const uint8_t *auth,
                 const uint8_t *padding,
                 const size_t sizeof_padding)
{
        const uint8_t *p_digest = NULL;
        size_t digest_len = 0;

        switch (sha_type) {
        case 224:
                p_digest = vec->hmac_sha224;
                digest_len = vec->hmac_sha224_len;
                break;
        case 256:
                p_digest = vec->hmac_sha256;
                digest_len = vec->hmac_sha256_len;
                break;
        case 384:
                p_digest = vec->hmac_sha384;
                digest_len = vec->hmac_sha384_len;
                break;
        case 512:
                p_digest = vec->hmac_sha512;
                digest_len = vec->hmac_sha512_len;
                break;
        default:
                printf("line:%d wrong SHA type 'SHA-%d' ", __LINE__, sha_type);
                return 0;
                break;
        }

        if (job->status != IMB_STATUS_COMPLETED) {
                printf("line:%d job error status:%d ", __LINE__, job->status);
                return 0;
        }

        /* hash checks */
        if (memcmp(padding, &auth[sizeof_padding + digest_len],
                   sizeof_padding)) {
                printf("hash overwrite tail\n");
                hexdump(stderr, "Target",
                        &auth[sizeof_padding + digest_len],
                        sizeof_padding);
                return 0;
        }

        if (memcmp(padding, &auth[0], sizeof_padding)) {
                printf("hash overwrite head\n");
                hexdump(stderr, "Target", &auth[0], sizeof_padding);
                return 0;
        }

        if (memcmp(p_digest, &auth[sizeof_padding], digest_len)) {
                printf("hash mismatched\n");
                hexdump(stderr, "Received", &auth[sizeof_padding], digest_len);
                hexdump(stderr, "Expected", p_digest, digest_len);
                return 0;
        }
        return 1;
}

static int
test_hmac_shax(struct IMB_MGR *mb_mgr,
               const struct hmac_rfc4231_vector *vec,
               const uint32_t num_jobs,
               const int sha_type)
{
        struct IMB_JOB *job;
        uint8_t padding[16];
        uint8_t **auths = malloc(num_jobs * sizeof(void *));
        uint32_t i = 0, jobs_rx = 0;
        int ret = -1;
        uint8_t key[IMB_SHA_512_BLOCK_SIZE];
        uint8_t buf[IMB_SHA_512_BLOCK_SIZE];
        DECLARE_ALIGNED(uint8_t ipad_hash[IMB_SHA512_DIGEST_SIZE_IN_BYTES], 16);
        DECLARE_ALIGNED(uint8_t opad_hash[IMB_SHA512_DIGEST_SIZE_IN_BYTES], 16);
        uint32_t key_len = 0;
        size_t digest_len = 0;
        size_t block_size = 0;

        if (auths == NULL) {
		fprintf(stderr, "Can't allocate buffer memory\n");
		goto end2;
        }

        switch (sha_type) {
        case 224:
                digest_len = vec->hmac_sha224_len;
                block_size = IMB_SHA_256_BLOCK_SIZE;
                break;
        case 256:
                digest_len = vec->hmac_sha256_len;
                block_size = IMB_SHA_256_BLOCK_SIZE;
                break;
        case 384:
                digest_len = vec->hmac_sha384_len;
                block_size = IMB_SHA_384_BLOCK_SIZE;
                break;
        case 512:
                digest_len = vec->hmac_sha512_len;
                block_size = IMB_SHA_512_BLOCK_SIZE;
                break;
        default:
                fprintf(stderr, "Wrong SHA type selection 'SHA-%d'!\n",
                        sha_type);
                goto end2;
        }

        memset(padding, -1, sizeof(padding));
        memset(auths, 0, num_jobs * sizeof(void *));

        for (i = 0; i < num_jobs; i++) {
                const size_t alloc_len =
                        digest_len + (sizeof(padding) * 2);

                auths[i] = malloc(alloc_len);
                if (auths[i] == NULL) {
                        fprintf(stderr, "Can't allocate buffer memory\n");
                        goto end;
                }
                memset(auths[i], -1, alloc_len);
        }

        /* prepare the key */
        memset(key, 0, sizeof(key));
        if (vec->key_len <= block_size) {
                memcpy(key, vec->key, vec->key_len);
                key_len = (int) vec->key_len;
        } else {
                switch (sha_type) {
                case 224:
                        IMB_SHA224(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA224_DIGEST_SIZE_IN_BYTES;
                        break;
                case 256:
                        IMB_SHA256(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA256_DIGEST_SIZE_IN_BYTES;
                        break;
                case 384:
                        IMB_SHA384(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA384_DIGEST_SIZE_IN_BYTES;
                        break;
                case 512:
                        IMB_SHA512(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA512_DIGEST_SIZE_IN_BYTES;
                        break;
                default:
                        fprintf(stderr, "Wrong SHA type selection 'SHA-%d'!\n",
                                sha_type);
                        goto end;
                }
        }

        /* compute ipad hash */
        memset(buf, 0x36, sizeof(buf));
        for (i = 0; i < key_len; i++)
                buf[i] ^= key[i];

        switch (sha_type) {
        case 224:
                IMB_SHA224_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 256:
                IMB_SHA256_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 384:
                IMB_SHA384_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 512:
        default:
                IMB_SHA512_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        }

        /* compute opad hash */
        memset(buf, 0x5c, sizeof(buf));
        for (i = 0; i < key_len; i++)
                buf[i] ^= key[i];

        switch (sha_type) {
        case 224:
                IMB_SHA224_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 256:
                IMB_SHA256_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 384:
                IMB_SHA384_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 512:
        default:
                IMB_SHA512_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        }

        /* empty the manager */
        while (IMB_FLUSH_JOB(mb_mgr) != NULL)
                ;

        for (i = 0; i < num_jobs; i++) {
                job = IMB_GET_NEXT_JOB(mb_mgr);
                job->enc_keys = NULL;
                job->dec_keys = NULL;
                job->cipher_direction = IMB_DIR_ENCRYPT;
                job->chain_order = IMB_ORDER_HASH_CIPHER;
                job->dst = NULL;
                job->key_len_in_bytes = 0;
                job->auth_tag_output = auths[i] + sizeof(padding);
                job->auth_tag_output_len_in_bytes = digest_len;
                job->iv = NULL;
                job->iv_len_in_bytes = 0;
                job->src = vec->data;
                job->cipher_start_src_offset_in_bytes = 0;
                job->msg_len_to_cipher_in_bytes = 0;
                job->hash_start_src_offset_in_bytes = 0;
                job->msg_len_to_hash_in_bytes = vec->data_len;
                job->u.HMAC._hashed_auth_key_xor_ipad = ipad_hash;
                job->u.HMAC._hashed_auth_key_xor_opad = opad_hash;
                job->cipher_mode = IMB_CIPHER_NULL;

                switch (sha_type) {
                case 224:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_224;
                        break;
                case 256:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_256;
                        break;
                case 384:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_384;
                        break;
                case 512:
                default:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_512;
                        break;
                }

                job->user_data = auths[i];

                job = IMB_SUBMIT_JOB(mb_mgr);
                if (job) {
                        jobs_rx++;
                        /*
                         * SHANI HMAC-SHA implementation can return a completed
                         * job after 2nd submission
                         */
                        if (num_jobs < 2) {
                                printf("%d Unexpected return from submit_job\n",
                                       __LINE__);
                                goto end;
                        }
                        if (!hmac_shax_job_ok(vec, job, sha_type,
                                              job->user_data,
                                              padding, sizeof(padding)))
                                goto end;
                }
        }

        while ((job = IMB_FLUSH_JOB(mb_mgr)) != NULL) {
                jobs_rx++;
                if (!hmac_shax_job_ok(vec, job, sha_type,
                                      job->user_data,
                                      padding, sizeof(padding)))
                        goto end;
        }

        if (jobs_rx != num_jobs) {
                printf("Expected %u jobs, received %u\n", num_jobs, jobs_rx);
                goto end;
        }
        ret = 0;

 end:
        for (i = 0; i < num_jobs; i++) {
                if (auths[i] != NULL)
                        free(auths[i]);
        }

 end2:
        if (auths != NULL)
                free(auths);

        return ret;
}

static int
test_hmac_shax_burst(struct IMB_MGR *mb_mgr,
                     const struct hmac_rfc4231_vector *vec,
                     const uint32_t num_jobs,
                     const int sha_type)
{
        struct IMB_JOB *job, *jobs[max_burst_jobs] = {NULL};
        uint8_t padding[16];
        uint8_t **auths = malloc(num_jobs * sizeof(void *));
        uint32_t i = 0, jobs_rx = 0, completed_jobs = 0;
        int ret = -1, err;
        uint8_t key[IMB_SHA_512_BLOCK_SIZE];
        uint8_t buf[IMB_SHA_512_BLOCK_SIZE];
        DECLARE_ALIGNED(uint8_t ipad_hash[IMB_SHA512_DIGEST_SIZE_IN_BYTES], 16);
        DECLARE_ALIGNED(uint8_t opad_hash[IMB_SHA512_DIGEST_SIZE_IN_BYTES], 16);
        uint32_t key_len = 0;
        size_t digest_len = 0;
        size_t block_size = 0;

        if (auths == NULL) {
		fprintf(stderr, "Can't allocate buffer memory\n");
		goto end2;
        }

        switch (sha_type) {
        case 224:
                digest_len = vec->hmac_sha224_len;
                block_size = IMB_SHA_256_BLOCK_SIZE;
                break;
        case 256:
                digest_len = vec->hmac_sha256_len;
                block_size = IMB_SHA_256_BLOCK_SIZE;
                break;
        case 384:
                digest_len = vec->hmac_sha384_len;
                block_size = IMB_SHA_384_BLOCK_SIZE;
                break;
        case 512:
                digest_len = vec->hmac_sha512_len;
                block_size = IMB_SHA_512_BLOCK_SIZE;
                break;
        default:
                fprintf(stderr, "Wrong SHA type selection 'SHA-%d'!\n",
                        sha_type);
                goto end2;
        }

        memset(padding, -1, sizeof(padding));
        memset(auths, 0, num_jobs * sizeof(void *));

        for (i = 0; i < num_jobs; i++) {
                const size_t alloc_len =
                        digest_len + (sizeof(padding) * 2);

                auths[i] = malloc(alloc_len);
                if (auths[i] == NULL) {
                        fprintf(stderr, "Can't allocate buffer memory\n");
                        goto end;
                }
                memset(auths[i], -1, alloc_len);
        }

        /* prepare the key */
        memset(key, 0, sizeof(key));
        if (vec->key_len <= block_size) {
                memcpy(key, vec->key, vec->key_len);
                key_len = (int) vec->key_len;
        } else {
                switch (sha_type) {
                case 224:
                        IMB_SHA224(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA224_DIGEST_SIZE_IN_BYTES;
                        break;
                case 256:
                        IMB_SHA256(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA256_DIGEST_SIZE_IN_BYTES;
                        break;
                case 384:
                        IMB_SHA384(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA384_DIGEST_SIZE_IN_BYTES;
                        break;
                case 512:
                        IMB_SHA512(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA512_DIGEST_SIZE_IN_BYTES;
                        break;
                default:
                        fprintf(stderr, "Wrong SHA type selection 'SHA-%d'!\n",
                                sha_type);
                        goto end;
                }
        }

        /* compute ipad hash */
        memset(buf, 0x36, sizeof(buf));
        for (i = 0; i < key_len; i++)
                buf[i] ^= key[i];

        switch (sha_type) {
        case 224:
                IMB_SHA224_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 256:
                IMB_SHA256_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 384:
                IMB_SHA384_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 512:
        default:
                IMB_SHA512_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        }

        /* compute opad hash */
        memset(buf, 0x5c, sizeof(buf));
        for (i = 0; i < key_len; i++)
                buf[i] ^= key[i];

        switch (sha_type) {
        case 224:
                IMB_SHA224_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 256:
                IMB_SHA256_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 384:
                IMB_SHA384_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 512:
        default:
                IMB_SHA512_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        }

        while (IMB_GET_NEXT_BURST(mb_mgr, num_jobs, jobs) < num_jobs)
                IMB_FLUSH_BURST(mb_mgr, num_jobs, jobs);

        for (i = 0; i < num_jobs; i++) {
                job = jobs[i];
                job->enc_keys = NULL;
                job->dec_keys = NULL;
                job->cipher_direction = IMB_DIR_ENCRYPT;
                job->chain_order = IMB_ORDER_HASH_CIPHER;
                job->dst = NULL;
                job->key_len_in_bytes = 0;
                job->auth_tag_output = auths[i] + sizeof(padding);
                job->auth_tag_output_len_in_bytes = digest_len;
                job->iv = NULL;
                job->iv_len_in_bytes = 0;
                job->src = vec->data;
                job->cipher_start_src_offset_in_bytes = 0;
                job->msg_len_to_cipher_in_bytes = 0;
                job->hash_start_src_offset_in_bytes = 0;
                job->msg_len_to_hash_in_bytes = vec->data_len;
                job->u.HMAC._hashed_auth_key_xor_ipad = ipad_hash;
                job->u.HMAC._hashed_auth_key_xor_opad = opad_hash;
                job->cipher_mode = IMB_CIPHER_NULL;

                switch (sha_type) {
                case 224:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_224;
                        break;
                case 256:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_256;
                        break;
                case 384:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_384;
                        break;
                case 512:
                default:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_512;
                        break;
                }

                job->user_data = auths[i];

        }

        completed_jobs = IMB_SUBMIT_BURST(mb_mgr, num_jobs, jobs);
        err = imb_get_errno(mb_mgr);

        if (err != 0) {
                printf("submit_burst error %d : '%s'\n", err,
                       imb_get_strerror(err));
                goto end;
        }

 check_burst_jobs:
        for (i = 0; i < completed_jobs; i++) {
                job = jobs[i];

                if (job->status != IMB_STATUS_COMPLETED) {
                        printf("job %u status not complete!\n", i+1);
                        goto end;
                }

                if (!hmac_shax_job_ok(vec, job, sha_type,
                                      job->user_data,
                                      padding, sizeof(padding)))
                        goto end;
                jobs_rx++;
        }

        if (jobs_rx != num_jobs) {
                completed_jobs = IMB_FLUSH_BURST(mb_mgr,
                                                 num_jobs - completed_jobs,
                                                 jobs);
                if (completed_jobs == 0) {
                        printf("Expected %u jobs, received %u\n",
                               num_jobs, jobs_rx);
                        goto end;
                }
                goto check_burst_jobs;
        }
       ret = 0;

 end:
        for (i = 0; i < num_jobs; i++) {
                if (auths[i] != NULL)
                        free(auths[i]);
        }

 end2:
        if (auths != NULL)
                free(auths);

        return ret;
}

static int
test_hmac_shax_hash_burst(struct IMB_MGR *mb_mgr,
                          const struct hmac_rfc4231_vector *vec,
                          const uint32_t num_jobs,
                          const int sha_type)
{
        struct IMB_JOB *job, jobs[max_burst_jobs] = {0};
        uint8_t padding[16];
        uint8_t **auths = NULL;
        uint32_t i = 0, jobs_rx = 0, completed_jobs = 0;
        int ret = -1;
        uint8_t key[IMB_SHA_512_BLOCK_SIZE];
        uint8_t buf[IMB_SHA_512_BLOCK_SIZE];
        DECLARE_ALIGNED(uint8_t ipad_hash[IMB_SHA512_DIGEST_SIZE_IN_BYTES], 16);
        DECLARE_ALIGNED(uint8_t opad_hash[IMB_SHA512_DIGEST_SIZE_IN_BYTES], 16);
        uint32_t key_len = 0;
        size_t digest_len = 0;
        size_t block_size = 0;

        if (num_jobs == 0)
                return 0;

        auths = malloc(num_jobs * sizeof(void *));
        if (auths == NULL) {
		fprintf(stderr, "Can't allocate buffer memory\n");
		goto end2;
        }

        switch (sha_type) {
        case 224:
                digest_len = vec->hmac_sha224_len;
                block_size = IMB_SHA_256_BLOCK_SIZE;
                break;
        case 256:
                digest_len = vec->hmac_sha256_len;
                block_size = IMB_SHA_256_BLOCK_SIZE;
                break;
        case 384:
                digest_len = vec->hmac_sha384_len;
                block_size = IMB_SHA_384_BLOCK_SIZE;
                break;
        case 512:
                digest_len = vec->hmac_sha512_len;
                block_size = IMB_SHA_512_BLOCK_SIZE;
                break;
        default:
                fprintf(stderr, "Wrong SHA type selection 'SHA-%d'!\n",
                        sha_type);
                goto end2;
        }

        memset(padding, -1, sizeof(padding));
        memset(auths, 0, num_jobs * sizeof(void *));

        for (i = 0; i < num_jobs; i++) {
                const size_t alloc_len =
                        digest_len + (sizeof(padding) * 2);

                auths[i] = malloc(alloc_len);
                if (auths[i] == NULL) {
                        fprintf(stderr, "Can't allocate buffer memory\n");
                        goto end;
                }
                memset(auths[i], -1, alloc_len);
        }

        /* prepare the key */
        memset(key, 0, sizeof(key));
        if (vec->key_len <= block_size) {
                memcpy(key, vec->key, vec->key_len);
                key_len = (int) vec->key_len;
        } else {
                switch (sha_type) {
                case 224:
                        IMB_SHA224(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA224_DIGEST_SIZE_IN_BYTES;
                        break;
                case 256:
                        IMB_SHA256(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA256_DIGEST_SIZE_IN_BYTES;
                        break;
                case 384:
                        IMB_SHA384(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA384_DIGEST_SIZE_IN_BYTES;
                        break;
                case 512:
                        IMB_SHA512(mb_mgr, vec->key, vec->key_len, key);
                        key_len = IMB_SHA512_DIGEST_SIZE_IN_BYTES;
                        break;
                default:
                        fprintf(stderr, "Wrong SHA type selection 'SHA-%d'!\n",
                                sha_type);
                        goto end;
                }
        }

        /* compute ipad hash */
        memset(buf, 0x36, sizeof(buf));
        for (i = 0; i < key_len; i++)
                buf[i] ^= key[i];

        switch (sha_type) {
        case 224:
                IMB_SHA224_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 256:
                IMB_SHA256_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 384:
                IMB_SHA384_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        case 512:
        default:
                IMB_SHA512_ONE_BLOCK(mb_mgr, buf, ipad_hash);
                break;
        }

        /* compute opad hash */
        memset(buf, 0x5c, sizeof(buf));
        for (i = 0; i < key_len; i++)
                buf[i] ^= key[i];

        switch (sha_type) {
        case 224:
                IMB_SHA224_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 256:
                IMB_SHA256_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 384:
                IMB_SHA384_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        case 512:
        default:
                IMB_SHA512_ONE_BLOCK(mb_mgr, buf, opad_hash);
                break;
        }

        for (i = 0; i < num_jobs; i++) {
                job = &jobs[i];
                job->enc_keys = NULL;
                job->dec_keys = NULL;
                job->cipher_direction = IMB_DIR_ENCRYPT;
                job->chain_order = IMB_ORDER_HASH_CIPHER;
                job->dst = NULL;
                job->key_len_in_bytes = 0;
                job->auth_tag_output = auths[i] + sizeof(padding);
                job->auth_tag_output_len_in_bytes = digest_len;
                job->iv = NULL;
                job->iv_len_in_bytes = 0;
                job->src = vec->data;
                job->cipher_start_src_offset_in_bytes = 0;
                job->msg_len_to_cipher_in_bytes = 0;
                job->hash_start_src_offset_in_bytes = 0;
                job->msg_len_to_hash_in_bytes = vec->data_len;
                job->u.HMAC._hashed_auth_key_xor_ipad = ipad_hash;
                job->u.HMAC._hashed_auth_key_xor_opad = opad_hash;
                job->cipher_mode = IMB_CIPHER_NULL;

                switch (sha_type) {
                case 224:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_224;
                        break;
                case 256:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_256;
                        break;
                case 384:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_384;
                        break;
                case 512:
                default:
                        job->hash_alg = IMB_AUTH_HMAC_SHA_512;
                        break;
                }

                job->user_data = auths[i];

        }

        completed_jobs = IMB_SUBMIT_HASH_BURST(mb_mgr, jobs, num_jobs,
                                               job->hash_alg);
        if (completed_jobs != num_jobs) {
                int err = imb_get_errno(mb_mgr);

                if (err != 0) {
                        printf("submit_burst error %d : '%s'\n", err,
                               imb_get_strerror(err));
                        goto end;
                } else {
                        printf("submit_burst error: not enough "
                               "jobs returned!\n");
                        goto end;
                }
        }

        for (i = 0; i < num_jobs; i++) {
                job = &jobs[i];

                if (job->status != IMB_STATUS_COMPLETED) {
                        printf("job %u status not complete!\n", i+1);
                        goto end;
                }

                if (!hmac_shax_job_ok(vec, job, sha_type,
                                      job->user_data,
                                      padding, sizeof(padding)))
                        goto end;
                jobs_rx++;
        }

        if (jobs_rx != num_jobs) {
                printf("Expected %u jobs, received %u\n", num_jobs, jobs_rx);
                goto end;
        }
        ret = 0;

 end:
        for (i = 0; i < num_jobs; i++) {
                if (auths[i] != NULL)
                        free(auths[i]);
        }

 end2:
        if (auths != NULL)
                free(auths);

        return ret;
}

static void
test_hmac_shax_std_vectors(struct IMB_MGR *mb_mgr,
                           const int sha_type,
                           const uint32_t num_jobs,
                           struct test_suite_context *ts)
{
	const int vectors_cnt = DIM(hmac_sha256_sha512_vectors);
	int vect;

	printf("HMAC-SHA%d standard test vectors (N jobs = %u):\n",
               sha_type, num_jobs);
	for (vect = 1; vect <= vectors_cnt; vect++) {
                const int idx = vect - 1;
                const int flag = (sha_type == 224 &&
                        hmac_sha256_sha512_vectors[idx].hmac_sha224 == NULL) ||
                        (sha_type == 256 &&
                         hmac_sha256_sha512_vectors[idx].hmac_sha256 == NULL) ||
                        (sha_type == 384 &&
                         hmac_sha256_sha512_vectors[idx].hmac_sha384 == NULL) ||
                        (sha_type == 512 &&
                         hmac_sha256_sha512_vectors[idx].hmac_sha512 == NULL);
#ifdef DEBUG
		printf("[%d/%d] RFC4231 Test Case %d key_len:%d data_len:%d\n",
                       vect, vectors_cnt,
                       hmac_sha256_sha512_vectors[idx].test_case_num,
                       (int) hmac_sha256_sha512_vectors[idx].key_len,
                       (int) hmac_sha256_sha512_vectors[idx].data_len);
#else
		printf(".");
#endif

                if (flag) {
#ifdef DEBUG
                        printf("Skipped vector %d, N/A for HMAC-SHA%d\n",
                               vect, sha_type);
#endif
                        continue;
                }

                if (test_hmac_shax(mb_mgr, &hmac_sha256_sha512_vectors[idx],
                                   num_jobs, sha_type)) {
                        printf("error #%d\n", vect);
                        test_suite_update(ts, 0, 1);
                } else {
                        test_suite_update(ts, 1, 0);
                }
                if (test_hmac_shax_burst(mb_mgr,
                                         &hmac_sha256_sha512_vectors[idx],
                                         num_jobs, sha_type)) {
                        printf("error #%d - burst API\n", vect);
                        test_suite_update(ts, 0, 1);
                } else {
                        test_suite_update(ts, 1, 0);
                }
                if (test_hmac_shax_hash_burst(mb_mgr,
                                              &hmac_sha256_sha512_vectors[idx],
                                              num_jobs, sha_type)) {
                        printf("error #%d - hash-only burst API\n", vect);
                        test_suite_update(ts, 0, 1);
                } else {
                        test_suite_update(ts, 1, 0);
                }

        }
	printf("\n");
}

int
hmac_sha256_sha512_test(struct IMB_MGR *mb_mgr)
{
        const int sha_types_tab[] = {
                224, 256, 384, 512
        };
        static const char * const sha_names_tab[] = {
                "HMAC-SHA224", "HMAC-SHA256", "HMAC-SHA384", "HMAC-SHA512"
        };
        unsigned i, num_jobs;
        int errors = 0;

        for (i = 0; i < DIM(sha_types_tab); i++) {
                struct test_suite_context ts;

                test_suite_start(&ts, sha_names_tab[i]);
                for (num_jobs = 1; num_jobs <= max_burst_jobs; num_jobs++)
                        test_hmac_shax_std_vectors(mb_mgr, sha_types_tab[i],
                                                   num_jobs, &ts);
                errors += test_suite_end(&ts);
        }

	return errors;
}
