    .section .text._start,"ax",@progbits
    .globl main
    .globl p9_test_user_entry
main:
p9_test_user_entry:
    movl test_data(%rip), %eax
    call p9_fixture_value
    call p9_fixture_extra
1:  jmp 1b
    .section .data,"aw",@progbits
    .globl test_data
test_data:
    .long 0x12345678
    .section .note.GNU-stack,"",@progbits

    .section .text,"ax",@progbits
    .globl exits
exits:
1:  jmp 1b
