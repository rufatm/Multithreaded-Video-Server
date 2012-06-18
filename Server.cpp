#include "Server.h"

using namespace std;

int num_workers; 

int main(int argc, char *argv[]) 
{
	if(argc != 3) 
	{ 
		cout << "usage: ./server <pool> <port>" << endl;
		return -1;
	} 
	signal(SIGTSTP, signal_callback_handler);  // register the handler

	pool_limit = atoi(argv[1]); 
	port = atoi(argv[2]);

	init(pool_limit);

	return 0;
}

void init(int poolSize) 
{ 
	clock_running = false;
	// set buffer size
	messageBuffer = new CircularBuffer(MAXSLOTS);
	if(messageBuffer)
        {
		// lock dispatcherMutex and dispatcherCompleteMutex
		pthread_mutex_lock(&dispatcherMutex);
		pthread_mutex_lock(&dispatcherCompleteMutex);

		num_workers = 0; 
		num_active_workers = 0;
	
		// create dispatcher thread
		pthread_t dispatcher;
		pthread_create(&dispatcher, NULL, dispatcher_thread, NULL);
	
		// create worker threads
		pthread_mutex_lock(&pool_mutex);
		create_pool(poolSize);	
		pthread_mutex_unlock(&pool_mutex);
	
		servConn(port);
	}
}

void *dispatcher_thread(void* var)
{
	while (1)
	{
		messageBuffer->popMessages();
	}
}

bool messageSorter(bufferMessage const& lhs, bufferMessage const& rhs)
{
	return lhs.priority > rhs.priority;
}

int create_pool(int n) 
{ 
	#ifdef DEBUG
		cout << "in create_pool" << endl;
	#endif

	volatile worker_thread_t** temp =(volatile worker_thread_t**)(malloc(n*sizeof(worker_thread_t)));
	for (int i = 0; i < num_workers; i++) 
	{ 
		temp[i] = pool[i];
	}
	pool = temp;
	
	for (int i = num_workers; i < n; i++) 
	{ 
		worker_thread_t* worker = (worker_thread_t*) malloc((sizeof(worker_thread_t)));
		worker->request_type = -1; 

		worker->sd = -1; 
		worker->tid = i; 
		worker->socket = -1; 
		
		pthread_mutex_lock(&active_mutex);
		worker->is_active = false; 
		pthread_mutex_unlock(&active_mutex);
		#ifdef DEBUG
			cout << "worker_function is about to begin " << endl;
		#endif
		pthread_create(&(worker->thread),NULL, worker_function, worker);

		pool[i] = worker;
	}
	num_workers = n;
	return 0;
}

void* worker_function(void* var) 
{ 
	worker_thread_t* thread = (worker_thread_t*) var; 
	
	while (!(thread->is_active)) { } // busy loop PUT THE CONDITIONAL VARIABLE IN HERE

	int photoID = 0;
        int play_continuously = 0;
	message_t* msg;

	while (1)
	{ 
		if (thread->is_active)
		{ 
			char data[80];
			bzero(data, 80); 

			int frame_size = 100;
			int len = 0;
			ioctl(thread->sd, FIONREAD, &len);

                        while (len == 0 && play_continuously)
			{
				ioctl(thread->sd, FIONREAD, &len);
				insertIntoBuffer(thread->sd,msg,photoID);

				if (!thread->repeat && photoID == frame_size-1) play_continuously = 0;

				photoID = (photoID + 1) % frame_size;
			}
			int res = read(thread->sd, &data, 80);

			if (res == 0) 
			{
				pthread_mutex_lock(&active_mutex); 
				thread->is_active = false;
				thread->sd = -1; 
				thread->tid = -1; 
				thread->socket = -1;
 				pthread_mutex_unlock(&active_mutex);
				continue;
			}					
			msg = create_message(data);
			msg->threadID = thread->tid;
			strcpy(thread->message,data);

			if (thread->client_id < 0)
			{
				thread->client_id = msg->client_id;
				thread->priority = msg->priority;
			}

			// if the message is correct, then assign necessary variables to this particular worker such as client_id, priority
			thread->request_type = msg->request_type; 

			switch (thread->request_type) 
			{ 
				case 0:  // start  
					thread->request_type = START; 
					thread->repeat = msg->repeat;
                                        play_continuously = 1;
                                        photoID = 0;
					break; 
				case 1:  // stop
					thread->request_type = STOP;
                                        play_continuously = 0;
					break; 
				case 2:  // seek
					thread->request_type = SEEK; 
					thread->frame_number = msg->frame_number;
					thread->repeat = 1;  // seek always repeats
                                        play_continuously = 1;
                                        photoID = msg->frame_number%100;
					break;
			}
		}
	}
	// depending on the request change the request_type and assign repeat and frame number if needed 
	//if no new messages received do whatever is needed depending on the state. All you need to do here is
	// say if the state is play, insert the message into the buffer and repeat if needed
}

