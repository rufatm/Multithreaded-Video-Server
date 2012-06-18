#include "MyMutex.h"


MyMutex::MyMutex() 
{ 
    
}


MyMutex::MyMutex(int n) 
{ 
	this->init(n);
}


void MyMutex::init(int n) 
{ 
	char module[] = "Leonardo"; 
	char name[80]; 
        fd = open ("/proc/Leonardo", O_RDONLY); 

	struct mutex_struct pass;
  	pass.id = -1; 
        pass.pid = getpid(); // hopefully will work 
        pass.action = 0;
	pass.test = n; 
        int res = ioctl(fd, MUTEX, &pass); 
	id = pass.id;        

} 


int MyMutex::kill_mutex() 
{ 
	close(fd); 
	return 0;
}


int MyMutex::lock() 
{ 
   struct mutex_struct pass;
   pass.id = id; 
   pass.pid = getpid(); // hopefully will work 
   pass.action = 1; 
   int res = ioctl(fd, MUTEX, &pass);
   return 0;   
} 



int MyMutex::unlock() 
{ 
    struct mutex_struct pass;
   pass.id = id;; 
   pass.pid = getpid(); // hopefully will work 
   pass.action = 2; 
   int res = ioctl(fd, MUTEX, &pass);
   return 0; 
}




