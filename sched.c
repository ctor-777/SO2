/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include "types.h"
#include <sched.h>
#include <list.h>
#include <mm.h>
#include <io.h>

#include <libc.h>

struct list_head readyq;
struct list_head freeq;

struct task_struct* idle_task;

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
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}


void init_idle (void)
{

	union task_union* idle_union = &(task[0]);
	idle_task = (struct task_struct*) idle_union;


	idle_task->PID = 0;
	idle_task->kernel_esp = (DWord)&(idle_union->stack[KERNEL_STACK_SIZE-2]);
	idle_task->quantum = 1;

	allocate_DIR(idle_task);

	idle_union->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&cpu_idle;
	idle_union->stack[KERNEL_STACK_SIZE-2] = (unsigned long)&(idle_union->stack[KERNEL_STACK_SIZE-1]);


	// set_cr3(get_DIR(idle_task));
}

void init_task1(void)
{

	union task_union* init_union = &(task[1]);

	struct task_struct* init_task = (struct task_struct*)init_union;


	init_task->PID = 1;
	init_task->kernel_esp = (DWord)&(init_union->stack[KERNEL_STACK_SIZE-2]);
	init_task->quantum = 100;
	quantum_ticks = 100;


	allocate_DIR(init_task);
	set_user_pages(init_task);

	set_cr3(get_DIR(init_task));
}


void init_sched()
{
    INIT_LIST_HEAD( &readyq );
    INIT_LIST_HEAD( &freeq );

	//fill free queue with all procesess
	for(int i = 2; i < NR_TASKS; i++) {
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

int quantum_ticks;

void update_sched_data_rr() {
	--quantum_ticks;
}
void sched_next_rr() {

	struct task_struct* next;

	if (list_empty(&readyq)) {
		next = idle_task;
	} else {
		struct list_head* next_l = list_first(&readyq);
		next = list_entry(next_l, struct task_struct, anchor);
		update_process_state_rr(next, NULL);
	}

	char buff[3];
	itoa(next->PID, buff);
	printk(buff);
	printk("\n");

	task_switch((union task_union*)next);
}

//1 if:
//	- quantum_ticks < 0 and readyq not empty
//	- TODO current exited
int needs_sched_rr() {
	if (list_empty(&readyq))
		quantum_ticks = current()->quantum;

	if (quantum_ticks < 0)
		return 1;
	else 
		return 0;
}

//TODO if t exited do not add to dest
void update_process_state_rr(struct task_struct *t, struct list_head *dest) {
	struct list_head* t_elem = &(t->anchor);

	//if t in queue -> delete it
	if(!(t_elem->next == NULL && t_elem->prev == NULL))
		list_del(t_elem);

	//if dest not NULL -> add t to dest
	if(dest) 
		list_add_tail(t_elem, dest);
}

void scheduler() {
	update_sched_data_rr();

	if (needs_sched_rr()) {
		update_process_state_rr(current(), &readyq);
		printk("\nscheudled next task PID: ");
		sched_next_rr();
	} else {
		// printk("\nsheduled same task\n");
	}
}

void inner_task_switch(union task_union *t) {

	set_cr3(get_DIR(&(t->task)));

	tss.esp0 = t->task.kernel_esp;

	current()->kernel_esp = ebp_value();

	asm volatile (
		"movl %0, %%esp\n\t"
		"popl %%ebp\n\t"
		"ret"
		:
		: "r" (t->task.kernel_esp)
		: "memory"
	);
}

int last_pid = 1;

int new_pid() {
	last_pid++;
	return last_pid;
}

void ret_from_fork()
{
	__asm__ __volatile__(
		"movl $0, %eax"
	);
}
