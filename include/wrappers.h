
#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

int write (int fd, char * buffer, int size);

int gettime();

int getpid();

int fork();

void exit();

void block();

int unblock(int pid);


#endif /* __WRAPPERS_H__ */

