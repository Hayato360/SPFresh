;;
;; Copyright (c) 2017-2022, Intel Corporation
;;
;; Redistribution and use in source and binary forms, with or without
;; modification, are permitted provided that the following conditions are met:
;;
;;     * Redistributions of source code must retain the above copyright notice,
;;       this list of conditions and the following disclaimer.
;;     * Redistributions in binary form must reproduce the above copyright
;;       notice, this list of conditions and the following disclaimer in the
;;       documentation and/or other materials provided with the distribution.
;;     * Neither the name of Intel Corporation nor the names of its contributors
;;       may be used to endorse or promote products derived from this software
;;       without specific prior written permission.
;;
;; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
;; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
;; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
;; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
;; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
;; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
;; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;;

;; In System V AMD64 ABI
;;	callee saves: RBX, RBP, R12-R15
;; Windows x64 ABI
;;	callee saves: RBX, RBP, RDI, RSI, RSP, R12-R15
;;
;; Registers:		RAX RBX RCX RDX RBP RSI RDI R8  R9  R10 R11 R12 R13 R14 R15
;;			-----------------------------------------------------------
;; Windows clobbers:	RAX     RCX RDX             R8  R9  R10 R11
;; Windows preserves:	    RBX         RBP RSI RDI                 R12 R13 R14 R15
;;			-----------------------------------------------------------
;; Linux clobbers:	RAX     RCX RDX     RSI RDI R8  R9  R10 R11
;; Linux preserves:	    RBX         RBP                         R12 R13 R14 R15
;;			-----------------------------------------------------------
;; Clobbers ZMM0-31

%include "include/os.asm"
%include "include/imb_job.asm"
%include "include/mb_mgr_datastruct.asm"
%include "include/reg_sizes.asm"
%include "include/memcpy.asm"
%include "include/const.inc"
%include "include/clear_regs.asm"
;; %define DO_DBGPRINT
%include "include/dbgprint.asm"

%use smartalign

extern sha256_x16_avx512

mksection .rodata
default rel

align 16
byteswap:
	dq 0x0405060700010203, 0x0c0d0e0f08090a0b

mksection .text

%ifdef LINUX
%define arg1	rdi
%define arg2	rsi
%define arg3	rcx
%define arg4	rdx
%else
%define arg1	rcx
%define arg2	rdx
%define arg3	rdi
%define arg4	rsi
%endif

%define state	arg1
%define job	arg2
%define len2	arg2

; idx needs to be in rbp, r15
%define last_len	rbp
%define idx		rbp

%define p		r11
%define start_offset	r11

%define unused_lanes	rbx
%define p2		rbx
%define tmp4		rbx

%define job_rax		rax
%define len		rax

%define size_offset	arg3
%define tmp2		arg3

%define lane		arg4
%define tmp3		arg4

%define extra_blocks	r8
%define tmp		r9
%define lane_data	r10

%define len_upper	r13
%define idx_upper	r14

; we clobber rbx, rsi, rdi, rbp; called routine also clobbers r9 to r15
struc STACK
_gpr_save:	resq	8
_rsp_save:	resq	1
endstruc

; JOB* FUNC(MB_MGR_HMAC_SHA_256_OOO *state, IMB_JOB *job)
; arg 1 : rcx : state
; arg 2 : rdx : job
	align 32
%ifdef SHA224
MKGLOBAL(submit_job_hmac_sha_224_avx512,function,internal)
submit_job_hmac_sha_224_avx512:
%else
MKGLOBAL(submit_job_hmac_sha_256_avx512,function,internal)
submit_job_hmac_sha_256_avx512:
%endif
	mov	rax, rsp
	sub	rsp, STACK_size
	and	rsp, -32
	mov	[rsp + _gpr_save + 8*0], rbx
	mov	[rsp + _gpr_save + 8*1], rbp
	mov	[rsp + _gpr_save + 8*2], r12
	mov	[rsp + _gpr_save + 8*3], r13
	mov	[rsp + _gpr_save + 8*4], r14
	mov	[rsp + _gpr_save + 8*5], r15
