/*
 * sys.c - Syscalls implementation
 */
#include "list.h"
#include <libc.h>
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <sys.h>

#include <interrupt.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
	if (fd!=1) return -9; /*EBADF*/
	if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
	return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_write(int fd, char * buffer, int size)
{
	char buff[size];

	int valor=check_fd(fd, ESCRIPTURA);
	if (valor != 0)
		return valor;

	if (size <= 0 || buffer == NULL)
		return -22; //EINVAL
	
	if (!access_ok(VERIFY_READ, buffer, size)) 
		return -13; //EFAULT


	copy_from_user(buffer, buff, size);
	valor=sys_write_console(buff, size);


	return valor;
}

int sys_gettime() {
	return zeos_ticks;
}

int sys_getpid()
{
	return current()->PID;
}

void sys_exit()
{  
}

int sys_fork()
{
	int PID=-1;
	if(list_empty(&freeq)) 
		return PID;

	struct list_head* new_h = list_first(&freeq);
	list_del(new_h);

	struct task_struct* new_s = list_entry(new_h, struct task_struct, anchor);
	union task_union* new_u = (union task_union*)new_s;

	copy_data(current(), new_s, 4096);
	allocate_DIR(new_s);

	int pag;
	int new_ph_pag;
	page_table_entry * process_PT_new =  get_PT(new_s);
	page_table_entry * process_PT_current =  get_PT(current());

	//NO USAMOS set_user_pages PORQUE EN LAS INSTRUCCIONES NOS PIDEN USAR ALLOC_FRAME() 
	//Y ADEMAS PORQUE NECESITAMOS SABER CUANDO NOS DEVUELVE ERROR POR FALTA DE PAGINAS DISPONIBLES Y	
	//SOLO QUEREMOS RESERVAR PAGIAS DE DATA (CODIGO LO COMPARTIMOS CON EL PADRE)

	/* CODE */
	for (pag=0;pag<NUM_PAG_CODE;pag++){
		process_PT_new[PAG_LOG_INIT_CODE+pag]=process_PT_current[PAG_LOG_INIT_CODE+pag];
	}

	/* DATA */
	unsigned temp_pag = 0x3FF000 >> 12;	//<------------ Necesitamos una pagina temporal para copiar los datos del proceso 1 al 2 (como las 20 paginas de data van antes
	for (pag=0;pag<NUM_PAG_DATA;pag++){		 // que las paginas de code, si usabamos una pagina numero 21 de data, reescribiriamos una pagina de code(dir valida?)
		new_ph_pag=alloc_frame();
		if(new_ph_pag==-1) {				   //<------------ Devuelve error si no ha podido reservar la pagina
			for (int i = 0; i < pag; i++) {
				free_frame(process_PT_new[PAG_LOG_INIT_DATA + i].bits.pbase_addr);
				process_PT_new[PAG_LOG_INIT_DATA + i].entry = 0;
			}
			return PID;
		}
		process_PT_new[PAG_LOG_INIT_DATA+pag].entry = 0;
		process_PT_new[PAG_LOG_INIT_DATA+pag].bits.pbase_addr = new_ph_pag;
		process_PT_new[PAG_LOG_INIT_DATA+pag].bits.user = 1;
		process_PT_new[PAG_LOG_INIT_DATA+pag].bits.rw = 1;
		process_PT_new[PAG_LOG_INIT_DATA+pag].bits.present = 1;
		
		set_ss_pag(process_PT_current, temp_pag, new_ph_pag);
		// set_cr3(get_DIR(current()));
		copy_data((void*)((PAG_LOG_INIT_DATA + pag) << 12),(void*)(temp_pag << 12),4096);
		del_ss_pag(process_PT_current, temp_pag);
	}

	//PID
	int pos;
	pos = ((int)new_s-(int)task)/sizeof(union task_union);
	PID=new_pid();
	while(PID==pos) PID=new_pid();
	new_s->PID=PID;
	
	// void* curr_sys_ebp = (void*)current()->kernel_esp;
	unsigned long* curr_sys_ebp = (void *)ebp_value();
	int ebp_diff = (&( (union task_union*)current() )->stack[KERNEL_STACK_SIZE-1]) - (curr_sys_ebp - 1);

	new_s->kernel_esp = (DWord)(( &( new_u )->stack[KERNEL_STACK_SIZE-1] ) - ebp_diff - 1);

	new_u->stack[KERNEL_STACK_SIZE - ebp_diff] = (unsigned long)&ret_from_fork;

	new_u->stack[KERNEL_STACK_SIZE - ebp_diff - 1] = (unsigned long)0;

	list_add_tail(new_h, &readyq);
	return PID;
}


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


void sys_block()
{
	struct task_struct* t= current();
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
 		if(child->anchor==pos) 
		{
			trobat=1;
			break;
		}
	}
	if(!trobat) child->pending_unblocks++;
	else
	{
		list_del(child->anchor);
		list_add_tail(child->anchor, &readyq);
	}
	return 0;
}

