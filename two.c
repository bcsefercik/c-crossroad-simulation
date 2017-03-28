#include <pthread.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_THREADS  5
#define CAPACITY 1313
pthread_mutex_t mutex;
pthread_cond_t honk;
int policePlaying = 0;

/* Sleep Function */
int pthread_sleep (int seconds)
{
   pthread_mutex_t mutex;
   pthread_cond_t conditionvar;
   struct timespec timetoexpire;
   if(pthread_mutex_init(&mutex,NULL))
    {
      return -1;
    }
   if(pthread_cond_init(&conditionvar,NULL))
    {
      return -1;
    }
   struct timeval tp;
   //When to expire is an absolute time, so get the current time and add //it to our delay time
   gettimeofday(&tp, NULL);
   timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;

   pthread_mutex_lock (&mutex);
   int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
   pthread_mutex_unlock (&mutex);
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&conditionvar);

   //Upon successful completion, a value of zero shall be returned
   return res;

}



/* Car structure */
typedef struct Car{
	int ID;
	time_t time;
}Car;


/* Queue Implementation */
typedef struct Queue
{
	char *name;
    int capacity;
    int size;
    int front;
    int rear;
    Car *elements;
}Queue;

Queue *createQueue()
{
    /* Create a Queue */
    Queue *Q;
    Q = (Queue *)malloc(sizeof(Queue));
    /* Initialise its properties */
    Q->elements = (Car *)malloc(sizeof(Car)*CAPACITY);
    Q->size = 0;
    Q->capacity = CAPACITY;
    Q->front = 0;
    Q->rear = -1;
    /* Return the pointer */
    return Q;
}

void Dequeue(Queue *Q){
    /* If Queue size is zero then it is empty. So we cannot pop */
    if(Q->size!=0){
        Q->size--;
        Q->front++;
        /* As we fill elements in circular fashion */
        if(Q->front==Q->capacity)
        	Q->front=0;
    }
    return;
}

Car front(Queue *Q){
    if(Q->size!=0) {
        return Q->elements[Q->front];
    }
}

void Enqueue(Queue *Q,Car element)
{
    /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
    if(Q->size == Q->capacity) {
        printf("Queue is Full\n");
    }else{
        Q->size++;
        Q->rear = Q->rear + 1;
        /* As we fill the queue in circular fashion */
        if(Q->rear == Q->capacity)
        {
            Q->rear = 0;
        }
        /* Insert the element in its rear side */
        Q->elements[Q->rear] = element;
    }
    return;
}


/* Initilizations */

/* 	Put four lanes into one array */
Queue *lanes[4];


static int carID = 1;
float probability;

int emptynorth=0;

int currentQueue=3;

int snaptime;

FILE *carlog;
FILE *policelog;


/* lane_init() is called in main() It puts one car into every lane.*/
void lane_init(){
	while(carID<5){
		
		Car ddosh;
		ddosh.ID = carID;
		ddosh.time = time(NULL);
		Enqueue(lanes[(carID-1)],ddosh);
		carID++;
	}
}


/* Function to decide the next lane */
int selectQueue(Queue *l[],int cr){
	/* 	In the instructions N>E>S>W
		therefore I put lanes into array accordingly. N=3...W=0.
		It saves us to think about this priority system with the correct max selector functions.
	*/
	int j=0;
	int crt = (int) time(NULL);
	/*	This function gives index of maximum element of array. If there is more than one,
		it is out is given according to priority system.
	*/
	int maxindex(int a[]){
	   int i, max=-1, ind=3;
	   for (i=0; i<4; i++){
		 if (a[i]>=max){
		    max=a[i];
		    ind = i;
		 }
	   }
	   return ind;
	}

	/*	This function gives maximum element of array. If there is more than one,
		it is out is given according to priority system.
	*/
	int maxval(int a[]){
	   int i, max=-1;
	   for (i=0; i<4; i++){
		 if (a[i]>=max){
		    max=a[i];
		 }
	   }
	   return max;
	}

	int sizes[4] = {l[0]->size,l[1]->size,l[2]->size,l[3]->size};

	/*	I put waiting times into arrivals array.
	*/
	int arrivals[4];
	for(j=0; j<4; j++){
		if(sizes[j]==0){arrivals[j]=0;}
		else{
			arrivals[j] = crt-((int) front(l[j]).time);
		}
		//For test:  printf("%d\n", sizes[j]);
	}

	int msizev = maxval(sizes);
	int msizei = maxindex(sizes);

	int marrv = maxval(arrivals);
	int marri = maxindex(arrivals);
	/*	If there are 5 or more cars in a lane, current lane becomes it. 
		Again since array is used, every priority issue is considered. */
	if(msizev>=5){return msizei;}
	/*	If there is a car waiting for 20 seconds in a lane, current lane becomes it. 
		Again since array is used, every priority issue is considered. */
	else if(marrv>=20){return marri;}
	/*	If the current lane is empty,the lane containing most cars becomes current.
		Again since array is used, every priority issue is considered. */
	else if(sizes[cr]==0){return msizei;}
	/* If no condition holds, current lane stays current. */
	else{return cr;}

}


