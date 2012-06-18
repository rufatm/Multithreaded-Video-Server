#include "Client.h"

/* Global Vars */
char cmd[MAX_REQ_LEN];
char cmd_copy[MAX_REQ_LEN];
char *tokens;
char *method;
char *data;
int connected;

int main(int argc, char **argv)
{
	// initial prompt and input
	prompt(0);

	while (strcmp(cmd, "quit") != 0)
	{
		// process command into [method, data]
		if (process_cmd() == -1) continue;

		// connect <IPAddress:port>
		if (strcmp(method, "connect") == 0)
		{
			if (cliConn() == -1) continue;
		}
		// send request stop_movie
		else if (strcmp(method, "stop") == 0)
		{
			if (cliReq1() == -1) continue;
		}
		// send requests start_movie or seek_movie
		else if (strcmp(method, "start") == 0 || strcmp(method, "seek") == 0)
		{
			if (cliReq2() == -1) continue;
		}
		// reprint prompt and wait for commands
		int valid = 0;
		while (valid == 0)
		{
			valid = prompt(1);
			if (strcmp(method,"") == 0) valid = 1;
		}
	}
	// quit: close socket if exists
	cliQuit();
	exit(0);
}

/* prints a prompt and stores the input in 'cmd'
	returns 0 if something was inputted */
int prompt(int wait)
{
	printf("coredump$ ");

	// clear out the cmd string in case just press return
	bzero(cmd, strlen(cmd));
	if (wait == 1)
	{
		getchar();  // wait for user input

		if ( scanf("%[^\n]", cmd) == 1 )
		{
			strcpy(cmd_copy, cmd);
			return 1;
		}
	}
	else
	{
		scanf("%[^\n]", cmd);
		strcpy(cmd_copy, cmd);
		return 1;
	}
	return 0;
}

/* process input at prompt 
	returns -1 if invalid */
int process_cmd()
{
	// save entire cmd
	char cmd_copy[MAX_REQ_LEN];
	strcpy(cmd_copy, cmd);

	// process command into [method, data]
	tokens = strtok(cmd, " ");

	if (tokens != NULL)
	{
		//printf("method: %s\n", tokens);
		method = tokens;
		tokens = strtok(NULL, "\n");
	}
	else
	{	
		if (connected == 0)
		{
			printf("please first establish a connection: connect <IPAddress>:<port>\n");
		}
		return 0;
	}
	data = tokens;
	//printf("process cmd: [%s]\n", cmd);
	//printf("method: [%s]\n", method);
	//printf("data: [%s]\n", tokens);

	// invalid input: print usage and prompt again
	if (strcmp(method, "\n") == 0)
	{
		prompt(1);
		return -1;
	}
	else if (valid_method(method) == -1)
	{		
		printf("usage:\tconnect <IPAddress>:<port> || quit\n");
		printf("\tstart <movie_name>,<repeat> || stop <movie_name> || seek <movie_name>,<frame_number>\n");

		prompt(1);
		return -1;
	}
	return 0;
}

/* test for a valid method
	returns -1 if invalid */
int valid_method(char *method)
{
	if (!strcmp(method, "connect") ^ !strcmp(method, "start") 
	    ^ !strcmp(method, "stop") ^ !strcmp(method, "seek") == 0)
	{
		return -1;
	}
	return 0;
}

/* connect to the requested IP address and port 
	returns -1 if invalid arg or error*/
