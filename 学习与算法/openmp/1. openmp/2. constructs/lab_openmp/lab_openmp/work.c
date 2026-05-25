#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <stdio.h>
#include <math.h>
void pwork(int nthreads, int mytid, double factor, int N, double *x, double **a);

int main(int argc, char *argv[])
{
   int N=3400;
   //   int N=6000;
   //   int N=12000;
   int j,i,nthreads,mytid;
   struct timeval it0,it1;
   double x[N];
   double **a = (double**)malloc(N*sizeof(double *));
   double factor,time;

   for(i=0; i<N; ++i) a[i] = (double*)malloc(N*sizeof(double));   
   for(i=0; i<N; ++i)
   {
      x[i]=9.9;
      for(j=0; j<N; ++j)
      {
         a[i][j]=9.5;
      }
   }
   factor=0.5;

   gettimeofday(&it0,NULL);

#pragma omp parallel private(mytid,nthreads)
   {
      nthreads=omp_get_num_threads();
      mytid   =omp_get_thread_num();
      printf("nthreads,mytid: %d %d\n",nthreads,mytid);
      pwork(nthreads,mytid,factor,N,x,a);
   }

   gettimeofday(&it1,NULL);
   long seconds = it1.tv_sec - it0.tv_sec;
   long microseconds = it1.tv_usec - it0.tv_usec;
   time = seconds + microseconds*1e-6;
   printf(" wall-clock time = %e\n",time);
}

void pwork(int nthreads, int mytid, double factor, int N, double *x, double **a)
{
   int jbeg,jend,i,j;

   jbeg= mytid *(N/nthreads);
   jend=(1 + mytid)*(N/nthreads);

   for(i=0; i<N; ++i)
   {
      for(j=jbeg; j<jend; ++j)
      {  
         a[i][j] = sqrt(a[i][j]/x[j])*sqrt(x[i]*factor/a[i][j]);
      }
   }
}