%ifndef LINUX
	mov	[rsp + _gpr_save + 8*6], rsi
	mov	[rsp + _gpr_save + 8*7], rdi
%endif
	mov	[rsp + _rsp_save], rax	; original SP

	mov	unused_lanes, [state + _unused_lanes_sha256]
	mov	lane, unused_lanes
	and	lane, 0xF	    ;; just a nibble
	shr	unused_lanes, 4

	imul	lane_data, lane, _HMAC_SHA1_LANE_DATA_size
	lea	lane_data, [state + _ldata_sha256 + lane_data]
	mov	[state + _unused_lanes_sha256], unused_lanes

	add	dword [state + _num_lanes_inuse_sha256], 1

	mov	len, [job + _msg_len_to_hash_in_bytes]
	mov	tmp, len
	shr	tmp, 6	; divide by 64, len in terms of blocks

	mov	[lane_data + _job_in_lane], job
	mov	dword [lane_data + _outer_done], 0
        VPINSRW_M256 state + _lens_sha256, xmm0, xmm1, p, lane, tmp, scale_x16

	mov	last_len, len
	and	last_len, 63
	lea	extra_blocks, [last_len + 9 + 63]
	shr	extra_blocks, 6
	mov	[lane_data + _extra_blocks], DWORD(extra_blocks)

	mov	p, [job + _src]
	add	p, [job + _hash_start_src_offset_in_bytes]
	mov	[state + _args_data_ptr_sha256 + PTR_SZ*lane], p

	cmp	len, 64
	jb	copy_lt64

fast_copy:
	vmovdqu32	zmm0, [p - 64 + len]
	vmovdqu32	[lane_data + _extra_block], zmm0
end_fast_copy:

	mov	size_offset, extra_blocks
	shl	size_offset, 6
	sub	size_offset, last_len
	add	size_offset, 64-8
	mov	[lane_data + _size_offset], DWORD(size_offset)
	mov	start_offset, 64
	sub	start_offset, last_len
	mov	[lane_data + _start_offset], DWORD(start_offset)

	lea	tmp, [8*64 + 8*len]
	bswap	tmp
	mov	[lane_data + _extra_block + size_offset], tmp

	mov	tmp, [job + _auth_key_xor_ipad]
	vmovdqu	xmm0, [tmp]
	vmovdqu	xmm1, [tmp + 4*4]
	vmovd	[state + _args_digest_sha256 + 4*lane + 0*SHA256_DIGEST_ROW_SIZE], xmm0
	vpextrd	[state + _args_digest_sha256 + 4*lane + 1*SHA256_DIGEST_ROW_SIZE], xmm0, 1
	vpextrd	[state + _args_digest_sha256 + 4*lane + 2*SHA256_DIGEST_ROW_SIZE], xmm0, 2
	vpextrd	[state + _args_digest_sha256 + 4*lane + 3*SHA256_DIGEST_ROW_SIZE], xmm0, 3
	vmovd	[state + _args_digest_sha256 + 4*lane + 4*SHA256_DIGEST_ROW_SIZE], xmm1
	vpextrd	[state + _args_digest_sha256 + 4*lane + 5*SHA256_DIGEST_ROW_SIZE], xmm1, 1
	vpextrd	[state + _args_digest_sha256 + 4*lane + 6*SHA256_DIGEST_ROW_SIZE], xmm1, 2
	vpextrd	[state + _args_digest_sha256 + 4*lane + 7*SHA256_DIGEST_ROW_SIZE], xmm1, 3

	test	len, ~63
	jnz	ge64_bytes

lt64_bytes:
        VPINSRW_M256 state + _lens_sha256, xmm0, xmm1, tmp, lane, extra_blocks, scale_x16
	lea	tmp, [lane_data + _extra_block + start_offset]
	mov	[state + _args_data_ptr_sha256 + PTR_SZ*lane], tmp
	mov	dword [lane_data + _extra_blocks], 0

