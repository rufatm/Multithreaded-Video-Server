#ifndef __TOOLS_H
#define __TOOLS_H
#define DEBUG
#include "pthread.h"
#include <string> 

using namespace std;

enum REQUEST{START=0, STOP=1, SEEK=2, ERROR=-1};


// for now I'm using it to store the port for server. If we just need port number we may not need this struct 
// and just cast integer, but we may need more information such as thread id and what not..

// is replaced by worker_thread_t below
typedef struct _thread_data_t 
{ 
	int client_sd;
	int tid;
	char message[80]; 
	// in non-debug mode have a frame here. 
} thread_data_t; 
 

// Thread pool
// probably obsolete
typedef struct _thread_pool_t
{ 
	pthread_t pool[]; // make it of some fixed size
	int pool_size; 
	int active_threads; 
} thread_pool_t;



typedef struct _worker_thread_t 
{ 
	char message[80];
	pthread_t thread;	// thread associated with the worker
	int sd;  			// socket descriptor
	int tid; 			// thread id
	int socket; 		// port
	volatile bool is_active;
	int client_id;	 	// there's gotta be an id of a client associated with it 
	
	// I'm thinking we need to get all the information from the message into this struct 
	// such as priority, repeat, request_type. so when we read we need to write the information 
	
	int request_type;	// lets say start is 0, stop is 1, seek is 2 and -1 is error
	int priority; 
	int repeat;
	int frame_number;
	//pthread_cond_t allow = ;

} worker_thread_t;

typedef struct _message_t 
{ 
	int request_type; 	// 0 is start, 1 is stop, 2 is seek, -1 is error . see enum above
	int repeat; 
	int frame_number;
	char* movie_name;	 
	int client_id; 
	int priority;
	int threadID;
	char message[80];   // the message that we are sending right now
	char *buffer_message;
} message_t;

typedef struct imageBuffer
{
	unsigned char *buffer;
	int size;
}imageBuffer;
 
 #endif
