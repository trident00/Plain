section .text
global _main

_main:
    push rax                    ; Save RAX on the stack
    mov dword [rsp + 4], 2     ; Store 2 at RSP + 4
    mov dword [rsp], 5         ; Store 5 at RSP
    mov eax, [rsp]             ; Load value from RSP into EAX
    pop rcx                    ; Restore RAX into RCX
    ret                         ; Return from the function
