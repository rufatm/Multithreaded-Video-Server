#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <algorithm>
#include <GL/glx.h>
#include <X11/keysym.h>
#include "tools.h"

extern "C" 
{
	#include <pam.h>
}

#define IMG_SIZE	57600  // row * col * 3

using namespace std;

struct bufferMessage
{
	int threadID;
	int socket;
	int priority;
	int photoID;
};	

class CircularBuffer
{
	public:
		CircularBuffer(int maxslots);
		void insertMessage(int threadID, int socket, int priority, int photoID);
		int numberOfMessages();
		void popMessages();
		void printMessages();
		static bool messageSorter(bufferMessage const& lhs, bufferMessage const& rhs);

	private:
		unsigned char* bufferFromImage(char *photoLocation);
		bufferMessage *buffer;
		int size;
		int nextFreeIndex;
		int messages;
		pthread_cond_t bufferFullCondition;
		pthread_cond_t bufferNotFullCondition;		
		pthread_mutex_t insertMutex; 
		pthread_mutex_t popMessagesMutex; 
};

