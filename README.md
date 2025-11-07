EXIT():



sys_call_table.S:

ENTRY (sys_call_table)
        .long sys_ni_syscall // 0
        .long sys_exit // 1
        .long sys_fork // 2
        .long sys_ni_syscall // 3
        .long sys_write // 4
        .long sys_ni_syscall // 5
        .long sys_ni_syscall // 6
        .long sys_ni_syscall // 7
        .long sys_ni_syscall // 8
        .long sys_ni_syscall // 9
        .long sys_gettime // 10
        .long sys_ni_syscall // 11
        .long sys_ni_syscall // 12
        .long sys_ni_syscall // 13
        .long sys_ni_syscall // 14
        .long sys_ni_syscall // 15
        .long sys_ni_syscall // 16
        .long sys_ni_syscall // 17
        .long sys_ni_syscall // 18
        .long sys_ni_syscall // 19
 	.long sys_getpid // 20



wrappers.S:

ENTRY(exit)
        movl $1, %eax           //sys_call_table entry for exit is 1
        int $0x80
        cmpl $0, %eax
        jge exit
        neg %eax
        movl %eax, errno
        movl $-1, %eax
exit:
        ret



wrappers.h:

int exit();



sys.c:

void sys_exit()
{
	free_user_pages(current());
	//set_cr3(get_DIR(current()));
	list_add_tail(t->anchor, &freeq);
	sched_next_rr();
}


sys.h:

void sys_exit();
