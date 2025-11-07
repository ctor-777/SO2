#include <asm.h>

ENTRY(write)
	pushl %ebx
	movl 8(%esp), %edx	//fd
	movl 12(%esp), %ecx	//buffer
	movl 16(%esp), %ebx	//size
	movl $4, %eax		//sys_call_table entry for write is 4
	int $0x80
	cmpl $0, %eax
	jge exit_wr
	neg %eax
	movl %eax, errno
	movl $-1, %eax
exit_wr:
	popl %ebx
	ret

ENTRY(gettime)
	movl $10, %eax		//sys_call_table entry for gettime is 10
	int $0x80
	ret


ENTRY(getpid)
	movl $20, %eax           //sys_call_table entry for getpid is 20
	int $0x80
	cmpl $0, %eax
	jge exit_pid
	neg %eax
	movl %eax, errno
	movl $-1, %eax
exit_pid:
	ret

ENTRY(fork)
	movl $2, %eax           //sys_call_table entry for fork is 2
	int $0x80
	cmpl $0, %eax
	jge exit_fork
	neg %eax
	movl %eax, errno
	movl $-1, %eax
exit_fork:
	ret

ENTRY(exit)
  	movl $1, %eax           //sys_call_table entry for exit is 1
   	int $0x80
   	cmpl $0, %eax
   	jge exit_exit
   	neg %eax
   	movl %eax, errno
   	movl $-1, %eax
exit_exit:
  	ret


ENTRY(block)
	movl $5, %eax           //sys_call_table entry for block is 5
	int $0x80
  	cmpl $0, %eax
 	jge exit_block
   	neg %eax
   	movl %eax, errno
   	movl $-1, %eax
exit_block:
 	ret

ENTRY(unblock)
	movl 4(%esp), %ebx     //pid
    movl $6, %eax           //sys_call_table entry for unblock is 6
   	int $0x80
   	cmpl $0, %eax
   	jge exit_unblock
   	neg %eax
   	movl %eax, errno
   	movl $-1, %eax
exit_unblock:
   	ret
