TASK_SWITCH


Problema: Si el proceso al que se cambia, nunca ha realizado un task_switch, al hacer el pop %ebp y ret dara error



entry.S:

ENTRY(task_switch)

        pushl %edi; \
        pushl %esi; \
        pushl %ebx; \

        //move parameter to top of stack
        pushl 4(%ebp)

        call inner_task_switch

        //discard parameter
        addl $4, %esp;

        popl %ebx; \
        popl %esi; \
        popl %edi; \

        ret






sched.c:

void inner_task_switch(union task_union *t) {

        __asm__ __volatile__ (
                "pushl %%ebp \n\t"
                "movl  %%esp, %%ebp"
                :
                :
                : "%ebp", "%esp"
        );

        set_cr3(get_DIR(&(t->task)));

        change_TSS_EBP(t);

        current()->kernel_esp = ebp_value();

        void* tmp = (void*)t->task.kernel_esp;
        __asm__ __volatile__(
                "movl %0, %%esp\n\t"
                "popl %%ebp"
		:
                : "r" (tmp)
                : "memory"
        );

}













GETPID()




sys_call_table.S:

ENTRY (sys_call_table)
        .long sys_ni_syscall // 0
        .long sys_ni_syscall // 1
        .long sys_ni_syscall // 2
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

ENTRY(getpid)
        movl $20, %eax           //sys_call_table entry for getpid is 20
        int $0x80
        cmpl $0, %eax
        jge exit
        neg %eax
        movl %eax, errno
        movl $-1, %eax
exit:
        ret




sys.h:

int sys_getpid();

wrappers.h:

int getpid();








FORK()

sys_call_table.S:

ENTRY (sys_call_table)
        .long sys_ni_syscall // 0
        .long sys_ni_syscall // 1
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


wrapper.S:

ENTRY(fork)
        movl $2, %eax           //sys_call_table entry for fork is 2
        int $0x80
        cmpl $0, %eax
        jge exit
        neg %eax
        movl %eax, errno
        movl $-1, %eax
exit:
        ret


sys.c:

int sys_fork()
{
	int PID=-1;
	if(list_empty(&freeq)) return PID;
	struct task_struct* t;
        struct list_head* t = list_first(&freeq);
        list_del(t);
        t_new =  list_entry(t, struct task_struct, anchor);
	

	return PID;
}

wrapper.h:

int fork();

sys.h:

int sys_fork();
