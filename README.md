GETPID()


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