void insertIntoBuffer(int socket, message_t* msg, int photoID)
{
	messageBuffer->insertMessage(msg->threadID,socket,msg->priority,photoID);
}

void* servConn (int p) 
{
	int port = p;	
  	int sd, new_sd;
  	struct sockaddr_in name, cli_name;
  	int sock_opt_val = 1;
  	int cli_len;
  	char data[80];		/* Our receive data buffer. */
  
 	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) 
 	{
 		perror("(servConn): socket() error");
 	   	exit (-1);
  	}

  
  	if (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, (char *) &sock_opt_val, sizeof(sock_opt_val)) < 0) 
  	{
   		perror ("(servConn): Failed to set SO_REUSEADDR on INET socket");
    	exit (-1);
  	}

  	name.sin_family = AF_INET;
  	name.sin_port = htons (port);
  	name.sin_addr.s_addr = htonl(INADDR_ANY);
  
  	if (bind (sd, (struct sockaddr *)&name, sizeof(name)) < 0) 
  	{
   		perror ("(servConn): bind() error");
  		exit (-1);
  	}

  	  	listen (sd, 5);
	cout << "listening" << endl;

  	for (;;) 
  	{
  		mysd = sd;
    	cli_len = sizeof (cli_name);
    	new_sd = accept (sd, (struct sockaddr *) &cli_name, (socklen_t* )&cli_len);
    	#ifdef DEBUG
	      	printf ("Assigning new socket descriptor:  %d\n", new_sd);
	    #endif
      
      	if (new_sd < 0) 
      	{
	      	#ifdef DEBUG
      		cout << "STOP" << endl;
      		#endif
			perror ("(servConn): accept() error");
			exit (-1);
      	}
      	//pthread_mutex_lock(&pool_mutex);
		go(new_sd);
	//	pthread_mutex_unlock(&pool_mutex);

      	
  	}
}

int go (int sd) 
{ 
	#ifdef DEBUG
	cout << "GO" << endl; 
	#endif

	for(int i = 0; i < num_workers; i++)
    {
    	 pthread_mutex_lock(&active_mutex); 

    	if(pool[i]->is_active == false)
        {
			//pthread_mutex_lock(&pool_mutex);
            pool[i]->sd = sd; 
            pool[i]->socket =  port;
            
            pool[i]->is_active = true; 
            #ifdef DEBUG
            cout << pool[i]->tid << " is activated" << endl;
            #endif
            pthread_mutex_unlock(&active_mutex);
            
           // pthread_mutex_unlock(&pool_mutex);

            
			//num_active_workers++;
			return 0;
    	}
    	pthread_mutex_unlock(&active_mutex);

    }
	#ifdef DEBUG
    cout << "pool size exceeded... Expanding the pool"  <<endl;
    #endif
    // what if there are no available workers? create some and go again  
	pthread_mutex_lock(&pool_mutex);
    create_pool(num_workers*2); 
	pthread_mutex_unlock(&pool_mutex);
    go(sd);
    
    // oh yes. If we got to this line, then pool has been expanded so get the clock running

    if (clock_running ==false) 
    { 
	    #ifdef DEBUG
        cout << "clock running = " << clock_running << endl;
        #endif

    	pthread_t michelangelo;  // thread name is boring
    	clock_running  =true; 
    	pthread_create(&michelangelo,NULL, clock_thread_function, NULL);
    	
    }
    
     
}

void* clock_thread_function(void* var) 
{ 
	clock_t start_time = clock();
	#ifdef DEBUG
	cout << "clock thread is running" << endl;
	#endif
	while (1) 
	{ 
		//cout << "start_time " << start_time << endl;
		clock_t current = clock() - start_time; 
		//cout<<"current time " << current <<  endl;
		//cout << "seconds passed " <<   (double)current / (double)(CLOCKS_PER_SEC) << endl;
		
		if ( ( (double)current / (double)(CLOCKS_PER_SEC)) > PERIOD) 
		{ 
			// clean up
			#ifdef DEBUG
			cout << "num_workers " << num_workers << " pool_limit " << pool_limit << endl;
			#endif
			int num_remove_workers = num_workers - pool_limit;
			#ifdef DEBUG
			cout << "num_remove_workers " << num_remove_workers << endl;
			#endif
			
			//pthread_mutex_lock(&pool_mutex);
			clean_pool(num_remove_workers); 
			//pthread_mutex_unlock(&pool_mutex);
		
			#ifdef DEBUG
			cout << "left clean_pool" << endl;
			#endif
			// i dont think mutex is needed here
			num_remove_workers = num_workers - pool_limit;

			if (num_remove_workers <= 0) 
			{ 
				clock_running  =false;
				pthread_exit(NULL); 
				return (void*)1;;
				 
			} 
				start_time = clock();

		} 
	} 
	#ifdef DEBUG
	cout << "leaving Michelangelo" << endl;
	#endif
}

