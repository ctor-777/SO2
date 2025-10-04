#include <libc.h>
#include <wrappers.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
	int i;

	buff [0] = 'a';
	buff [1] = 0;
    
	while(1) { 
		itoa(15, buff);
		write(1, "itoa fine: \n", 12);
		write(1, buff , 1);
		write(1, "a", 1);
	}
}
