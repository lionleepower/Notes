program daxpy2

   integer,parameter :: N=48*1024*1024
   real*8,save  :: a(N),b(N)

   call suba(N,a,b)

end program

subroutine suba(N,a,b)
   integer :: N
   real*8  :: a(N),b(N)
   real*8  :: s,time,rate,t
   integer,parameter :: KMAX=10

   a=1.1;b=1.1;s=2.0

   call system_clock(count_rate=irate)
   call system_clock(it0)
   call system_clock(it1)

   do k = 1,KMAX
      call daxpy(N, s, a, 1, b, 1)
   end do

   call system_clock(it2)
   time = dble(it2 - it1*2 + it0)/dble(irate)
   rate = KMAX*dble(2*N)/(dble(1024*1024)*time)
   print*," MFLOPS=",rate,"    time=",time

end subroutine
