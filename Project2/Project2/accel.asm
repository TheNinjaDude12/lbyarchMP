bits 64
default rel

section .data
kmh_to_ms: dq 0.2777777777777778     

section .text
global compute_accel

; void compute_accel(const double *mat, int *out, long long n);
; rcx = mat, rdx = out, r8 = n   (Windows x64 ABI)
compute_accel:
    push rbx
    push rsi
    push rdi
    mov rdi, rcx          ; rdi = mat pointer  
    mov rsi, rdx          ; rsi = out pointer
    mov rdx, r8           ; rdx = n
    xor rcx, rcx          ; rcx = i = 0
    test rdx, rdx
    jle .done
    movsd xmm4, [kmh_to_ms]

.loop:
    mov rax, rcx
    imul rax, rax, 24         ; each row = 3 doubles = 24 bytes
    lea rbx, [rdi + rax]      ; rbx -> &mat[i][0]  (Vi)
    movsd xmm1, [rbx]         ; xmm1 = Vi
    movsd xmm2, [rbx + 8]     ; xmm2 = Vf
    movsd xmm3, [rbx + 16]    ; xmm3 = T
    subsd xmm2, xmm1          ; xmm2 = Vf - Vi          (km/h)
    mulsd xmm2, xmm4          ; xmm2 = (Vf - Vi) * 1000/3600   (m/s)
    divsd xmm2, xmm3          ; xmm2 = acceleration (m/s^2)
    cvtsd2si eax, xmm2        ; round to nearest int
    mov [rsi + rcx*4], eax    ; out[i] = eax
    inc rcx
    cmp rcx, rdx
    jl .loop

.done:
    pop rdi
    pop rsi
    pop rbx
    ret