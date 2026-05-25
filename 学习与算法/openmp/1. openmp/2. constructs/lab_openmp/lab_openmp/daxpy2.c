#include <stdio.h>
#include <sys/time.h>
#include <omp.h>
#ifdef __aarch64__
#include <nvpl_blas.h>
#endif
#ifdef __x86_64__
#include <mkl.h>
#endif
void suba(int N, double a[], double b[]);

int main(int argc, char *argv[])
{
   int N = 48*1024*1024;
   double a[N],b[N];
   
   suba(N,a,b);
}

void suba(int N, double a[], double b[])
{
   struct timeval it0, it1;
   double s,time,rate;
   int i,k,KMAX=10;

   for(i=0; i<N; ++i)
   {
      a[i] = 1.1;
      b[i] = 1.1;
   }
   s = 2.0;

   gettimeofday(&it0,NULL);

   for(k=0; k<KMAX; ++k)
   {
      cblas_daxpy(N, s, a, 1, b, 1);
   }
   
   gettimeofday(&it1,NULL);
   long seconds = it1.tv_sec - it0.tv_sec;
   long microseconds = it1.tv_usec - it0.tv_usec;
   time = seconds + microseconds*1e-6;
   rate = KMAX*(double)(2*N)/((double)(1024*1024)*time);
   printf(" MFLOPS= %f    time= %f\n",rate,time);
}
