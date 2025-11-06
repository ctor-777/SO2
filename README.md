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
        struct list_head* t_head = list_first(&freeq);
        list_del(t_head);
	
	union task_union* t_union;
	copy_data(current(), t, 4096);
	allocate_dir(t);
		
 	int pag;
 	int new_ph_pag;
 	page_table_entry * process_PT_new =  get_PT(t);
	page_table_entry * process_PT_current =  get_PT(current());
	
	//NO USAMOS set_user_pages PORQUE EN LAS INSTRUCCIONES NOS PIDEN USAR ALLOC_FRAME() Y ADEMAS PORQUE NECESITAMOS SABER CUANDO NOS DEVUELVE ERROR POR FALTA DE PAGINAS DISPONIBLES Y 	//SOLO QUEREMOS RESERVAR PAGIAS DE DATA (CODIGO LO COMPARTIMOS CON EL PADRE)

  	/* CODE */
  	for (pag=0;pag<NUM_PAG_CODE;pag++){
  	      process_PT_new[PAG_LOG_INIT_CODE+pag]=process_PT_current[PAG_LOG_INIT_CODE+pag];
  	}


  	/* DATA */
	unsigned temp_page = 0xA00000 >> 12;		    <------------ Necesitamos una pagina temporal para copiar los datos del proceso 1 al 2 (como las 20 paginas de data van antes
  	for (pag=0;pag<NUM_PAG_DATA;pag++){				  que las paginas de code, si usabamos una pagina numero 21 de data, reescribiriamos una pagina de code(dir valida?)
        	new_ph_pag=alloc_frame();
 		if(new_ph_pag==-1)                          <------------ Devuelve error si no ha podido reservar la pagina
		(
            		for (int i = 0; i < pag; i++) {
                		free_frame(process_PT_new[PAG_LOG_INIT_DATA + i].bits.pbase_addr);
                		process_PT_new[PAG_LOG_INIT_DATA + i].entry = 0;
        		}
			return PID;
		)
        	process_PT_new[PAG_LOG_INIT_DATA+pag].entry = 0;
        	process_PT_new[PAG_LOG_INIT_DATA+pag].bits.pbase_addr = new_ph_pag;
        	process_PT_new[PAG_LOG_INIT_DATA+pag].bits.user = 1;
        	process_PT_new[PAG_LOG_INIT_DATA+pag].bits.rw = 1;
        	process_PT_new[PAG_LOG_INIT_DATA+pag].bits.present = 1;
		
		set_ss_pag(process_PT_current, temp_page, new_ph_pag);
		copy_data(temp,new_ph_pag,4096);
		del_ss_page(process_PT_current, temp_page);
  	}

	//PID
	int pos;
        pos = ((int)t-(int)task)/sizeof(union task_union);
	PID=new_pid();
	while(PID==pos) PID=new_pid();
	t->PID=PID;
	
	void* tmp_curr = (void*)current().kernel_esp;
        void* tmp = (void*)t.kernel_esp;
        __asm__ __volatile__(
                "movl %0, %%esp\n\t"
		"addl $4, %%esp\n\t"
		"push ret_from_fork\n\t"
		"push 0"
 		:
                : "r" (tmp)
                : "memory"
        );

	void* tmp = (void*)t.kernel_esp;
        __asm__ __volatile__(
                "movl %0, %%esp\n\t"
 		:
                : "r" (tmp_curr)
                : "memory"
        );


	list_add_tail(t_head, &readyq);
	return PID;
}


wrapper.h:

int fork();

sys.h:

int sys_fork();

sched.c:

int new_pid() {return last_pid++;}

void ret_from_fork()
{
       __asm__ __volatile__(
 		"movl $0, 0x24(%esp)"	<------ Establecer el %eax guardado en la pila a 0 (se devuelve PID 0 para el hijo)
        );

}


sched.h:

int last_pid=1;
int new_pid();
void ret_from_fork();