ge64_bytes:
        cmp	dword [state + _num_lanes_inuse_sha256], 0x10 ; all 16 lanes used?
	jne	return_null

	align	16
start_loop:
	; Find min length
	vmovdqa	xmm0, [state + _lens_sha256]
	vphminposuw	xmm1, xmm0
	vpextrw	DWORD(len2), xmm1, 0	; min value
	vpextrw	DWORD(idx), xmm1, 1	; min index (0...7)

	vmovdqa	xmm2, [state + _lens_sha256 + 8*2]
	vphminposuw	xmm3, xmm2
	vpextrw	DWORD(len_upper), xmm3, 0	; min value
	vpextrw	DWORD(idx_upper), xmm3, 1	; min index (8...F)

	cmp	len2, len_upper
	jle	use_min

	vmovdqa	xmm1, xmm3
	mov	len2, len_upper
	mov	idx, idx_upper	; idx is in range 0..7
	add	idx, 8		; to reflect that real index is in 8..F range
use_min:
	cmp	len2, 0
	je	len_is_0

	vpbroadcastw	xmm1, xmm1 ; duplicate words across all lanes
	vpsubw	xmm0, xmm0, xmm1
	vmovdqa	[state + _lens_sha256 + 0*2], xmm0
	vpsubw	xmm2, xmm2, xmm1
	vmovdqa	[state + _lens_sha256 + 8*2], xmm2

	; "state" and "args" are the same address, arg1
	; len is arg2
	call	sha256_x16_avx512
	; state and idx are intact

len_is_0:
	; process completed job "idx"
	imul	lane_data, idx, _HMAC_SHA1_LANE_DATA_size
	lea	lane_data, [state + _ldata_sha256 + lane_data]
	mov	DWORD(extra_blocks), [lane_data + _extra_blocks]
	cmp	extra_blocks, 0
	jne	proc_extra_blocks
	cmp	dword [lane_data + _outer_done], 0
	jne	end_loop

proc_outer:
	mov	dword [lane_data + _outer_done], 1
	mov	DWORD(size_offset), [lane_data + _size_offset]
	mov	qword [lane_data + _extra_block + size_offset], 0
        VPINSRW_M256 state + _lens_sha256, xmm0, xmm1, tmp, idx, 1, scale_x16
	lea	tmp, [lane_data + _outer_block]
	mov	job, [lane_data + _job_in_lane]
	mov	[state + _args_data_ptr_sha256 + PTR_SZ*idx], tmp

	vmovd	xmm0, [state + _args_digest_sha256 + 4*idx + 0*SHA256_DIGEST_ROW_SIZE]
	vpinsrd	xmm0, xmm0, [state + _args_digest_sha256 + 4*idx + 1*SHA256_DIGEST_ROW_SIZE], 1
	vpinsrd	xmm0, xmm0, [state + _args_digest_sha256 + 4*idx + 2*SHA256_DIGEST_ROW_SIZE], 2
	vpinsrd	xmm0, xmm0, [state + _args_digest_sha256 + 4*idx + 3*SHA256_DIGEST_ROW_SIZE], 3
	vpshufb	xmm0, xmm0, [rel byteswap]
	vmovd	xmm1, [state + _args_digest_sha256 + 4*idx + 4*SHA256_DIGEST_ROW_SIZE]
	vpinsrd	xmm1, xmm1, [state + _args_digest_sha256 + 4*idx + 5*SHA256_DIGEST_ROW_SIZE], 1
	vpinsrd	xmm1, xmm1, [state + _args_digest_sha256 + 4*idx + 6*SHA256_DIGEST_ROW_SIZE], 2
%ifndef SHA224
	vpinsrd	xmm1, xmm1, [state + _args_digest_sha256 + 4*idx + 7*SHA256_DIGEST_ROW_SIZE], 3
%endif
	vpshufb	xmm1, xmm1, [rel byteswap]
	vmovdqa	[lane_data + _outer_block], xmm0
	vmovdqa	[lane_data + _outer_block + 4*4], xmm1
%ifdef SHA224
	mov	dword [lane_data + _outer_block + 7*4], 0x80
