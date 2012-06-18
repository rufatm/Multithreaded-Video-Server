
#include <iostream>
#include <pthread.h>
#include "MyMutex.h"
#include "MyCond.h"
using namespace std;







MyMutex mutex(1);
MyCond cond;
int unitsOfData;

static void* consRoutine(void* i) {

	for (int i = 0; i < 10; i++) {
		printf("Consumer %u about to wait on lock\n", (unsigned)pthread_self());
		mutex.lock();

		printf("Consumer %u doing work\n", (unsigned)pthread_self());

		sleep(1);

		if (--unitsOfData == 0) {
			cond.signal();
		}

		printf("Consumer %u done\n", (unsigned)pthread_self());
		mutex.unlock();

		sleep(1);
	}



	return (void*) 1;
}

static void* prodRoutine(void* i) {
	for (int i = 0; i < 8; i++) {
		printf("Producer about to wait on lock\n");
		mutex.lock();
		printf("Producer has lock\n");

		printf("Producer about to wait on condition variable\n");
		cond.wait();

		printf("Producer is doing work\n");

		sleep(1);
		unitsOfData += 30;

		printf("Producer done");
		mutex.unlock();
	}

}

int main() {

  int numConsumers = 20;
  unitsOfData = 30;
  
  pthread_t consumers[numConsumers];
  pthread_t producer;
  
  int t;
  for(t=0; t<numConsumers; t++){
    pthread_create(&consumers[t], NULL, consRoutine, (void *) 1);
  }
  
  pthread_create(&producer, NULL, prodRoutine, (void *) 1);
  
  for (t = 0; t< numConsumers; t++) {
    pthread_join(consumers[t], NULL);
  }
  
  pthread_join(producer, NULL);
  
  while(true);
  
  return 0;
}
