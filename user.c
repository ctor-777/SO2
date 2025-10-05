#include <libc.h>
#include <wrappers.h>
#include <errno.h>

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
		// i = write(1, "\nwrite: ", 8);
		// itoa(i, buff);
		// write(1, buff , strlen(buff));

		// i = write(1, "hello", 5);
		i = failed_syscall();
		if (i < 0) {
			write(1, "\nError: ", 7);
			
			perror();
		}
	}
}
