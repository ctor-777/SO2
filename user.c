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

	
	write(1, "\nin user...", 11);

	int pid = fork();

	if (pid == -1) {
		write(1, "\nfailed fork", 12);
	}

	if (pid == 0) {
		write(1, "\nchild returned", 15);
	}

	if (pid > 0) {
		write(1, "\nfather returned", 16);
	}

	while(1) { 
		// int i = getpid();
		// itoa(i, buff);
		// write(1, "\n", 1);
		// write(1, buff, 1);
	}
}
