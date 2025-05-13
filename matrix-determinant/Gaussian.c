
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include "timer.h"
 int main(int argc, char const *argv[])
{ 
    
    if(argc!=2){
        printf("usage:%s <size>\n",argv[0]);
        return 1;
    }
    int size=strtol(argv[1],NULL,10);
    const char* sizestr=argv[1];
    char fname[21];
    sprintf(fname,"input/m%sx%s.bin",sizestr,sizestr);
    FILE * fp;
    double ** m=malloc(size*sizeof(double*));
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
    double d=1; 
    double logd=0;
    double start, finish;
    GET_TIME(start);
    for (int pivot = 0; pivot < size; pivot++)
    {
        logd+=log10(fabs(m[pivot][pivot]));
        d*=m[pivot][pivot];
        for (int r = pivot+1; r < size; r++)
        {
            double mult=m[r][pivot]/m[pivot][pivot];
            for(int c=pivot;c<size;c++){
                m[r][c]-=m[pivot][c]*mult;
            }
        }
        
    }
    
    
    GET_TIME(finish);




    // free and return
    for (int i = 0; i < size; i++)
    {
        //for (int j = 0; j < size; j++) printf("%lf ",m[i][j]);printf("\n");
        free(m[i]);
    }
    free(m);
    printf("Determenant: %lf\nLog(det): %lf\nTime: %f\n\n",d,logd,finish-start);
    return 0;
}
