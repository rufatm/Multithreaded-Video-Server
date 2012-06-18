#ifndef MY_COND_H 
#define MY_COND_H


#include <stdio.h>
#include <iostream>
#include <pthread.h>

#include "MyMutex.h" 

class MyCond 
{
public:
	MyMutex* m;
	 MyMutex m2;
	 MyMutex sleeper;
	MyCond();
	int sleepers;
	int wait(); 
	int signal(); 
	int init_cond(); 
	int fd_cond; 
};

#endif 

