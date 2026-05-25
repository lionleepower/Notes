#include <stdio.h>
#include <sys/time.h>
#include <omp.h>
void suba(int N, double a[], double b[], double c[]);

int main(int argc, char *argv[])
{
   int N = 48*1024*1024;
   double a[N],b[N],c[N];
   
   suba(N,a,b,c);
}

void suba(int N, double a[], double b[], double c[])
{
   struct timeval it0,it1;
   double s,t,time,rate;
   int i,k;
   const int KMAX=10;

   for(i=0; i<N; ++i)
   {
      a[i] = 1.1;
      b[i] = 1.1;
   }
   s = 2.0;

   gettimeofday(&it0,NULL);

   for(k=0; k<KMAX; ++k)
   {
#pragma omp parallel for private(i,t), shared(a,b,c,N,s)
      for(i=0; i<N; ++i)
      {
         c[i] = a[i]*s+b[i];
      }
      t = t+c[N]+c[1];  //Keeps optimizer from removing outer loop
   }
   gettimeofday(&it1,NULL);
   long seconds = it1.tv_sec - it0.tv_sec;
   long microseconds = it1.tv_usec - it0.tv_usec;
   time = seconds + microseconds*1e-6;
   rate = KMAX*(double)(2*N)/((double)(1024*1024)*time);
   printf(" MFLOPS= %f    time= %f\n",rate,time);
   freopen("/dev/null","w",stdout);
   printf("%f\n",t);    //make sure t is used someplace
}