/* West Lane*/
void *west(void *t){
	time_t crt = time(NULL);
	char tbuf[20];
	long currenttime = (long) crt;
	long endtime = (long) t;

	float random;
	int rr;
	
	while(currenttime<endtime){
		pthread_sleep(1);
		
		/* implementing random float */
		rr = rand();
		random = (float) rr;
		random =  (rr%101)*0.01;

		if(probability>=random){
			pthread_mutex_lock(&mutex);
			Car wc;
			wc.ID = carID;
			wc.time = time(NULL);
			Enqueue(lanes[0],wc);
			//For test:  printf("w work\n");
			carID++;
			strftime(tbuf, 20, "%H:%M:%S", localtime(&crt));

			if (policePlaying==1){
				policePlaying=0; 
				//For test:  printf("honkiii ponkiiii  %s\n",tbuf); 
				fprintf(policelog, "%s\t\tHonk\n", tbuf);
				pthread_cond_signal(&honk);
			}
			pthread_mutex_unlock(&mutex);
		
		}
		
		crt = time(NULL);
		currenttime = (long) crt;
	}
	pthread_exit(NULL);

}		



/* South Lane*/
void *south(void *t){
	time_t crt = time(NULL);
	char tbuf[20];
	long currenttime = (long) crt;
	long endtime = (long) t;

	float random;
	int rr;
	
	while(currenttime<endtime){
		pthread_sleep(1);

		/* implementing random float */
		rr = rand();
		random = (float) rr;
		random =  (rr%101)*0.01;
	
		if(probability>=random){
			pthread_mutex_lock(&mutex);
			Car wc;
			wc.ID = carID;
			wc.time = time(NULL);
			Enqueue(lanes[1],wc);
			//For test:  printf("s work\n");
			carID++;
			strftime(tbuf, 20, "%H:%M:%S", localtime(&crt));
			if (policePlaying==1){
				policePlaying=0; 
				//For test:  printf("honkiii ponkiiii  %s\n",tbuf); 
				fprintf(policelog, "%s\t\tHonk\n", tbuf);
				pthread_cond_signal(&honk);
			}
			pthread_mutex_unlock(&mutex);
		}
		
		crt = time(NULL);
		currenttime = (long) crt;
	}
	pthread_exit(NULL);

}


/* East Lane*/
void *east(void *t){
	time_t crt = time(NULL);
	char tbuf[20];
	long currenttime = (long) crt;
	long endtime = (long) t;

	float random;
	int rr;
	
	while(currenttime<endtime){
		pthread_sleep(1);
		
		/* implementing random float */
		rr = rand();
		random = (float) rr;
		random =  (rr%101)*0.01;
		
		if(probability>=random){
			pthread_mutex_lock(&mutex);
			Car ec;
			ec.ID = carID;
			ec.time = time(NULL);
			Enqueue(lanes[2],ec);
			carID++;
			//For test:  printf("e work\n");
			strftime(tbuf, 20, "%H:%M:%S", localtime(&crt));
			if (policePlaying==1){
				policePlaying=0; 
				//For test:  printf("honkiii ponkiiii  %s\n",tbuf); 
				fprintf(policelog, "%s\t\tHonk\n", tbuf);
				pthread_cond_signal(&honk);
			}
			pthread_mutex_unlock(&mutex);
		}
		crt = time(NULL);
		currenttime = (long) crt;
	}
	pthread_exit(NULL);

}


/* North Lane*/
void *north(void *t){
	time_t crt = time(NULL);
	char tbuf[20];
	long currenttime = (long) crt;
	long endtime = (long) t;

	float random;
	int rr;
	
	while(currenttime<endtime){
		pthread_sleep(1);		
		/* implementing random float */
		pthread_mutex_lock(&mutex);
		rr = rand();
		random = (float) rr;
		random =  (rr%101)*0.01;

		/*	Here different than other lanes I check whether any car comes to this lane in every 20 second or not.
			If it didn't happen, I put a car manually. I hold the time in emptynorth variable.
			If probability holds I reset the variable, otherwise it is increased by one.
		*/
		if((1.0-probability)>=random || emptynorth==19){
			
			crt = time(NULL);
			currenttime = (long) crt;
			Car nc;
			nc.ID = carID;
			nc.time = crt;
			Enqueue(lanes[3],nc);
			emptynorth = 0;
			carID++;
			//For test:  printf("n work\n");
			strftime(tbuf, 20, "%H:%M:%S", localtime(&crt));
			if (policePlaying==1){
				policePlaying=0; 
				//For test:  printf("honkiii ponkiiii  %s\n",tbuf); 
				fprintf(policelog, "%s\t\tHonk\n", tbuf);
				pthread_cond_signal(&honk);
			}
			
		} else{
			
			emptynorth++;
		}
		crt = time(NULL);
		currenttime = (long) crt;
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);

}