int cliConn()
{
	struct sockaddr_in name;
	struct hostent *hent;

	pthread_t listenthrd;
	
	// may only call connect once
	if (connected == 1) printf("connection already established\n");
	else
	{
		char *ipaddress;
		char *port_str;
		int port;

		// parse IP address, port
		tokens = strtok(data, ":");
		//printf("tokens: %s\n", tokens);

		if (tokens != NULL)
		{
			ipaddress = tokens;		
			tokens = strtok(NULL, "\n");
			//printf("after strtok for newline: %s\n", tokens);	
		}
		else
		{
			printf("usage: connect <IPAddress>:<port>\n");
			prompt(1);
			return -1;
		}
		// improper format 
		if (tokens == NULL)
		{
			printf("usage: connect <IPAddress>:<port>\n");
			prompt(1);
			return -1;
		}
		//printf("port: %s\n", tokens);
		port_str = tokens;
		sscanf(port_str, "%d", &port);

		if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		{
			printf("cliConn: socket() error\n");
			prompt(1);
			return -1;
		}
		if ( (hent = gethostbyname(ipaddress)) == NULL )
		{
			printf("cliConn: IP address %s not found\n", ipaddress);
			prompt(1);
			return -1;
		}
		bzero(&name, sizeof(name));
		bcopy(hent->h_addr, &name.sin_addr, hent->h_length);
		name.sin_family = AF_INET;
		name.sin_port = htons(port);

		// create listening thread
		pthread_create(&(listenthrd), NULL, listening_thread, NULL);

		/* connect port */
		int conn;
		if ( (conn = connect(sd, (struct sockaddr *) &name, sizeof(name))) < 0 )
		{
			printf("cliConn: connect() error\n");
			prompt(1);
			return -1;
		}
		else
		{
			// connection established
			connected = 1;

			srand(time(NULL));
			priority = rand() % NUM_PRIORITY + 1;

			id = 0;
		}
	}
	return 0;
}

void* listening_thread(void *x)
{
	while (1)
	{
		unsigned char *data = (unsigned char*)malloc(IMG_SIZE);
		bzero(data, IMG_SIZE);

		int n = 0;
		while (n < IMG_SIZE)
		{
			n += read(sd, data+n, IMG_SIZE-n);
		}
		usleep(SLEEP_TIME);
		init(data);
	}
}

/* request stop_movie
	returns -1 if invalid arg */
int cliReq1()
{
	if (connected == 1)
	{
		if (data != NULL)
		{
			// send request to the server and get response
			stop_movie(data);
		}
		else
		{
			printf("usage: %s <movie_name>\n", method);
			prompt(1);
			return -1;
		}
	}
	else printf("please first establish a connection: connect <IPAddress>:<port>\n");

	return 0;
}

/* request start_movie or seek_movie
	returns -1 if invalid arg */

int cliReq2()
{
	if (connected == 1)
	{
		char name[MAX_REQ_LEN];
		int num;
		if (data != NULL && (sscanf(data, "%[^,],%i", name, &num) == 2))
		{
			// send request to the server and get response
			if (strcmp(method, "start") == 0) start_movie(name, num);
			else seek_movie(name, num);
		}
		else
		{
			if (strcmp(method, "start") == 0)
				printf("usage: %s <movie_name>,<repeat>\n", method);
			else printf("usage: %s <movie_name>,<frame_number>\n", method);

			prompt(1);
			return -1;
		}
	}
	else printf("please first establish a connection: connect <IPAddress>:<port>\n");

	return 0;
}

/* quit: close socket if exists
	returns -1 on error */
int cliQuit()
{
	if (connected == 1) return close(sd);

	return 0;
}

void start_movie(char *movie_name, int repeat)
{
	string message;
	stringstream out;
	out << repeat;
	string arguments[] = {movie_name,out.str()};
	message = createMessage("start_movie", arguments, 2);
	const char *newMessage = message.c_str();

	write(sd, message.c_str(), strlen(message.c_str())); 
}

void stop_movie(char *movie_name)
{
	string arguments[] = {movie_name};
	string message = createMessage("stop_movie", arguments, 1);

	write(sd, message.c_str(), strlen(message.c_str()));	
}

void seek_movie(char *movie_name, int frame_number)
{
	stringstream out;
	out << frame_number;

	string arguments[] = {movie_name,out.str()};
	string message = createMessage("seek_movie", arguments, 2);

	write(sd, message.c_str(), strlen(message.c_str()));	
}

string createMessage(string function, string arguments[], int len)
{
	stringstream out;
	out << id;

	string message;
	message += out.str();
	message += ":";

	out.str("");
	out << priority;
	
	message += out.str();
	message += ":";
	message += function;
	message += ":";

	for (int i=0; i<len; i++)
	{
		out.str("");
		out << arguments[i];

		message += out.str();
		if (i != len-1) message += ",";		
	}
	return message;
}

