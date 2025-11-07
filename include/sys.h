#ifndef __SYS_H__
#define __SYS_H__

void system_call_handler();

int sys_write(int fd, char* buffer, int size);

int sys_getpid();

int sys_fork();

#endif /* __SYS_H__ */
