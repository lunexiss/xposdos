global idt_load

idt_load:
    mov eax, [esp + 4] ; get pointer to idt_ptr
    lidt [eax]
    ret
