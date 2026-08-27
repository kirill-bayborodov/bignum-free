; =============================================================================
; bignum_free.asm
; =============================================================================
; @file bignum_free.asm
; @brief x86-64 YASM implementation of bignum_free.
; @details The function validates RDI, then clears the complete bignum_t
; representation with a fixed 33-qword REP STOSQ sequence. bignum_t contains
; BIGNUM_CAPACITY words followed by size_t len, so the operation clears 32 word
; slots plus the length field. It does not release the object storage.
;
; System V AMD64 ABI contract:
;   RDI: caller-owned writable bignum_t pointer
;   RAX: bignum_free_status_t return value
;   RCX/RDI/RFLAGS: caller-saved and clobbered by REP STOSQ
;   No stack frame is required; no callee-saved register is modified.
;   The object is treated as a contiguous 264-byte representation.
; =============================================================================

BITS 64
DEFAULT REL

%define BIGNUM_CAPACITY 32
%define QWORDS_TO_CLEAR (BIGNUM_CAPACITY + 1)
%define SUCCESS 0
%define ERROR_NULL_ARG -1

global bignum_free
section .text

bignum_free:
    xor     eax, eax
    test    rdi, rdi
    jz      .error_null_arg

    ; Clear all words and len. The fixed count covers sizeof(bignum_t).
    mov     ecx, QWORDS_TO_CLEAR
    rep     stosq
    ret

.error_null_arg:
    mov     eax, ERROR_NULL_ARG
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
