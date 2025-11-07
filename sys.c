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

	struct list_head* t_head = list_first(&freeq);
	list_del(t_head);

	struct task_struct* t = list_entry(t_head, struct task_struct, anchor);

	// union task_union* t_union;
	copy_data(current(), t, 4096);
	allocate_DIR(t);

	int pag;
	int new_ph_pag;
	page_table_entry * process_PT_new =  get_PT(t);
	page_table_entry * process_PT_current =  get_PT(current());

	//NO USAMOS set_user_pages PORQUE EN LAS INSTRUCCIONES NOS PIDEN USAR ALLOC_FRAME() 
	//Y ADEMAS PORQUE NECESITAMOS SABER CUANDO NOS DEVUELVE ERROR POR FALTA DE PAGINAS DISPONIBLES Y	
	//SOLO QUEREMOS RESERVAR PAGIAS DE DATA (CODIGO LO COMPARTIMOS CON EL PADRE)

	/* CODE */
	for (pag=0;pag<NUM_PAG_CODE;pag++){
		process_PT_new[PAG_LOG_INIT_CODE+pag]=process_PT_current[PAG_LOG_INIT_CODE+pag];
	}

	/* DATA */
	unsigned temp_pag = 0xA00000 >> 12;	//<------------ Necesitamos una pagina temporal para copiar los datos del proceso 1 al 2 (como las 20 paginas de data van antes
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
		copy_data((void*)temp_pag,(void*)new_ph_pag,4096);
		del_ss_pag(process_PT_current, temp_pag);
	}

	//PID
	int pos;
	pos = ((int)t-(int)task)/sizeof(union task_union);
	PID=new_pid();
	while(PID==pos) PID=new_pid();
	t->PID=PID;
	
	void* curr_sys_esp = (void*)current()->kernel_esp;
	void* new_sys_esp = (void*)t->kernel_esp;

	__asm__ __volatile__(
		"movl %0, %%esp\n\t"
		"addl $4, %%esp\n\t"
		"push ret_from_fork\n\t"
		"push 0"
		:
		: "r" (new_sys_esp)
		: "memory"
	);

	__asm__ __volatile__(
		"movl %0, %%esp\n\t"
		:
		: "r" (curr_sys_esp)
		: "memory"
	);

	list_add_tail(t_head, &readyq);
	return PID;
}
