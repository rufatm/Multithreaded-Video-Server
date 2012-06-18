#ifndef __SERVER_H
#define __SERVER_H

#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <cstdio>

#include "CircularBuffer.h"
#include "tools.h"



#define DEFAULT_POOL	10
#define ID		0;

const int PERIOD = 8;
const int MAXSLOTS = 15;

pthread_t server_thread;
 
int mysd;  // this is to hold the value of sd so that when we hit ctrl-c we still close the connection
int pool_limit;
int port;
int num_active_workers;
bool clock_running;
CircularBuffer *messageBuffer;
volatile worker_thread_t** pool;   // vector is not syncronization safe

thread_pool_t* pool_struct;
pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER; // pool needs to be accessed sequentially
pthread_mutex_t active_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t messageBufferMutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t dispatcherMutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t dispatcherCompleteMutex = PTHREAD_MUTEX_INITIALIZER;

void init(int);
std::string form_message(worker_thread_t*);

void *dispatcher_thread(void* var);
bool messageSorter(bufferMessage const& lhs, bufferMessage const& rhs);
void insertIntoBuffer(int socket, message_t* msg, int photoID);
void* print_message(void*); // test function to make server simply print the message sent by the client
void* worker_function(void*);
void* servConn(int);
void signal_callback_handler(int signum);
void* clock_thread_function(void*);
int create_pool(int);
int go(int);
int clean_pool(int);
message_t* create_message(char[]);

#endif

