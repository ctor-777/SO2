/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <list.h>
#include <mm.h>
#include <io.h>

struct list_head readyq;
struct list_head freeq;

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	printk("\nin idle...");
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{

	struct list_head* idle = list_first(&freeq);
	list_del(idle);
	struct task_struct* idle_task =  list_entry(idle, struct task_struct, anchor);

	idle_task->PID = 0;

	allocate_DIR(idle_task);

	union task_union* idle_union = (union task_union*) &idle_task;

	idle_union->stack[KERNEL_STACK_SIZE-1] = (unsigned int)&cpu_idle;
	idle_union->stack[KERNEL_STACK_SIZE-2] =  0;


	set_cr3(get_DIR(idle_task));

	list_add_tail(idle, &readyq);
}

void init_task1(void)
{

	struct list_head* init = list_first(&freeq);
	list_del(init);
	struct task_struct* init_task =  list_entry(init, struct task_struct, anchor);

	init_task->PID = 1;

	allocate_DIR(init_task);
	set_user_pages(init_task);


	set_cr3(get_DIR(init_task));

	list_add_tail(init, &readyq);
}


void init_sched()
{

    INIT_LIST_HEAD( &readyq );
    INIT_LIST_HEAD( &freeq );


	//fill free queue with all procesess
	for(int i = 0; i < NR_TASKS; i++) {
		list_add_tail(&(task[i].task.anchor), &freeq);
	}

}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void scheduler() {
}

void inner_task_switch(union task_union *t) {
	
	set_cr3(get_DIR(t));

	change_TSS_EBP(t);

	current()->kernel_esp = ebp_value();

	void* tmp = (void*)t->task.kernel_esp;
	__asm__ __volatile__(
		"movl %0, %%esp"
		:
		: "r" (tmp)
		: "memory"
	);
	
}
