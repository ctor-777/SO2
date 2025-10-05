#include <asm.h>

ENTRY(write)
	movl 4(%esp), %edx	//fd
	movl 8(%esp), %ecx	//buffer
	movl 12(%esp), %ebx	//size
	movl $4, %eax		//sys_call_table entry for write is 4
	int $0x80
	cmpl $0, %eax
	jge exit
	neg %eax
	movl %eax, errno
	movl $-1, %eax
exit:
	ret

ENTRY(gettime)
	movl $10, %eax		//sys_call_table entry for gettime is 10
	int $0x80
	ret
