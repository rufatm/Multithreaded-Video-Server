#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(int maxslots)
{
	size = maxslots;
	buffer = new bufferMessage[maxslots];
	nextFreeIndex = 0;
	messages = 0;
	
	pthread_mutex_init(&insertMutex, NULL);
	pthread_mutex_init(&popMessagesMutex, NULL);

	pthread_cond_init (&bufferFullCondition, NULL);
	pthread_cond_init (&bufferNotFullCondition, NULL);
}

void CircularBuffer::insertMessage(int threadID, int socket, int priority, int photoID)
{
	pthread_mutex_lock(&insertMutex);
	while(messages == size)
		pthread_cond_wait(&bufferNotFullCondition, &insertMutex);
	bufferMessage *newMessage = new bufferMessage;
	newMessage->threadID = threadID;
	newMessage->socket = socket;
	newMessage->priority = priority;
	newMessage->photoID = photoID;

	buffer[nextFreeIndex] = *newMessage;
	nextFreeIndex = (nextFreeIndex+1) % size;
	messages++;
	
	if(messages == size)
	{
		// signal dispatcher that the buffer is full
		while(messages == size)
			pthread_cond_signal(&bufferFullCondition);
	}
	else
	{
		// signal the next insert message
		pthread_cond_signal(&bufferNotFullCondition);
	}
    pthread_mutex_unlock(&insertMutex);
}

int CircularBuffer::numberOfMessages()
{
	return messages;
}

void CircularBuffer::popMessages()
{
	pthread_mutex_lock(&popMessagesMutex);
	if(messages != size)
		pthread_cond_wait(&bufferFullCondition, &popMessagesMutex);
	
	vector<bufferMessage> *messageBuffer = new vector<bufferMessage>;

	for(int i=0; i<messages; i++)
	{
		messageBuffer->push_back(buffer[i%size]);
	}
	
	stable_sort(messageBuffer->begin(), messageBuffer->end(), CircularBuffer::messageSorter);

	for(std::vector<bufferMessage>::iterator it = messageBuffer->begin(); it != messageBuffer->end(); ++it) 
	{
		bufferMessage message = *it;
		char photoLocation[80];
		sprintf(photoLocation, "./images/sw%d.ppm", message.photoID + 1);
		unsigned char* buff = bufferFromImage(photoLocation);
		write(message.socket, buff, IMG_SIZE);
	}

	messages = 0;
	while(messages == 0)
		pthread_cond_signal(&bufferNotFullCondition);
    pthread_mutex_unlock(&popMessagesMutex);
    
	//return newBuffer;
}

void CircularBuffer::printMessages()
{
	for(int i=0; i< messages; i++)
	{
		bufferMessage b = buffer[i%size];
		printf("Socket: %d Priority: %d Photo ID: %d\n", b.socket, b.priority, b.photoID);
	}
}

bool CircularBuffer::messageSorter(bufferMessage const& lhs, bufferMessage const& rhs)
{
	return lhs.priority > rhs.priority;
}

unsigned char* CircularBuffer::bufferFromImage(char *photoLocation)
{
    pixel** pixarray;
    FILE *fp;
    int cols, rows;
    pixval maxval;
    register int x, y;
    unsigned char *buf=NULL;

    if ((fp = fopen (photoLocation,"r")) == NULL) {
      	fprintf (stderr, "%s: Can't open input file:\n %s.\n", photoLocation);
      	exit (1);
    }

    pixarray = ppm_readppm (fp, &cols, &rows, &maxval);
    fclose (fp);

    if (buf == NULL)
      buf = (unsigned char *)malloc(IMG_SIZE);

    /* Use this double 'for loop' in the consumer/client threads.
     * You'll need to modify this code to read from a socket into 
     * a data structure called "pixarray" first.
     */

    for (y = 0; y < rows; y++) {
      for (x = 0; x < cols; x++) {
	buf[(y*cols+x)*3+0] = PPM_GETR(pixarray[rows-y-1][x]);
	buf[(y*cols+x)*3+1] = PPM_GETG(pixarray[rows-y-1][x]);
	buf[(y*cols+x)*3+2] = PPM_GETB(pixarray[rows-y-1][x]);
      }
    }
   ppm_freearray (pixarray, rows);
   return buf;
}


