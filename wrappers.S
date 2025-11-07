#include <asm.h>

ENTRY(write)
	movl 4(%esp), %edx	//fd
	movl 8(%esp), %ecx	//buffer
	movl 12(%esp), %ebx	//size
	movl $4, %eax		//sys_call_table entry for write is 4
	int $0x80
	cmpl $0, %eax
	jge exit_wr
	neg %eax
	movl %eax, errno
	movl $-1, %eax
exit_wr:
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

