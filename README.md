


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
	struct task_struct *t = current();
	struct list_head *pos, *n;
	struct task_struct *tmp;
	if(t->parent != NULL) list_del(t->child_anchor);
 	list_for_each_safe(pos, n, t->childs)
 	{
 		tmp= list_entry(pos, struct task_struct, child_anchor);
		tmp->parent=idle_task;
		list_del(pos);
		list_add_tail(pos, idle_task->childs);
 	}


	free_user_pages(t);
	list_add_tail(t->anchor, &freeq);
	sched_next_rr();
}


sys.h:

void sys_exit();


















BLOCK():




sys_call_table.S:

ENTRY (sys_call_table)
        .long sys_ni_syscall // 0
        .long sys_exit // 1
        .long sys_fork // 2
        .long sys_ni_syscall // 3
        .long sys_write // 4
        .long sys_block // 5
        .long sys_unblock // 6
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

ENTRY(block)
        movl $5, %eax           //sys_call_table entry for block is 5
        int $0x80
        cmpl $0, %eax
        jge exit
        neg %eax
        movl %eax, errno
        movl $-1, %eax
exit:
        ret

ENTRY(unblock)
	movl 4(%esp), %ebx     //pid
        movl $6, %eax           //sys_call_table entry for unblock is 6
        int $0x80
        cmpl $0, %eax
        jge exit
        neg %eax
        movl %eax, errno
        movl $-1, %eax
exit:
        ret


wrappers.h:

void block();
int unblock(int pid);


sys.c:

void sys_block()
{
	task task_struct* t= current();
	if(t->pending_unblocks) t->pending_unblocks--;
	else
	{
 		list_add_tail(t->anchor, &blocked);
 		sched_next_rr();
	}
}

int sys_unblock(int pid)
{
	struct list_head *pos;
	struct task_struct *tmp;
	struct task_struct *child=NULL;

	list_for_each(pos, current()->childs)
	{
		tmp= list_entry(pos, struct task_struct, child_anchor);
		if(tmp->PID==pid)
		{
			child=tmp;
			break;
		}
	}
	if(child==NULL) return -1;
	
	int trobat=0;
	list_for_each(pos, &blocked)
	{
 		if(child->anchor==pos) trobat=1;
		
	}
	if(!trobat) child->pending_unblock++;
	else
	{
		list_del(child->anchor);
		list_add_tail(child->anchor, &readyq);
	}
	return 0;
}	


sys.h:

void sys_block();
int sys_unblock(int pid);











struct task_struct {
        int PID;                        /* Process ID. This MUST be the first field of the struct. */
        struct list_head anchor;
        Word kernel_esp;
        page_table_entry * dir_pages_baseAddr;

	struct task_struct parent*		//Puntero al padre
	struct list_head child_anchor;		//list_head para la lista de hijos del padre
	struct list_head childs;		//Lista de hijos de este proceso
	int pending_unblocks;
};
