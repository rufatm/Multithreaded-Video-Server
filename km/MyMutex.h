#ifndef MY_MUTEX_H 
#define MY_MUTEX_H

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#include "pthread.h" 
#include "Leonardo.h" 


class MyMutex 
{

public:
	MyMutex();
	MyMutex(int x);

	int fd;

	int id;
	void init(int); 
	int kill_mutex(); 
	int lock(); 
	int unlock();
};


#endif
