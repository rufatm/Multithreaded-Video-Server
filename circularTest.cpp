#include <iostream>
#include "CircularBuffer.h"
pthread_t dispatcher;
pthread_t producer;
CircularBuffer *buff;

void* worker_function(void* var) 
{
	for(int i=0; ; i++ % 100)
	{
		printf("%d\n",i);
		buff->insertMessage(1,1, 1, 1);
	}
}

void* dispatcher_function(void* var) 
{
	while(1)
		buff->popMessages();
}

int main()
{
	buff = new CircularBuffer(5);
	if(buff)
	{
		pthread_create(&producer, NULL, worker_function, NULL);
		pthread_create(&dispatcher, NULL, dispatcher_function, NULL);
		pthread_join(dispatcher, NULL);	
	}
	
	return 0;
}