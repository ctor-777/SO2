/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

extern int quantum_ticks;

extern struct list_head readyq;
extern struct list_head freeq;

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
	int PID;			/* Process ID. This MUST be the first field of the struct. */
	struct list_head anchor;	
	DWord kernel_esp;
	enum state_t state;
	unsigned int quantum;
	page_table_entry* dir_pages_baseAddr;
	struct task_struct* parent;		//Puntero al padre
	struct list_head child_anchor;		//list_head para la lista de hijos del padre
	struct list_head childs;		//Lista de hijos de este proceso
	int pending_unblocks;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */

extern struct task_struct* idle_task;

extern struct list_head blocked;

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

//context switch
void task_switch(union task_union*t);
DWord ebp_value();

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

void scheduler();

//fork and getpid
extern int last_pid;

int new_pid();
void ret_from_fork();

#endif  /* __SCHED_H__ */