%endif

	mov	tmp, [job + _auth_key_xor_opad]
	vmovdqu	xmm0, [tmp]
	vmovdqu	xmm1, [tmp + 4*4]
	vmovd	[state + _args_digest_sha256 + 4*idx + 0*SHA256_DIGEST_ROW_SIZE], xmm0
	vpextrd	[state + _args_digest_sha256 + 4*idx + 1*SHA256_DIGEST_ROW_SIZE], xmm0, 1
	vpextrd	[state + _args_digest_sha256 + 4*idx + 2*SHA256_DIGEST_ROW_SIZE], xmm0, 2
	vpextrd	[state + _args_digest_sha256 + 4*idx + 3*SHA256_DIGEST_ROW_SIZE], xmm0, 3
	vmovd	[state + _args_digest_sha256 + 4*idx + 4*SHA256_DIGEST_ROW_SIZE], xmm1
	vpextrd	[state + _args_digest_sha256 + 4*idx + 5*SHA256_DIGEST_ROW_SIZE], xmm1, 1
	vpextrd	[state + _args_digest_sha256 + 4*idx + 6*SHA256_DIGEST_ROW_SIZE], xmm1, 2
	vpextrd	[state + _args_digest_sha256 + 4*idx + 7*SHA256_DIGEST_ROW_SIZE], xmm1, 3

	jmp	start_loop

	align	16
proc_extra_blocks:
	mov	DWORD(start_offset), [lane_data + _start_offset]
        VPINSRW_M256 state + _lens_sha256, xmm0, xmm1, tmp, idx, extra_blocks, scale_x16
	lea	tmp, [lane_data + _extra_block + start_offset]
	mov	[state + _args_data_ptr_sha256 + 8*idx], tmp
	mov	dword [lane_data + _extra_blocks], 0
	jmp	start_loop

	align 16
copy_lt64:
	;; less than one message block of data
	;; beginning of source block
	;; destination extrablock but backwards by len from where 0x80 pre-populated
	lea	p2, [lane_data + _extra_block  + 64]
	sub	p2, len
	memcpy_avx2_64_1 p2, p, len, tmp, tmp2, ymm0, ymm1
	mov	unused_lanes, [state + _unused_lanes_sha256]
	jmp	end_fast_copy

return_null:
	xor	job_rax, job_rax
	jmp	return

	align	16
end_loop:
	mov	job_rax, [lane_data + _job_in_lane]
	mov	unused_lanes, [state + _unused_lanes_sha256]
	mov	qword [lane_data + _job_in_lane], 0
	or	dword [job_rax + _status], IMB_STATUS_COMPLETED_AUTH
	shl	unused_lanes, 4
	or	unused_lanes, idx
	mov	[state + _unused_lanes_sha256], unused_lanes
	sub	dword [state + _num_lanes_inuse_sha256], 1

	mov	p, [job_rax + _auth_tag_output]

%ifdef SHA224
        cmp     qword [job_rax + _auth_tag_output_len_in_bytes], 14
        jne     copy_full_digest
%else
        cmp     qword [job_rax + _auth_tag_output_len_in_bytes], 16
        jne     copy_full_digest
%endif

	;; copy 14 bytes for SHA224 // 16 bytes for SHA256
	mov	DWORD(tmp),  [state + _args_digest_sha256 + 4*idx + 0*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp2), [state + _args_digest_sha256 + 4*idx + 1*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp3), [state + _args_digest_sha256 + 4*idx + 2*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp4), [state + _args_digest_sha256 + 4*idx + 3*SHA256_DIGEST_ROW_SIZE]
	bswap	DWORD(tmp)
	bswap	DWORD(tmp2)
	bswap	DWORD(tmp3)
	bswap	DWORD(tmp4)
	mov	[p + 0*4], DWORD(tmp)
	mov	[p + 1*4], DWORD(tmp2)
	mov	[p + 2*4], DWORD(tmp3)
%ifdef SHA224
	mov	[p + 3*4], WORD(tmp4)
