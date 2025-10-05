/*
 * sys.c - Syscalls implementation
 */
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

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}
