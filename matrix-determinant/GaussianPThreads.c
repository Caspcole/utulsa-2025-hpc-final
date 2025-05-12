
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <pthread.h>
 #include "timer.h"
 double d;
 double logd;
 int size;
 double ** m;
 pthread_mutex_t * done;
 void* Thread_work(void* rank);
 void apply(int pivot, int target);
int threads;
 int main(int argc, char const *argv[])
{ 
    
    if(argc!=3){
        printf("usage:%s <size> <threads>\n",argv[0]);
        return 1;
    }
    size=strtol(argv[1],NULL,10);
    threads=strtol(argv[2],NULL,10);
    const char* sizestr=argv[1];
    char fname[21];
    sprintf(fname,"input/m%sx%s.bin",sizestr,sizestr);
    FILE * fp;
    m=malloc(size*sizeof(double*));
    fp = fopen(fname, "r");
    if(NULL==fp){
        printf("error opening file. was size a 4-digit power of 2 or multiple of 1000 between 16 and 5000? ");
        return 2;
    }

    for (int i = 0; i < size; i++)
    {
        m[i]=malloc(size*sizeof(double));
        fread(m[i],sizeof(double),size,fp);
    }
    fclose(fp);
    d=m[0][0]; 
    logd=log10(fabs(m[0][0]));
    double start, finish;
    GET_TIME(start);

    done=malloc(sizeof(pthread_mutex_t)*size);
    for (int i = 0; i < size; i++)
    {
        pthread_mutex_init(&done[i],NULL);
        pthread_mutex_lock(&done[i]);
        
    }
    pthread_mutex_unlock(&done[0]);
    
    pthread_t* thread_handles = malloc (threads*sizeof(pthread_t));
     for (int thread = 0; thread < threads; thread++)
       pthread_create(&thread_handles[thread], NULL,
           Thread_work, (void*) thread);
 
    for (int thread = 0; thread < threads; thread++) {
       pthread_join(thread_handles[thread], NULL);
    }
    
    
    
    GET_TIME(finish);




    // free and return
    for (int i = 0; i < size; i++)
    {
        //for (int j = 0; j < size; j++) printf("%lf ",m[i][j]);printf("\n");
        free(m[i]);
    }
    free(m);
    free(done);
    printf("Determenant: %lf\nLog(det): %lf\nTime: %f\nThreads:%i \n\n",d,logd,finish-start,threads);
    return 0;
}

void* Thread_work(void* in){
    int rank=in;
    //printf("rank:%i\n",*rank);
    for (int target = rank; target < size; target+=threads)
    {
        for (int pivot = 0; pivot < target; pivot++)
        {
            //printf("waiting for finish of row %i in thread %i\n",pivot,rank);
            pthread_mutex_lock(&done[pivot]);
            pthread_mutex_unlock(&done[pivot]);
            apply(pivot,target);
            //printf("in thread %i, row %i to row %i\n", *rank,pivot,target);
        }
        pthread_mutex_unlock(&done[target]);
        //printf("done doing %i in thread %i\n",target,rank);
         logd+=log10(fabs(m[target][target]));
        d*=m[target][target];
        
    }

}

void apply(int pivot, int target){
    double mult=m[target][pivot]/m[pivot][pivot];
            for(int c=pivot;c<size;c++){
                m[target][c]-=m[pivot][c]*mult;
            }
}