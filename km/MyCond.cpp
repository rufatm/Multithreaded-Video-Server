#include "MyCond.h" 


using namespace std;

MyCond::MyCond() 
{ 
	MyMutex sleeper(0);
	
	MyMutex m2(1);
	init_cond();
}


int MyCond::init_cond() 

{ 
	char module[] = "Leonardo"; 
	char name[80]; 
	fd_cond = open("/proc/Leonardo", O_RDONLY); 
	sleepers = 0;
	
	
	
	
	
	//struct condition_struct pass; 
	//pass.pid = getpid(); 
	//pass.action = 0; 
	//int res = ioctl(fd,CONDITION,&pass); 
	return 1;
}



int MyCond::wait() 
{ 

	m->unlock();
	sleepers++;
	sleeper.lock(); 
	m->lock();

	
	return 0;
	
}

int MyCond::signal() 
{
	m2.lock();
 
	if (sleepers > 0)	
		sleeper.unlock(); 
		
	m2.unlock();
	
}
