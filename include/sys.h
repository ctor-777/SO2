#ifndef __SYS_H__
#define __SYS_H__

void system_call_handler();

int sys_write(int fd, char* buffer, int size);

int sys_getpid();

int sys_fork();

void sys_exit();

void sys_block();

int sys_unblock(int pid);


#endif /* __SYS_H__ */