int clean_pool(int n) // number of workers to remove 
{ 
	for (int i =0; i < n; i++) 
	{ 
		#ifdef DEBUG
		cout << "num_workers " << num_workers << endl;
		#endif
		int size = num_workers;
		#ifdef DEBUG
		cout << "size "<< size << endl;
		#endif
		for (int j = num_workers; j >= 1; j--) 
		{ 
		pthread_mutex_lock(&active_mutex);
		#ifdef DEBUG
		cout << "testing thread " << pool[j-1]->tid << " active " << pool[j-1]->is_active << endl;
		#endif
			
			if ((pool[j-1]->is_active == 0))
			{
				#ifdef DEBUG
				cout << "thread " << pool[j-1]->tid << " is about to be deleted" << endl;
				#endif
					//	cout << "index " << pool.begin() << endl;
				
				pthread_cancel(pool[j-1]->thread);
			
				//pool.erase(pool.begin() + j);
				pool[j-1] = NULL;
				#ifdef DEBUG
				cout  << "end" << endl;
				cout << "num_workers " << num_workers << endl;
				#endif
				num_workers--; 
				
				//cout<< "decremented num_workers" << endl;
				//cout << "decremented num_active_workers" << endl;
			}
			#ifdef DEBUG
			cout << "about to unlock active_mutex" << endl;
			#endif
			pthread_mutex_unlock(&active_mutex);
			#ifdef DEBUG
			cout << "unlocked active mutex" << endl;
			#endif
		}
	}	
	#ifdef DEBUG
	cout << "leaving clean_pool" << endl;
	#endif
	return 0;
} 

void signal_callback_handler(int signum)
{
   #ifdef DEBUG
   printf("Caught signal %d\n",signum);
   #endif
   // close the socket
   close(mysd);
   exit(signum);
}

message_t * create_message(char data[]) 
{ 
	message_t* msg = (message_t*) malloc(sizeof(message_t));
	
	stringstream stream(data);
	string word;
	getline(stream, word, ':'); 
	msg->client_id = atoi(word.c_str());
	#ifdef DEBUG
	cout << "client id " << word << endl;
	#endif
			
	getline(stream, word, ':'); 
	msg->priority = atoi(word.c_str()); 
	#ifdef DEBUG
	cout << "priority " << word << endl;
	#endif
	
	getline(stream, word, ':'); 
	if (strcmp(word.c_str(), "start_movie") == 0) 
		msg->request_type = START; 
	else if (strcmp(word.c_str(), "stop_movie") == 0)
		msg->request_type = STOP; 		
	else if (strcmp(word.c_str(), "seek_movie") == 0)
		msg->request_type = SEEK; 
	else 
		msg->request_type = ERROR;
	#ifdef DEBUG				
	cout << "request " << word << endl;
	#endif
	
	getline(stream,word,':');
	stringstream arg_stream(word); 
	string arg;
	getline(arg_stream,arg,',');
	#ifdef DEBUG
	cout << "MOVIE NAME " << arg << endl;
	#endif
	//msg->movie_name = arg.c_str();
	
	if (msg->request_type == START)//  || msg->request_type == SEEK)
	{
		
		getline(arg_stream,arg,',');
		#ifdef DEBUG
		cout << "ARGUMENTS " << arg << endl;
		#endif
		msg->repeat  = atoi(arg.c_str()); 
	//	system("PAUSE");
	} 
	else if (msg->request_type == SEEK) 
	{ 
		getline(arg_stream,arg,',');
		#ifdef DEBUG
		cout << "ARGUMENTS " << arg << endl;
		#endif
		msg->frame_number  = atoi(arg.c_str()); 
	}
	return msg;
}

string form_message(worker_thread_t* thread)
{ 
	int tid = thread->tid;
	#ifdef DEBUG
	cout << "tid " << tid << endl;
	#endif
	
	int client_sd = thread->sd;
	char* message = thread->message;
	
	string buffer_message;
	stringstream ss; 
	
	ss << tid; 
	buffer_message += ss.str();
	
	buffer_message += ","; 
	
	ss.str("");
	ss << client_sd;
	buffer_message += ss.str();
	
	buffer_message += ",";
	
	//buffer_message += message;
	buffer_message.append(message);
	return buffer_message;
	
}

void* print_message(void* var) 
{ 
	thread_data_t* thread = (thread_data_t*) var;
	char* str = thread->message;
	cout << str << endl;
}

