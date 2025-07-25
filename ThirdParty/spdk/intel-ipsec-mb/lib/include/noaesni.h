/*******************************************************************************
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
*******************************************************************************/

#include "intel-ipsec-mb.h"

#ifndef NOAESNI_H
#define NOAESNI_H

IMB_DLL_EXPORT void init_mb_mgr_sse_no_aesni(IMB_MGR *state);
IMB_DLL_EXPORT IMB_JOB *submit_job_sse_no_aesni(IMB_MGR *state);
IMB_DLL_EXPORT IMB_JOB *submit_job_nocheck_sse_no_aesni(IMB_MGR *state);
IMB_DLL_EXPORT IMB_JOB *flush_job_sse_no_aesni(IMB_MGR *state);
IMB_DLL_EXPORT uint32_t queue_size_sse_no_aesni(IMB_MGR *state);
IMB_DLL_EXPORT IMB_JOB *get_completed_job_sse_no_aesni(IMB_MGR *state);
IMB_DLL_EXPORT IMB_JOB *get_next_job_sse_no_aesni(IMB_MGR *state);

IMB_DLL_EXPORT uint32_t
get_next_burst_sse_no_aesni(IMB_MGR *state, const uint32_t n_jobs,
                            IMB_JOB **jobs);
IMB_DLL_EXPORT uint32_t
submit_burst_sse_no_aesni(IMB_MGR *state, const uint32_t n_jobs,
                          IMB_JOB **jobs);
IMB_DLL_EXPORT uint32_t
submit_burst_nocheck_sse_no_aesni(IMB_MGR *state, const uint32_t n_jobs,
                                  IMB_JOB **jobs);
IMB_DLL_EXPORT uint32_t
flush_burst_sse_no_aesni(IMB_MGR *state, const uint32_t max_jobs,
                         IMB_JOB **jobs);

IMB_DLL_EXPORT uint32_t
submit_cipher_burst_sse_no_aesni(IMB_MGR *state, IMB_JOB *jobs,
                                 const uint32_t n_jobs,
                                 const IMB_CIPHER_MODE cipher,
                                 const IMB_CIPHER_DIRECTION dir,
                                 const IMB_KEY_SIZE_BYTES key_size);
IMB_DLL_EXPORT uint32_t
submit_cipher_burst_nocheck_sse_no_aesni(IMB_MGR *state, IMB_JOB *jobs,
                                         const uint32_t n_jobs,
                                         const IMB_CIPHER_MODE cipher,
                                         const IMB_CIPHER_DIRECTION dir,
                                         const IMB_KEY_SIZE_BYTES key_size);
IMB_DLL_EXPORT uint32_t
submit_hash_burst_sse_no_aesni(IMB_MGR *state, IMB_JOB *jobs,
                               const uint32_t n_jobs,
                               const IMB_HASH_ALG hash);
IMB_DLL_EXPORT uint32_t
submit_hash_burst_nocheck_sse_no_aesni(IMB_MGR *state, IMB_JOB *jobs,
                                       const uint32_t n_jobs,
                                       const IMB_HASH_ALG hash);
IMB_DLL_EXPORT void
aes_keyexp_128_sse_no_aesni(const void *key, void *enc_exp_keys,
                            void *dec_exp_keys);
IMB_DLL_EXPORT void
aes_keyexp_192_sse_no_aesni(const void *key, void *enc_exp_keys,
                            void *dec_exp_keys);
IMB_DLL_EXPORT void
aes_keyexp_256_sse_no_aesni(const void *key, void *enc_exp_keys,
                            void *dec_exp_keys);
IMB_DLL_EXPORT void
aes_xcbc_expand_key_sse_no_aesni(const void *key, void *k1_exp, void *k2,
                                 void *k3);
IMB_DLL_EXPORT void
aes_keyexp_128_enc_sse_no_aesni(const void *key, void *enc_exp_keys);
IMB_DLL_EXPORT void
aes_keyexp_192_enc_sse_no_aesni(const void *key, void *enc_exp_keys);
IMB_DLL_EXPORT void
aes_keyexp_256_enc_sse_no_aesni(const void *key, void *enc_exp_keys);
IMB_DLL_EXPORT void
aes_cmac_subkey_gen_sse_no_aesni(const void *key_exp, void *key1, void *key2);
IMB_DLL_EXPORT void
aes_cmac_256_subkey_gen_sse_no_aesni(const void *key_exp,
                                     void *key1, void *key2);
IMB_DLL_EXPORT void
aes_cfb_128_one_sse_no_aesni(void *out, const void *in, const void *iv,
                             const void *keys, uint64_t len);

#endif /* NOAESNI_H */