/* Police */
void *police(void *t){
	
	time_t crt;
	time_t arr;
	char tbuf[20];
	char abuf[20];
	
	long currenttime = (long) crt;
	long endtime = (long) t;
	
	while(currenttime<endtime && policePlaying==0){
		pthread_sleep(1);
		
		/* The output for 3 seconds is given according to given time. */
		if(snaptime==currenttime || (snaptime+1)==currenttime || (snaptime+2)==currenttime){
			printf("At %s:\n\n",tbuf);
			printf("    %d   \n",lanes[3]->size);
			printf(" %d     %d\n",lanes[0]->size,lanes[2]->size);
			printf("    %d   \n",lanes[1]->size);
			
		}
		
		pthread_mutex_lock(&mutex);
		currentQueue = selectQueue(lanes,currentQueue);
		crt = time(NULL);
		strftime(tbuf, 20, "%H:%M:%S", localtime(&crt));

		/* 	If currentline is empty,
			it means ever single lane is empty thanks to usefulness of selectQueue () function. 
		*/
			
		if(lanes[currentQueue]->size==0){
			//For test:  printf("Cell Phone  %s\n",tbuf);
			fprintf(policelog, "%s\t\tCell Phone\n", tbuf);
			policePlaying=1; 
			if(currenttime<(endtime-3)){pthread_cond_wait(&honk, &mutex);}
			currentQueue = selectQueue(lanes,currentQueue);
			pthread_sleep(3);

		}
		crt = time(NULL);
		strftime(tbuf, 20, "%H:%M:%S", localtime(&crt));
		arr = front(lanes[currentQueue]).time;
		strftime(abuf, 20, "%H:%M:%S", localtime(&arr));

		//For test:  printf("%d\t%s\t%s\t%s\t%d\n", front(lanes[currentQueue]).ID, lanes[currentQueue]->name,abuf,tbuf,abs((int)crt-(int)arr));
		fprintf(carlog,"%d\t\t%s\t\t%s\t\t%s\t\t%d\n", front(lanes[currentQueue]).ID, lanes[currentQueue]->name,abuf,tbuf,abs((int)crt-(int)arr));
		
		if(lanes[currentQueue]->size!=0)Dequeue(lanes[currentQueue]);
		//For test:  printf("p work\n");
		
		pthread_mutex_unlock(&mutex);
		
		crt = time(NULL);
		currenttime = (long) crt;
	}
	
	pthread_exit(NULL);
}




int main(int argc, char *argv[]){
	/*	Here, lanes are produced and named. */
	lanes[0] = createQueue();
	lanes[0]->name = "WEST";
	lanes[1] = createQueue();
	lanes[1]->name = "SOUTH";
	lanes[2] = createQueue();
	lanes[2]->name = "EAST";
	lanes[3] = createQueue();
	lanes[3]->name = "NORTH";
	int j;
	long exectime,et;

	/*Snap time is an input from user. */
	snaptime = atoi(argv[4]) + (long)time(NULL);

	/* car.log and polic.log files are initiated. */
	carlog = fopen("car.log","w");
	policelog = fopen("police.log","w");
	fprintf(carlog, "CarID\tDirection\tArrival-Time\tCross-Time\t\tWait-Time\n"
					"-------------------------------------------------------------\n");
	fprintf(policelog, "Time\t\t\tEvent\n"
					   "--------------------------\n");

	/*Here one car is putted into every lane before starting. */
	lane_init();

	/* Probability value is got from the user. */
	probability = atof(argv[3]);

	/* 	Here I get execution time from the user.
		Its unit would be minute or second. */

	exectime = (long) atoi(argv[2]);
	if(strncmp(argv[1], "-m", 2)==0){exectime*=60;}
	et=exectime+(long) time(NULL);

	/* Random is initiated for threads. */
	srand(et);

	/*	I implemented 5 threads for lanes and police.
		They are created here.
		*/
	pthread_t threads[5];
	pthread_attr_t attr;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&honk, NULL);

	pthread_attr_init(&attr);
  	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  	pthread_create(&threads[0], &attr, west, (void *)et);
  	pthread_create(&threads[1], &attr, south, (void *)et);
  	pthread_create(&threads[2], &attr, east, (void *)et);
  	pthread_create(&threads[3], &attr, north, (void *)et);
  	pthread_create(&threads[4], &attr, police, (void *)et);

  	/*	After the works is done, threads are joined.
  		Log files are closed.
  		Mutexes are destroyed.
  		Memories of lanes are freed.
  		And exited. */
  	for (j = 0; j < 5; j++)
  	{
  		pthread_join(threads[j], NULL);
  	}
  	fprintf(carlog, "\nTotal Time: %ds\t\tPorbability: %.2f\n",(int)exectime, probability);
	fprintf(policelog, "\nTotal Time: %ds\t\tPorbability: %.2f\n",(int)exectime, probability);
  	
  	pthread_attr_destroy(&attr);
  	pthread_mutex_destroy(&mutex);
  	pthread_cond_destroy(&honk);
  	pthread_exit (NULL);
  	
  	

  	fclose(carlog);
  	fclose(policelog);

  	
  	free(lanes[0]);
  	free(lanes[1]);
  	free(lanes[2]);
  	free(lanes[3]);
  	free(lanes[0]->elements);

	return 0;
}