%else
	mov	[p + 3*4], DWORD(tmp4)
%endif
        jmp     clear_ret
copy_full_digest:
	;; copy 28 bytes for SHA224 // 32 bytes for SHA256
	mov	DWORD(tmp),  [state + _args_digest_sha256 + 4*idx + 0*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp2), [state + _args_digest_sha256 + 4*idx + 1*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp3), [state + _args_digest_sha256 + 4*idx + 2*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp4), [state + _args_digest_sha256 + 4*idx + 3*SHA256_DIGEST_ROW_SIZE]
	bswap	DWORD(tmp)
	bswap	DWORD(tmp2)
	bswap	DWORD(tmp3)
	bswap	DWORD(tmp4)
	mov	[p + 0*4], DWORD(tmp)
	mov	[p + 1*4], DWORD(tmp2)
	mov	[p + 2*4], DWORD(tmp3)
	mov	[p + 3*4], DWORD(tmp4)

	mov	DWORD(tmp),  [state + _args_digest_sha256 + 4*idx + 4*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp2), [state + _args_digest_sha256 + 4*idx + 5*SHA256_DIGEST_ROW_SIZE]
	mov	DWORD(tmp3), [state + _args_digest_sha256 + 4*idx + 6*SHA256_DIGEST_ROW_SIZE]
%ifndef SHA224
	mov	DWORD(tmp4), [state + _args_digest_sha256 + 4*idx + 7*SHA256_DIGEST_ROW_SIZE]
%endif
	bswap	DWORD(tmp)
	bswap	DWORD(tmp2)
	bswap	DWORD(tmp3)
%ifndef SHA224
	bswap	DWORD(tmp4)
%endif
	mov	[p + 4*4], DWORD(tmp)
	mov	[p + 5*4], DWORD(tmp2)
	mov	[p + 6*4], DWORD(tmp3)
%ifndef SHA224
	mov	[p + 7*4], DWORD(tmp4)
%endif

clear_ret:

%ifdef SAFE_DATA
        ;; Clear digest (28B/32B), outer_block (28B/32B) and extra_block (64B) of returned job
%assign J 0
%rep 7
        mov     dword [state + _args_digest_sha256 + SHA256_DIGEST_WORD_SIZE*idx + J*SHA256_DIGEST_ROW_SIZE], 0
%assign J (J+1)
%endrep
%ifndef SHA224
        mov     dword [state + _args_digest_sha256 + SHA256_DIGEST_WORD_SIZE*idx + 7*SHA256_DIGEST_ROW_SIZE], 0
%endif

        vpxorq  zmm0, zmm0
        imul	lane_data, idx, _HMAC_SHA1_LANE_DATA_size
        lea	lane_data, [state + _ldata_sha256 + lane_data]
        ;; Clear first 64 bytes of extra_block
        vmovdqu64  [lane_data + _extra_block], zmm0

        ;; Clear first 28 bytes (SHA-224) or 32 bytes (SHA-256) of outer_block
%ifdef SHA224
        vmovdqa64 [lane_data + _outer_block], xmm0
        mov     qword [lane_data + _outer_block + 16], 0
        mov     dword [lane_data + _outer_block + 24], 0
%else
        vmovdqu64 [lane_data + _outer_block], ymm0
%endif
%endif ;; SAFE_DATA

return:
%ifdef SAFE_DATA
        clear_all_zmms_asm
%else
        vzeroupper
%endif

	mov	rbx, [rsp + _gpr_save + 8*0]
	mov	rbp, [rsp + _gpr_save + 8*1]
	mov	r12, [rsp + _gpr_save + 8*2]
	mov	r13, [rsp + _gpr_save + 8*3]
	mov	r14, [rsp + _gpr_save + 8*4]
	mov	r15, [rsp + _gpr_save + 8*5]
%ifndef LINUX
	mov	rsi, [rsp + _gpr_save + 8*6]
	mov	rdi, [rsp + _gpr_save + 8*7]
%endif
	mov	rsp, [rsp + _rsp_save]	; original SP

	ret

mksection stack-noexec
