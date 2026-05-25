program daxpy

   integer,parameter :: N=48*1024*1024
   real*8,save  :: a(N),b(N),c(N)

   call suba(N,a,b,c)

end program

subroutine suba(N,a,b,c)
   integer :: N
   real*8  :: a(N),b(N),c(N)
   real*8  :: s,t,time,rate
   integer :: irate,it0,it1,it2
   integer,parameter :: KMAX=10

   a=1.1;b=1.1;s=2.0

   call system_clock(count_rate=irate)
   call system_clock(it0)
   call system_clock(it1)

do k = 1,KMAX

!$OMP PARALLEL DO private(i,t), shared(a,b,c,N,s)
   do i = 1,N
      c(i) = a(i)*s+b(i)
   end do
   t = t+c(N)+c(1)  !Keeps optimizer from removing outer loop
end do

   call system_clock(it2)
   time = dble(it2 - it1*2 + it0)/dble(irate)
   rate = KMAX*dble(2*N)/(dble(1024*1024)*time)
   print*," MFLOPS=",rate,"    time=",time
   open(77,file='/dev/null')
   write(77,fmt='(d10.3)') t   ! make sure t is used someplace
   close(77)

end subroutine
