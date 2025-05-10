
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #define size 16
 int main(int argc, char const *argv[])
{ 
    char* sizestr="0016";

    char fname[21];
    sprintf(fname,"input/m%sx%s.bin",sizestr,sizestr);
    FILE * fp;
    size_t len = 0;
    double*  line;
    double ** m=malloc(size*sizeof(double*));

    fp = fopen(fname, "r");
    for (int i = 0; i < size; i++)
    {
        m[i]=malloc(size*sizeof(double));
        fread(m[i],sizeof(double),size,fp);
        
    }
    fclose(fp);
    double d=1;
    double logd=0;
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
    
    




    // free and return
    for (int i = 0; i < size; i++)
    {
        //for (int j = 0; j < size; j++) printf("%lf ",m[i][j]);printf("\n");
        free(m[i]);
    }
    free(m);
    printf("Determenant: %lf\n Log(det): %lf\n",d,logd);
    return 0;
}
