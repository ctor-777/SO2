#include "list.h"
#include <utils.h>
#include <types.h>
#include <sched.h>
#include <io.h>
#include <libc.h>

#include <mm_address.h>

void copy_data(void *start, void *dest, int size)
{
  DWord *p = start, *q = dest;
  Byte *p1, *q1;
  while(size > 4) {
    *q++ = *p++;
    size -= 4;
  }
  p1=(Byte*)p;
  q1=(Byte*)q;
  while(size > 0) {
    *q1++ = *p1++;
    size --;
  }
}
/* Copia de espacio de usuario a espacio de kernel, devuelve 0 si ok y -1 si error*/
int copy_from_user(void *start, void *dest, int size)
{
  DWord *p = start, *q = dest;
  Byte *p1, *q1;
  while(size > 4) {
    *q++ = *p++;
    size -= 4;
  }
  p1=(Byte*)p;
  q1=(Byte*)q;
  while(size > 0) {
    *q1++ = *p1++;
    size --;
  }
  return 0;
}
/* Copia de espacio de kernel a espacio de usuario, devuelve 0 si ok y -1 si error*/
int copy_to_user(void *start, void *dest, int size)
{
  DWord *p = start, *q = dest;
  Byte *p1, *q1;
  while(size > 4) {
    *q++ = *p++;
    size -= 4;
  }
  p1=(Byte*)p;
  q1=(Byte*)q;
  while(size > 0) {
    *q1++ = *p1++;
    size --;
  }
  return 0;
}

/* access_ok: Checks if a user space pointer is valid
 * @type:  Type of access: %VERIFY_READ or %VERIFY_WRITE. Note that
 *         %VERIFY_WRITE is a superset of %VERIFY_READ: if it is safe
 *         to write to a block, it is always safe to read from it
 * @addr:  User space pointer to start of block to check
 * @size:  Size of block to check
 * Returns true (nonzero) if the memory block may be valid,
 *         false (zero) if it is definitely invalid
 */
int access_ok(int type, const void * addr, unsigned long size)
{
  unsigned long addr_ini, addr_fin;

  addr_ini=(((unsigned long)addr)>>12);
  addr_fin=((((unsigned long)addr)+size)>>12);
  if (addr_fin < addr_ini) return 0; //This looks like an overflow ... deny access

  switch(type)
  {
    case VERIFY_WRITE:
      /* Should suppose no support for automodifyable code */
      if ((addr_ini>=USER_FIRST_PAGE)&&
          (addr_fin<=USER_FIRST_PAGE+NUM_PAG_DATA))
	  return 1;
    default:
      if ((addr_ini>=USER_FIRST_PAGE)&&
  	(addr_fin<=(USER_FIRST_PAGE+NUM_PAG_CODE+NUM_PAG_DATA)))
          return 1;
  }
  return 0;
}

void itoa_hex(int a, char* b) {
		int i, i1;
		char c;
		char hex[17] = "0123456789ABCDEF";
		
		
		if (a==0) { b[0]='0'; b[1]=0; return ;}
		
		i=0;
		while (a>0)
		{
				b[i]=hex[a%16];
				a=a/16;
				i++;
		}
		
		for (i1=0; i1<i/2; i1++)
		{
				c=b[i1];
				b[i1]=b[i-i1-1];
				b[i-i1-1]=c;
		}
		b[i]=0;
}

void print_stack(union task_union* t_u) {
	unsigned long * stack_base = (unsigned long *)&(t_u->stack[KERNEL_STACK_SIZE - 1]);
	unsigned long * stack_top = (unsigned long *)t_u->task.kernel_esp - 1;
	 
	if (t_u == (union task_union*) current()) {
		__asm__ __inline__ (
			"movl %%esp, %0"
			: "=a"(stack_top)
			: 
			:
			);
	}

	int i = (stack_base - stack_top);
	int aux = 50;

	char buff[9];

	if (stack_top > stack_base)
		return;
	while(i > 0 && aux > 0) {
		printk("\n");
		itoa(i, buff);
		printk(buff);
		itoa_hex((int)stack_base, buff);
		printk(" (");
		printk(buff);
		printk(")");
		printk(": [");
		itoa_hex(*stack_base, buff);
		printk(buff);
		printk("]");

		stack_base--;
		i--;
		aux--;
	}
}

void print_queue(struct list_head* head) {
	struct list_head* pos;
	struct task_struct* pos_task_s;

	char buff[9];
	list_for_each(pos, head) {
		pos_task_s = list_entry(pos, struct task_struct, anchor);

		int pos_task = ((int)pos_task_s-(int)task)/sizeof(union task_union);	
		
		itoa(pos_task, buff);
		printk("\n");
		printk(buff);
		printk(": PID ");

		itoa(pos_task_s->PID, buff);
		printk(buff);
	}
}

#define CYCLESPERTICK 109000

/*
 * do_div() is NOT a C function. It wants to return
 * two values (the quotient and the remainder), but
 * since that doesn't work very well in C, what it
 * does is:
 *
 * - modifies the 64-bit dividend _in_place_
 * - returns the 32-bit remainder
 *
 * This ends up being the most efficient "calling
 * convention" on x86.
 */
#define do_div(n,base) ({ \
        unsigned long __upper, __low, __high, __mod, __base; \
        __base = (base); \
        asm("":"=a" (__low), "=d" (__high):"A" (n)); \
        __upper = __high; \
        if (__high) { \
                __upper = __high % (__base); \
                __high = __high / (__base); \
        } \
        asm("divl %2":"=a" (__low), "=d" (__mod):"rm" (__base), "0" (__low), "1" (__upper)); \
        asm("":"=A" (n):"a" (__low),"d" (__high)); \
        __mod; \
})


#define rdtsc(low,high) \
        __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

unsigned long get_ticks(void) {
        unsigned long eax;
        unsigned long edx;
        unsigned long long ticks;

        rdtsc(eax,edx);

        ticks=((unsigned long long) edx << 32) + eax;
        do_div(ticks,CYCLESPERTICK);

        return ticks;
}
