#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
extern "C" {
#include <pam.h>
}

#define MAX_REQ_LEN	80
#define NUM_PRIORITY	10
#define IMG_SIZE	57600  // row * col * 3
#define SLEEP_TIME	30000

using namespace std;

int sd;
int id;
int priority;

int prompt(int wait);
int process_cmd();
int valid_method(char *method);
int cliConn();
int cliReq1();
int cliReq2();
int cliQuit();

void* listening_thread(void *x);
void start_movie(char *movie_name, int repeat);
void stop_movie(char *movie_name); 
void seek_movie(char *movie_name, int frame_number);
string createMessage(string function, string arguments[],int len);
extern "C" void init(unsigned char *data);

#endif

